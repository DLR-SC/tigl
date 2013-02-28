/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-28 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Export routines for CPACS configurations.
*/

#include "CTiglExportStep.h"
#include "CCPACSImportExport.h"
#include "CCPACSConfiguration.h"

#include "TopoDS_Shape.hxx"
#include "STEPControl_Controller.hxx"
#include "Standard_CString.hxx"
#include "Interface_Static.hxx"
#include "APIHeaderSection_MakeHeader.hxx"
#include "STEPControl_StepModelType.hxx"


namespace tigl {

    // Constructor
    CTiglExportStep::CTiglExportStep(CCPACSConfiguration& config)
    :myConfig(config)
    {
    }

    // Destructor
    CTiglExportStep::~CTiglExportStep(void)
    {
    }
	
    
    // Exports the whole configuration as STEP file
    // All wing- and fuselage segments are exported as single bodys
    void CTiglExportStep::ExportStep(const std::string& filename) const
    {
        STEPControl_Controller::Init();
        STEPControl_Writer            stepWriter;

        if( filename.empty()) {
           LOG(ERROR) << "Error: Empty filename in ExportStep.";
           return;
        }

        // Export all wings of the configuration
        for (int w = 1; w <= myConfig.GetWingCount(); w++)
        {
            CCPACSWing& wing = myConfig.GetWing(w);

            for (int i = 1; i <= wing.GetSegmentCount(); i++)
            {
                CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
                TopoDS_Shape loft = segment.GetLoft();

                stepWriter.Transfer(loft, STEPControl_AsIs);
            }
        }

        // Export all fuselages of the configuration
        for (int f = 1; f <= myConfig.GetFuselageCount(); f++)
        {
            CCPACSFuselage& fuselage = myConfig.GetFuselage(f);

            for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
            {
                CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
                TopoDS_Shape loft = segment.GetLoft();

                // Transform loft by fuselage transformation => absolute world coordinates
                loft = fuselage.GetFuselageTransformation().Transform(loft);

                stepWriter.Transfer(loft, STEPControl_AsIs);
            }
        }

        // Write STEP file
        if (stepWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True)
            throw CTiglError("Error: Export to STEP file failed in CTiglExportStep::ExportSTEP", TIGL_ERROR);
    }


    // Exports the whole configuration as one fused part to an STEP file
    void CTiglExportStep::ExportFusedStep(const std::string& filename)
    {
        TopoDS_Shape fusedAirplane = myConfig.GetFusedAirplane();

        STEPControl_Controller::Init();
        STEPControl_Writer            stepWriter;

        if( filename.empty()) {
           LOG(ERROR) << "Error: Empty filename in ExportFusedStep.";
           return;
        }

        stepWriter.Transfer(fusedAirplane, STEPControl_AsIs);

        // Write STEP file
        if (stepWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True)
            throw CTiglError("Error: Export fused shapes to STEP file failed in CTiglExportStep::ExportFusedStep", TIGL_ERROR);
    }




    // Save a sequence of shapes in STEP Format
    void CTiglExportStep::ExportShapes(const Handle(TopTools_HSequenceOfShape)& aHSequenceOfShape, const std::string& filename)
    {
        STEPControl_Controller::Init();
        STEPControl_Writer            stepWriter;

        if( filename.empty()) {
           LOG(ERROR) << "Error: Empty filename in ExportShapes.";
           return;
        }

        for (Standard_Integer i=1;i<=aHSequenceOfShape->Length();i++)
        {
            stepWriter.Transfer(aHSequenceOfShape->Value(i), STEPControl_AsIs);
        }

        if (stepWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True)
            throw CTiglError("Error: Export of shapes to STEP file failed in CTiglExportStep::ExportShapes", TIGL_ERROR);
    }


    // Saves as step, with cpacs metadata information in it
    void CTiglExportStep::ExportStepWithCPACSMetadata(const std::string& filename)
       {
           if( filename.empty()) {
               LOG(ERROR) << "Error: Empty filename in ExportStepWithCPACSMetadata.";
               return;
           }

           CCPACSImportExport generator(myConfig);
           Handle(TDocStd_Document) hDoc = generator.buildXDEStructure();

           STEPControl_Controller::Init();
           STEPCAFControl_Writer writer;
           writer.Transfer(hDoc, STEPControl_AsIs);
           writer.Write(filename.c_str());
       }

} // end namespace tigl
