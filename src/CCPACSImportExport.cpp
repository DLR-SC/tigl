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
* @brief  Class to manage geometry import and export. At this time only export will be supported.
*/

#include "CCPACSImportExport.h"
#include "CTiglLogger.h"
#include "CCPACSConfigurationManager.h"


#include <iostream>
#include <sstream>
#include <exception>

#include "IGESControl_Controller.hxx"
#include "IGESControl_Writer.hxx"
#include "Interface_Static.hxx"
#include "TCollection_ExtendedString.hxx"
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDocStd_Document.hxx"
#include "IGESControl_Controller.hxx"
#include "IGESCAFControl_Writer.hxx"

namespace tigl {

	// Constructor
	CCPACSImportExport::CCPACSImportExport()
	{
	}

	// Destructor
	CCPACSImportExport::~CCPACSImportExport(void)
	{
	}


    bool CCPACSImportExport::SaveStructuredIges(TiglCPACSConfigurationHandle cpacsHandle, const std::string& filename)
    {
        if( filename.empty()) {
            LOG(ERROR) << "Error: Empty filename in SaveStructuredIges.";
            return false;
        }

        Handle(XCAFApp_Application) hApp = XCAFApp_Application::GetApplication();
        Handle(TDocStd_Document) hDoc;
        hApp->NewDocument(TCollection_ExtendedString("MDTV-CAF"), hDoc);
        Handle_XCAFDoc_ShapeTool hShapeTool = XCAFDoc_DocumentTool::ShapeTool(hDoc->Main());

        TDF_Label rootLabel= TDF_TagSource::NewChild(hDoc->Main());


        CCPACSConfigurationManager& manager = CCPACSConfigurationManager::GetInstance();
        CCPACSConfiguration& config = manager.GetConfiguration(cpacsHandle);
        CTiglUIDManager& uidManager = config.GetUIDManager();

        CTiglAbstractPhysicalComponent* rootComponent = uidManager.GetRootComponent();
        if (rootComponent == NULL) {
            LOG(ERROR) << "Error: No Root Component";
            return false;
        }

        rootComponent->ExportDataStructure(rootLabel);


        IGESControl_Controller::Init();
        IGESCAFControl_Writer writer;
        writer.Transfer(hDoc);
        writer.Write(filename.c_str());

        return true;
    }





	// Save a sequence of shapes in IGES Format
	void CCPACSImportExport::SaveIGES(const Handle(TopTools_HSequenceOfShape)& aHSequenceOfShape, const std::string& filename)
	{
		IGESControl_Controller::Init();
	    IGESControl_Writer igesWriter;

        Interface_Static::SetIVal("write.iges.brep.mode", 0);
        Interface_Static::SetCVal("write.iges.header.author", "TIGL");
        Interface_Static::SetCVal("write.iges.header.company", "German Aerospace Center (DLR), SC");

		for (Standard_Integer i=1;i<=aHSequenceOfShape->Length();i++)  
		{
			igesWriter.AddShape (aHSequenceOfShape->Value(i));
		}

		igesWriter.ComputeModel();
		if (igesWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True)
            throw CTiglError("Error: Export of shapes to IGES file failed in CCPACSImportExport::SaveIGES", TIGL_ERROR);
	}

} // end namespace tigl
