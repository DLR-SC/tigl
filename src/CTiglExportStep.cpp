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
#include "CCPACSFuselageSegment.h"
#include "CCPACSWingSegment.h"

#include "TopoDS_Shape.hxx"
#include "STEPControl_Controller.hxx"
#ifdef TIGL_USE_XCAF
#include "STEPCAFControl_Writer.hxx"
#endif
#include "STEPControl_Writer.hxx"
#include "Standard_CString.hxx"
#include "Interface_Static.hxx"
#include "APIHeaderSection_MakeHeader.hxx"
#include "STEPControl_StepModelType.hxx"
#include "TopExp_Explorer.hxx"
#include "BRepAlgoAPI_Cut.hxx"

#define STEP_WRITEMODE STEPControl_AsIs

namespace tigl 
{

// Constructor
CTiglExportStep::CTiglExportStep(CCPACSConfiguration& config)
:myConfig(config)
{
    SetExportMode(AS_FACES);
}

// Destructor
CTiglExportStep::~CTiglExportStep(void)
{
}

void CTiglExportStep::AddFacesOfShape(const TopoDS_Shape& shape, STEPControl_Writer& writer) const 
{
    if (exportMode == AS_FACES) {
        TopExp_Explorer faceExplorer;
        for (faceExplorer.Init(shape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
            const TopoDS_Face& currentFace = TopoDS::Face(faceExplorer.Current());
            int ret = writer.Transfer(currentFace, STEP_WRITEMODE);
            if (ret > IFSelect_RetDone) {
                throw CTiglError("Error: Export to STEP file failed in CTiglExportStep. Could not translate face to step entity,", TIGL_ERROR);
            }
        }
    }
    else {
        int ret = writer.Transfer(shape, STEP_WRITEMODE);
        if (ret > IFSelect_RetDone) {
            throw CTiglError("Error: Export to STEP file failed in CTiglExportStep. Could not translate shape to step entity,", TIGL_ERROR);
        }
    }
}

void CTiglExportStep::SetExportMode(TiglStepExportMode mode) 
{
    exportMode = mode;
}


// Exports the whole configuration as STEP file
// All wing- and fuselage segments are exported as single bodys
void CTiglExportStep::ExportStep(const std::string& filename) const
{
    STEPControl_Controller::Init();
    STEPControl_Writer            stepWriter;

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportStep.";
       return;
    }

    // Export all wings of the configuration
    for (int w = 1; w <= myConfig.GetWingCount(); w++) {
        CCPACSWing& wing = myConfig.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();
            AddFacesOfShape(loft, stepWriter);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= myConfig.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = myConfig.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();

            AddFacesOfShape(loft, stepWriter);
        }
    }

    if (myConfig.GetFarField().GetFieldType() != NONE) {
        AddFacesOfShape(myConfig.GetFarField().GetLoft(), stepWriter);
    }

    // Write STEP file
    if (stepWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True) {
        throw CTiglError("Error: Export to STEP file failed in CTiglExportStep::ExportSTEP", TIGL_ERROR);
    }
}


// Exports the whole configuration as one fused part to an STEP file
void CTiglExportStep::ExportFusedStep(const std::string& filename)
{
    TopoDS_Shape fusedAirplane = myConfig.GetFusedAirplane();

    STEPControl_Controller::Init();
    STEPControl_Writer            stepWriter;

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportFusedStep.";
       return;
    }

    // if we have a far field, do boolean subtract of plane from far-field
    if (myConfig.GetFarField().GetFieldType() != NONE) {
        try {
            LOG(INFO) << "Trimming plane with far field...";
            TopoDS_Shape& farField = myConfig.GetFarField().GetLoft();
            fusedAirplane = BRepAlgoAPI_Cut(farField, fusedAirplane);
            LOG(INFO) << "Done trimming.";
        }
        catch(Standard_ConstructionError& err) {
            LOG(ERROR) << "OpenCascade error: " << err.GetMessageString();
            LOG(ERROR) << "Can not trim plane with far field. Far field will not be part of STEP export.";
        }
    }

    AddFacesOfShape(fusedAirplane, stepWriter);

    // Write STEP file
    if (stepWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True) {
        throw CTiglError("Error: Export fused shapes to STEP file failed in CTiglExportStep::ExportFusedStep", TIGL_ERROR);
    }
}




// Save a sequence of shapes in STEP Format
void CTiglExportStep::ExportShapes(const Handle(TopTools_HSequenceOfShape)& aHSequenceOfShape, const std::string& filename)
{
    STEPControl_Controller::Init();
    STEPControl_Writer            stepWriter;

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportShapes.";
       return;
    }

    for (Standard_Integer i=1;i<=aHSequenceOfShape->Length();i++) {
        AddFacesOfShape(aHSequenceOfShape->Value(i), stepWriter);
    }

    if (stepWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True) {
        throw CTiglError("Error: Export of shapes to STEP file failed in CTiglExportStep::ExportShapes", TIGL_ERROR);
    }
}

#ifdef TIGL_USE_XCAF
// Saves as step, with cpacs metadata information in it
void CTiglExportStep::ExportStepWithCPACSMetadata(const std::string& filename)
{
    if ( filename.empty()) {
        LOG(ERROR) << "Error: Empty filename in ExportStepWithCPACSMetadata.";
        return;
    }
    
    CCPACSImportExport generator(myConfig);
    Handle(TDocStd_Document) hDoc = generator.buildXDEStructure();
    
    STEPControl_Controller::Init();
    STEPCAFControl_Writer writer;
    writer.Transfer(hDoc, STEP_WRITEMODE);
    writer.Write(filename.c_str());
}
#endif

} // end namespace tigl
