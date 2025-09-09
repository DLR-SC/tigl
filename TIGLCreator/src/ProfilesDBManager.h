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

#ifndef TIGL_PROFILESDBMANAGER_H
#define TIGL_PROFILESDBMANAGER_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QStringList>

#include "generated/CPACSProfiles.h"
#include "tixi.h"
#include "CTiglUIDManager.h"
#include "CTiglSectionElement.h"

#include "UniquePtr.h"

/**
 * This class manages available profiles.
 * Basically there are two places where profiles can be stored: In current CPACS file (configuration profiles)
 * and in the local profileDB file (local profiles).
 * This class merges the profiles of these two places into a unique interface and can transfer profiles
 * from one place to another.
 * @remark The local profileDB file is a file that contains only the CPACS "profiles" section.
 *
 */
class ProfilesDBManager : public QObject
{
    // TODO: Refactoring: use singleton pattern for this class

public:
    ProfilesDBManager();
    ~ProfilesDBManager(){ localProfiles.reset(); }

    void setConfigProfiles(boost::optional<tigl::CCPACSProfiles>& newConfigProfiles);
    void cleanConfigProfiles();
    void setLocalProfiles(QString newDBFile);

    QStringList getAllFuselagesProfiles();
    QStringList getAllWingProfiles();
    QStringList getAllAvailableProfilesFor(tigl::CTiglSectionElement* cElement);

    void copyProfileFromLocalToConfig(QString profileID);

    // Check if the profileID is a configuration profile base on its name
    bool hasProfileConfigSuffix(QString profileID);
    bool hasProfileLocalSuffix(QString profileID);
    bool isAWingProfile(QString profileID);
    bool isAFuselageProfile(QString profileID);

    QString removeSuffix(QString profileID);

    inline QString getLocalSuffix() { return localSuffix; };
    inline QString getConfigSuffix() { return configSuffix; };


protected:
    void updateProfilesLists();

private:
    tigl::CTiglUIDManager myDBUIDManager;
    tigl::CCPACSProfiles* configProfiles;
    std::unique_ptr<tigl::CCPACSProfiles> localProfiles;

    QStringList wingProfileUIDs; // all wing profiles with suffix
    QStringList fuselageProfilesUIDs; // all fuselages profiles with suffix

    QString softDBFilename; // the file that store the records of the software profiles
    TixiDocumentHandle tixiHandle;

    QString localSuffix  = " (DB)";
    QString configSuffix = " (In File)";
};

#endif //TIGL_PROFILESDBMANAGER_H
