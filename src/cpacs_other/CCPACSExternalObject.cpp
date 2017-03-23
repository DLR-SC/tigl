/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-27 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CCPACSExternalObject.h"

#include "CCPACSConfiguration.h"
#include "CTiglImporterFactory.h"
#include "CGroupShapes.h"
#include "tiglcommonfunctions.h"
#include "CNamedShape.h"

namespace tigl
{

namespace external_object_private
{
    TIGL_EXPORT std::string getPathRelativeToApp(const std::string& cpacsPath, const std::string& linkedFilePath)
    {
        if (IsPathRelative(linkedFilePath)) {
            // the path is relative to the xml file
            // we need the path relative or absolute to the
            // executable that opened the cpacs file

            size_t pos = cpacsPath.find_last_of("/\\");
            if (pos == std::string::npos) {
                // no separator found, current directory
                return linkedFilePath;
            }
            std::string dirPath = cpacsPath.substr(0,pos);

            // create path of file
            return dirPath + "/" + linkedFilePath;
        }
        else {
            return linkedFilePath;
        }
    }

    /// Returns true, if the fileType is supported by the component loader
    TIGL_EXPORT bool fileTypeSupported(const std::string& fileType)
    {
        return CTiglImporterFactory::Instance().ImporterSupported(fileType);
    }
}
using namespace external_object_private;

CCPACSExternalObject::CCPACSExternalObject(CCPACSExternalObjects* parent)
    : generated::CPACSGenericGeometricComponent(parent), CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation) {}

const std::string& CCPACSExternalObject::GetUID() const {
    return generated::CPACSGenericGeometricComponent::GetUID();
}

// Read CPACS wing element
void CCPACSExternalObject::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& objectXPath)
{
    Reset();
    generated::CPACSGenericGeometricComponent::ReadCPACS(tixiHandle, objectXPath);

    char* cCPACSPath = NULL;
    tixiGetDocumentPath(tixiHandle, &cCPACSPath);
    _filePath = getPathRelativeToApp(cCPACSPath ? cCPACSPath : "", m_linkToFile.GetBase());

    // test if file can be read
    if (!IsFileReadable(_filePath)) {
        throw tigl::CTiglError("File " + _filePath + " can not be read!", TIGL_OPEN_FAILED);
    }

    // Register ourself at the unique id manager
    if (m_parent) {
        CCPACSConfiguration& config = m_parent->GetParent()->GetConfiguration();
        config.GetUIDManager().AddUID(m_uID, this);
    }
}

const std::string& CCPACSExternalObject::GetFilePath() const
{
    return _filePath;
}

TiglGeometricComponentType CCPACSExternalObject::GetComponentType() const
{
    return TIGL_COMPONENT_PHYSICAL;
}

PNamedShape CCPACSExternalObject::BuildLoft()
{
    const std::string& fileType = CPACSLinkToFileType_formatToString(m_linkToFile.GetFormat());
    PTiglCADImporter importer = CTiglImporterFactory::Instance().Create(fileType);
    if (importer) {
        ListPNamedShape shapes = importer->Read(_filePath);
        PNamedShape shapeGroup = CGroupShapes(shapes);
        if (shapeGroup) {
            shapeGroup->SetName(GetUID().c_str());
            shapeGroup->SetShortName(GetUID().c_str());

            // Apply transformation
            TopoDS_Shape sh = GetTransformation().Transform(shapeGroup->Shape());
            shapeGroup->SetShape(sh);
        }

        return shapeGroup;
    }
    else {
        throw CTiglError("Cannot open externalComponent. Unknown file format " + fileType);
    }
}

} // namespace tigl

