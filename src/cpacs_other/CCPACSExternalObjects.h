/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-19 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CCPACSEXTERNALOBJECTS_H
#define CCPACSEXTERNALOBJECTS_H

#include "tigl_internal.h"
#include "tixi.h"

#include <vector>

namespace tigl
{

class CCPACSExternalObject;
class CCPACSConfiguration;

class CCPACSExternalObjects
{
public:
    TIGL_EXPORT CCPACSExternalObjects(CCPACSConfiguration* config = 0);
    TIGL_EXPORT ~CCPACSExternalObjects();
    
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const char* configurationUID);
    
    TIGL_EXPORT CCPACSExternalObject& GetObject(int index) const;
    TIGL_EXPORT int GetObjectCount(void) const;

protected:
    void Cleanup(void);
    
private:
    typedef std::vector<CCPACSExternalObject*> CCPACSExtObjectContainer;
    CCPACSExtObjectContainer _objects;
    CCPACSConfiguration* _config;
};

} //namespace tigl

#endif // CCPACSEXTERNALOBJECTS_H
