/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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
/**
* @file 
* @brief  Class to build up a OCC-XDE document structure which could be exported
*           in different file types.
*/

#ifndef CCPACSIMPORTEXPORT_H
#define CCPACSIMPORTEXPORT_H

#include "tigl_config.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CTiglError.h"
#include "CCPACSConfiguration.h"
#include <string>

#include "TopoDS_Shape.hxx"
#ifdef TIGL_USE_XCAF
#include "Handle_TDocStd_Document.hxx"
#endif

namespace tigl
{

enum ShapeStoreType
{
    WHOLE_SHAPE,           /** Inserts the shape as it is into IGES. All faces will be named correctly but they will not be grouped by name */
    NAMED_COMPOUNDS,       /** Collects all faces with the same origin into compounds. All faces are named correctly */
    FACES                  /** Exports each face as its own group. The group name and the face name are identical    */
};

class CCPACSImportExport
{
private:
        
public:
    // Constructor
    TIGL_EXPORT CCPACSImportExport(CCPACSConfiguration& config);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSImportExport(void);

#ifdef TIGL_USE_XCAF
    Handle_TDocStd_Document buildXDEStructure();
#endif


protected:
        

private:
    CCPACSConfiguration&    myConfig;
};

} // end namespace tigl

#endif // CCPACSIMPORTEXPORT_H
