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

#include "CCPACSImportExport.h"
#include "CTiglLogger.h"
#include "CCPACSConfigurationManager.h"

#include <iostream>
#include <sstream>
#include <exception>

#include "Interface_Static.hxx"
#include "TCollection_ExtendedString.hxx"
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDocStd_Document.hxx"
#include "IGESControl_Controller.hxx"
#include "IGESCAFControl_Writer.hxx"
#include "TDataStd_Name.hxx"

namespace tigl {

    // Constructor
    CCPACSImportExport::CCPACSImportExport(CCPACSConfiguration& config)
    :myConfig(config)
    {
    }

    // Destructor
    CCPACSImportExport::~CCPACSImportExport(void)
    {
    }


    Handle_TDocStd_Document CCPACSImportExport::buildXDEStructure()
    {

        Handle(XCAFApp_Application) hApp = XCAFApp_Application::GetApplication();
        Handle(TDocStd_Document) hDoc;
        hApp->NewDocument(TCollection_ExtendedString("MDTV-XCAF"), hDoc);
        Handle_XCAFDoc_ShapeTool hShapeTool = XCAFDoc_DocumentTool::ShapeTool(hDoc->Main());
        TDF_Label rootLabel= TDF_TagSource::NewChild(hDoc->Main());


        CTiglUIDManager& uidManager = myConfig.GetUIDManager();

        CTiglAbstractPhysicalComponent* rootComponent = uidManager.GetRootComponent();
        if (rootComponent == NULL) {
            LOG(ERROR) << "Error: No Root Component";
            return NULL;
        }

        TDataStd_Name::Set (rootLabel, myConfig.GetUID().c_str());
        rootComponent->ExportDataStructure(hShapeTool, rootLabel);


        return hDoc;
    }




} // end namespace tigl
