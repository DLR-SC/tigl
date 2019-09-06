/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ProfilesDBManager.h"
#include "TIGLViewerSettings.h"
#include "CTiglLogging.h"
#include "CCPACSWingProfiles.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselageProfiles.h"
#include "CCPACSFuselageProfile.h"
#include "CTiglWingSectionElement.h"
#include "CTiglFuselageSectionElement.h"
#include "TIGLViewerErrorDialog.h"


ProfilesDBManager::ProfilesDBManager()
{
    localProfiles.reset();
    configProfiles = nullptr;

    TIGLViewerSettings settings = TIGLViewerSettings::Instance();
    setLocalProfiles(settings.profilesDBPath());
}

QStringList ProfilesDBManager::getAllWingProfiles()
{
    return wingProfileUIDs;
}

QStringList ProfilesDBManager::getAllFuselagesProfiles()
{
    return fuselageProfilesUIDs;
}

QStringList ProfilesDBManager::getAllAvailableProfilesFor(tigl::CTiglSectionElement* cElement)
{
    // a dirty trick to get the type of the element
    if (typeid(*cElement) == typeid(tigl::CTiglWingSectionElement)) {
        return getAllWingProfiles();
    }
    else if (typeid(*cElement) == typeid(tigl::CTiglFuselageSectionElement)) {
        return getAllFuselagesProfiles();
    }
    else {
        LOG(WARNING)
            << "ProfilesDBManager::getAllAvailableProfilesFor: Impossible to determine the type of the given cElement";
        return QStringList();
    }
}

void ProfilesDBManager::setLocalProfiles(QString newDBFile)
{
    softDBFilename = newDBFile;
    myDBUIDManager.Clear();
    localProfiles.reset();
    tixiHandle = -1;

    QFileInfo checkFile(softDBFilename);
    if ((!checkFile.exists()) || (!checkFile.isFile()) || (checkFile.suffix() != "xml")) {
        LOG(ERROR) << "ProfilesDBManager::setLocalProfiles: The path " << softDBFilename.toStdString()
                   << " seems not to be a valid path to a profiles DB file."
                   << "Note that a valid profiles file has the same format as the CPACS profiles sections"
                   << " and its name end by .xml";
        updateProfilesLists();
        return;
    }

    if (tixiOpenDocument(softDBFilename.toStdString().c_str(), &tixiHandle) != SUCCESS) {
        LOG(ERROR) << "ProfilesDBManager::setLocalProfiles: impossible to open the file "
                   << softDBFilename.toStdString() << " with tixi.";
        updateProfilesLists();
        return;
    }

    localProfiles = tigl::make_unique<tigl::CCPACSProfiles>(&myDBUIDManager);
    localProfiles->ReadCPACS(tixiHandle, "/");
    updateProfilesLists();
}

void ProfilesDBManager::cleanConfigProfiles()
{
    configProfiles = nullptr;
    updateProfilesLists();
}

void ProfilesDBManager::setConfigProfiles(boost::optional<tigl::CCPACSProfiles>& newConfigProfiles)
{
    cleanConfigProfiles();
    if (newConfigProfiles) {
        configProfiles = &(*newConfigProfiles);
        updateProfilesLists();
    }
}

void ProfilesDBManager::updateProfilesLists()
{

    wingProfileUIDs.clear();
    fuselageProfilesUIDs.clear();

    QString uidTemp;
    if (configProfiles && configProfiles->GetWingAirfoils()) {
        tigl::CCPACSWingProfiles& profiles = *(configProfiles->GetWingAirfoils());
        for (int i = 0; i < profiles.GetProfileCount(); i++) {
            uidTemp = profiles.GetProfile(i + 1).GetUID().c_str() + configSuffix;
            wingProfileUIDs.push_back(uidTemp);
        }
    }
    if (configProfiles && configProfiles->GetFuselageProfiles()) {
        tigl::CCPACSFuselageProfiles& profiles = *(configProfiles->GetFuselageProfiles());
        for (int i = 0; i < profiles.GetProfileCount(); i++) {
            uidTemp = profiles.GetProfile(i + 1).GetUID().c_str() + configSuffix;
            fuselageProfilesUIDs.push_back(uidTemp);
        }
    }
    if (localProfiles && localProfiles->GetWingAirfoils()) {
        tigl::CCPACSWingProfiles& profiles = *(localProfiles->GetWingAirfoils());
        for (int i = 0; i < profiles.GetProfileCount(); i++) {
            uidTemp = profiles.GetProfile(i + 1).GetUID().c_str() + localSuffix;
            wingProfileUIDs.push_back(uidTemp);
        }
    }
    if (localProfiles && localProfiles->GetFuselageProfiles()) {
        tigl::CCPACSFuselageProfiles& profiles = *(localProfiles->GetFuselageProfiles());
        for (int i = 0; i < profiles.GetProfileCount(); i++) {
            uidTemp = profiles.GetProfile(i + 1).GetUID().c_str() + localSuffix;
            fuselageProfilesUIDs.push_back(uidTemp);
        }
    }
}

void ProfilesDBManager::copyProfileFromLocalToConfig(QString profileID)
{
    if (!hasProfileLocalSuffix(profileID)) {
        throw tigl::CTiglError(" profileID :" + removeSuffix(profileID).toStdString() + "not found in local DB.");
    }
    // check if the config has already a profile with the same uid
    tigl::CTiglUIDManager&  uidManager = configProfiles->GetUIDManager();
    std::string profileUID = removeSuffix(profileID).toStdString();
    if (uidManager.IsUIDRegistered(profileUID)) {
        throw tigl::CTiglError("Impossible to copy profile in the configuration. The same uid " + profileUID +
                               " is already present in the configuration.");
    }
    if (isAWingProfile(profileID)) {
        tigl::CCPACSWingProfile& wingProfile =
            configProfiles->GetWingAirfoils(tigl::CreateIfNotExistsTag()).AddWingAirfoil();
        std::string xpath =
            "/profiles/wingAirfoils/wingAirfoil[@uID=\"" + removeSuffix(profileID).toStdString() + "\"]";
        wingProfile.ReadCPACS(tixiHandle, xpath);
    }
    else if (isAFuselageProfile(profileID)) {
        tigl::CCPACSFuselageProfile& fuselageProfile =
            configProfiles->GetFuselageProfiles(tigl::CreateIfNotExistsTag()).AddFuselageProfile();
        std::string xpath =
            "/profiles/fuselageProfiles/fuselageProfile[@uID=\"" + removeSuffix(profileID).toStdString() + "\"]";
        fuselageProfile.ReadCPACS(tixiHandle, xpath);
    }
    else {
        throw tigl::CTiglError(" profileID :" + removeSuffix(profileID).toStdString() +
                               " seems not to be a wing or a fuselage profile.");
    }
    updateProfilesLists();
}

QString ProfilesDBManager::removeSuffix(QString profileID)
{

    if (hasProfileLocalSuffix(profileID)) {
        profileID.remove(profileID.size() - localSuffix.size(), localSuffix.size());
        return profileID;
    }

    if (hasProfileConfigSuffix(profileID)) {
        profileID.remove(profileID.size() - configSuffix.size(), configSuffix.size());
        return profileID;
    }

    LOG(ERROR) << "ProfilesDBManager::removeSuffix: no suffix found!";
    return profileID;
}

bool ProfilesDBManager::hasProfileConfigSuffix(QString profileID)
{

    QRegExp suffixRegex;
    suffixRegex.setPatternSyntax(QRegExp::Wildcard);
    suffixRegex.setCaseSensitivity(Qt::CaseSensitive);

    suffixRegex.setPattern("*" + configSuffix );
    if (profileID.contains(suffixRegex)) {
        return true;
    }
    return false;
}

bool ProfilesDBManager::hasProfileLocalSuffix(QString profileID)
{

    QRegExp suffixRegex;
    suffixRegex.setPatternSyntax(QRegExp::Wildcard);
    suffixRegex.setCaseSensitivity(Qt::CaseSensitive);

    suffixRegex.setPattern("*" + localSuffix );
    if (profileID.contains(suffixRegex)) {
        return true;
    }
    return false;
}

bool ProfilesDBManager::isAFuselageProfile(QString profileID)
{
    return fuselageProfilesUIDs.contains(profileID);
}

bool ProfilesDBManager::isAWingProfile(QString profileID)
{
    return wingProfileUIDs.contains(profileID);
}
