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
#include "CTiglFusePlane.h"
#include "tiglcommonfunctions.h"

#include "TopoDS_Shape.hxx"
#include "STEPControl_Controller.hxx"

#ifdef TIGL_USE_XCAF
#include "STEPCAFControl_Writer.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDocStd_Document.hxx"
#endif // TIGL_USE_XCAF
#include "XSControl_WorkSession.hxx"
#include "XSControl_TransferWriter.hxx"


#include "STEPControl_Writer.hxx"
#include "STEPConstruct.hxx"
#include "StepShape_AdvancedFace.hxx"
#include "StepShape_OpenShell.hxx"
#include "StepShape_ClosedShell.hxx"
#include "STEPControl_StepModelType.hxx"
#include "TransferBRep.hxx"
#include "TransferBRep_ShapeMapper.hxx"
#include "Transfer_FinderProcess.hxx"
#include "Interface_Static.hxx"

#include "Standard_CString.hxx"
#include "TCollection_HAsciiString.hxx"
#include "TopExp.hxx"
#include "TopExp_Explorer.hxx"
#include "TopTools_IndexedMapOfShape.hxx"

#include <cassert>

#define STEP_WRITEMODE STEPControl_AsIs

namespace
{

    /**
     * @brief WriteSTEPFaceNames takes the names of each face and writes it into the STEP model
     * as an advanced face property
     */
    void WriteSTEPFaceNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
        for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
            TopoDS_Face face = TopoDS::Face(faceMap(iface));
            std::string faceName = shape->GetFaceTraits(iface-1).Name();

            // set face name
            Handle(StepShape_AdvancedFace) SF;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, face );
            if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(StepShape_AdvancedFace), SF ) ) {
                Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(faceName.c_str());
                SF->SetName(str);
            }
        }

        // write shell names
        TopTools_IndexedMapOfShape shellMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_SHELL, shellMap);
        for (int ishell = 1; ishell <= shellMap.Extent(); ++ishell) {
            TopoDS_Shell shell = TopoDS::Shell(shellMap(ishell));
            std::string shellName = shape->Name();

            // set shell name
            Handle(StepShape_OpenShell) SOS;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, shell );
            if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(StepShape_OpenShell), SOS ) ) {
                Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(shellName.c_str());
                SOS->SetName(str);
            }
            Handle(StepShape_ClosedShell) SCS;
            if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(StepShape_ClosedShell), SCS ) ) {
                Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(shellName.c_str());
                SCS->SetName(str);
            }
        }
    }

} //namespace

namespace tigl 
{

// Constructor
CTiglExportStep::CTiglExportStep(CCPACSConfiguration& config)
:myConfig(config)
{
    SetOCAFStoreType(NAMED_COMPOUNDS);
}

// Destructor
CTiglExportStep::~CTiglExportStep(void)
{
}

void CTiglExportStep::AddFacesOfShape(const TopoDS_Shape& shape, STEPControl_Writer& writer) const 
{
    if (myStoreType == FACES) {
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

// Exports the whole configuration as STEP file
// All wing- and fuselage segments are exported as single bodys
void CTiglExportStep::ExportStep(const std::string& filename) const
{
    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportStep.";
       return;
    }

    ListPNamedShape shapes;

    // Export all wings of the configuration
    for (int w = 1; w <= myConfig.GetWingCount(); w++) {
        CCPACSWing& wing = myConfig.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();
            PNamedShape shape(new CNamedShape(loft, segment.GetUID().c_str()));
            shapes.push_back(shape);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= myConfig.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = myConfig.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();
            PNamedShape shape(new CNamedShape(loft, segment.GetUID().c_str()));
            shapes.push_back(shape);
        }
    }

    CCPACSFarField& farfield = myConfig.GetFarField();
    if (farfield.GetFieldType() != NONE) {
        PNamedShape shape(new CNamedShape(farfield.GetLoft(), farfield.GetUID().c_str()));
        shapes.push_back(shape);
    }

    // write step
    try {
        ExportShapes(shapes, filename);
    }
    catch (CTiglError&) {
        throw CTiglError("Cannot export airplane in CTiglExportStep", TIGL_ERROR);
    }
}


// Exports the whole configuration as one fused part to an STEP file
void CTiglExportStep::ExportFusedStep(const std::string& filename)
{
    if (filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportFusedStep.";
       return;
    }

    PTiglFusePlane fuser = myConfig.AircraftFusingAlgo();
    fuser->SetResultMode(HALF_PLANE_TRIMMED_FF);
    assert(fuser);

    PNamedShape fusedAirplane = fuser->FusedPlane();
    PNamedShape farField      = fuser->FarField();
    if (!fusedAirplane) {
        throw CTiglError("Error computing fused airplane.", TIGL_NULL_POINTER);
    }

    try {
        ListPNamedShape l;
        l.push_back(fusedAirplane);
        l.push_back(farField);

        // add intersections
        const ListPNamedShape& ints = fuser->Intersections();
        ListPNamedShape::const_iterator it;
        for (it = ints.begin(); it != ints.end(); ++it) {
            l.push_back(*it);
        }

        ExportShapes(l, filename);
    }
    catch (CTiglError&) {
        throw CTiglError("Cannot export fused Airplane as STEP", TIGL_ERROR);
    }
}




// Save a sequence of shapes in STEP Format
void CTiglExportStep::ExportShapes(const ListPNamedShape& shapes, const std::string& filename) const
{
    STEPControl_Controller::Init();
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    Interface_Static::SetCVal("write.step.unit", "M");

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportShapes.";
       return;
    }

    ListPNamedShape::const_iterator it;
#ifdef TIGL_USE_XCAF
    // create the xde document
    Handle(XCAFApp_Application) hApp = XCAFApp_Application::GetApplication();
    Handle(TDocStd_Document) hDoc;
    hApp->NewDocument("MDTV-XCAF", hDoc);
    Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(hDoc->Main());
    
    STEPCAFControl_Writer stepWriter;

    ListPNamedShape list;
    for (it = shapes.begin(); it != shapes.end(); ++it) {
        ListPNamedShape templist = GroupFaces(*it, myStoreType);
        for (ListPNamedShape::iterator it2 = templist.begin(); it2 != templist.end(); ++it2) {
            list.push_back(*it2);
        }
    }
    
    for (it = list.begin(); it != list.end(); ++it) {
        InsertShapeToCAF(myAssembly, *it, false);
    }

    if (stepWriter.Transfer(hDoc, STEP_WRITEMODE) == Standard_False) {
        throw CTiglError("Cannot export shape as STEP", TIGL_ERROR);
    }

    Handle(Transfer_FinderProcess) FP = stepWriter.Writer().WS()->TransferWriter()->FinderProcess();
    
    // write face entity names
    for (it = list.begin(); it != list.end(); ++it) {
        PNamedShape pshape = *it;
        WriteSTEPFaceNames(FP, pshape);
    }
#else
    STEPControl_Writer stepWriter;

    for (it = shapes.begin(); it != shapes.end(); ++it) {
        PNamedShape pshape = *it;
        AddFacesOfShape(pshape->Shape(), stepWriter);
    }

    Handle(Transfer_FinderProcess) FP = stepWriter.WS()->TransferWriter()->FinderProcess();
    
    // write face entity names
    for (it = shapes.begin(); it != shapes.end(); ++it) {
        PNamedShape pshape = *it;
        WriteSTEPFaceNames(FP, pshape);
    }
#endif

    if (stepWriter.Write(const_cast<char*>(filename.c_str())) > IFSelect_RetDone) {
        throw CTiglError("Error: Export of shapes to STEP file failed in CTiglExportStep::ExportShapes", TIGL_ERROR);
    }
}

void CTiglExportStep::SetOCAFStoreType(ShapeStoreType type)
{
    myStoreType = type;
}

} // end namespace tigl
