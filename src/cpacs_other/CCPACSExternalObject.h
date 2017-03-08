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

#ifndef CCPACSEXTERNALOBJECT_H
#define CCPACSEXTERNALOBJECT_H

#include "tixi.h"
#include "tigl_internal.h"

#include "CTiglAbstractPhysicalComponent.h"



namespace  tigl
{

class CCPACSConfiguration;

class CCPACSExternalObject : public CTiglAbstractPhysicalComponent
{
public:
    TIGL_EXPORT CCPACSExternalObject(CCPACSConfiguration* config = NULL);
    
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& objectXPath);
    
    TIGL_EXPORT std::string GetFilePath() const;
    
    TIGL_EXPORT TiglGeometricComponentType GetComponentType(void);
    
    TIGL_EXPORT ~CCPACSExternalObject();
    
private:
    /// reads in the CAD file
    virtual PNamedShape BuildLoft(void);

    
    CCPACSConfiguration* _config;
    std::string _filePath, _fileType;
 
};

} // namespace tigl

#endif // CCPACSEXTERNALOBJECT_H
