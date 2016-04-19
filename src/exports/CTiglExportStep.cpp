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

#include "XSControl_WorkSession.hxx"
#include "XSControl_TransferWriter.hxx"


#include "STEPControl_Writer.hxx"
#include "STEPConstruct.hxx"
#include "StepShape_AdvancedFace.hxx"
#include "StepShape_OpenShell.hxx"
#include "StepShape_SolidModel.hxx"
#include "StepShape_ClosedShell.hxx"
#include "StepShape_GeometricCurveSet.hxx"
#include "StepShape_ShapeDefinitionRepresentation.hxx"
#include "StepShape_GeometricSetSelect.hxx"
#include "StepRepr_PropertyDefinition.hxx"
#include "StepBasic_Product.hxx"
#include "StepBasic_ProductDefinition.hxx"
#include "StepBasic_ProductDefinitionFormation.hxx"
#include "StepGeom_Curve.hxx"
#include "StepGeom_TrimmedCurve.hxx"
#include "STEPControl_StepModelType.hxx"
#include "TransferBRep.hxx"
#include "Transfer_Finder.hxx"
#include "TransferBRep_ShapeMapper.hxx"
#include "Transfer_FinderProcess.hxx"
#include "Interface_Static.hxx"

#include "Standard_CString.hxx"
#include "TCollection_HAsciiString.hxx"
#include "TopExp.hxx"
#include "TopExp_Explorer.hxx"
#include "TopTools_IndexedMapOfShape.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "TopoDS_Edge.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"

#include <cassert>

// [[CAS_AES]] TODO: add missing includes!!!
// [[CAS_AES]] BEGIN
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepLib.hxx>
#include <BRepTools.hxx>

#include "CTiglCommon.h"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
// [[CAS_AES]] END

#define STEP_WRITEMODE STEPControl_AsIs

namespace
{

    /**
     * @brief WriteSTEPProductName writes the shape names as the step product identifier
     */
    void WriteStepProductName(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }
        
        // write product name
        Handle(StepShape_ShapeDefinitionRepresentation) SDR;
        Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, shape->Shape() );
        if ( ! FP->FindTypedTransient ( mapper, STANDARD_TYPE(StepShape_ShapeDefinitionRepresentation), SDR ) ) {
            return;
        }
            
        // set the name to the PRODUCT
        Handle(StepRepr_PropertyDefinition) propDef = SDR->Definition().PropertyDefinition();
        if ( propDef.IsNull() ) {
            return;
        }
        Handle(StepBasic_ProductDefinition) prodDef = propDef->Definition().ProductDefinition();
        if ( prodDef.IsNull() ) {
            return;
        }
        Handle(StepBasic_Product) product = prodDef->Formation()->OfProduct();
        
        Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(shape->Name());
        product->SetId ( str );
        product->SetName ( str );
    }
    
    void WriteStepSolidName(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }
        
        TopTools_IndexedMapOfShape solidMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_SOLID, solidMap);
        for (int isolid = 1; isolid <= solidMap.Extent(); ++isolid) {
            TopoDS_Solid solid = TopoDS::Solid(solidMap(isolid));
            std::string solidName = shape->Name();

            // set solid name
            Handle(StepShape_SolidModel) SSM;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, solid );
            if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(StepShape_SolidModel), SSM ) ) {
                Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(solidName.c_str());
                SSM->SetName(str);
            }
        }
    }
    
    void WriteStepWireName(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }
        
        TopTools_IndexedMapOfShape wireMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_WIRE, wireMap);
        for (int iwire = 1; iwire <= wireMap.Extent(); ++iwire) {
            TopoDS_Wire wire = TopoDS::Wire(wireMap(iwire));
            std::string wireName = shape->Name();

            // set wire name
            Handle(StepShape_GeometricCurveSet) SGC;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, wire );
            if (!FP->FindTypedTransient ( mapper, STANDARD_TYPE(StepShape_GeometricCurveSet), SGC )) {
                continue;
            }
            
            Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(wireName.c_str());
            SGC->SetName(str);
            
            // name edges
            for (int i = 1; i <= SGC->NbElements(); ++i) {
                StepShape_GeometricSetSelect elem = SGC->ElementsValue(i);
                Handle_StepGeom_Curve curve = Handle(StepGeom_Curve)::DownCast(elem.Value());
                
                if (!curve.IsNull()) {
                    curve->SetName(str);
                }
                
                // CATIA does only show the basis curves. Hence we must name them too
                Handle_StepGeom_TrimmedCurve tcurve = Handle(StepGeom_TrimmedCurve)::DownCast(elem.Value());
                if (!tcurve.IsNull() && !tcurve->BasisCurve().IsNull()) {
                    tcurve->BasisCurve()->SetName(str);
                }
            }
        }
    }
    
    void WriteStepShellName(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }
        
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
    
    /**
     * @brief WriteSTEPFaceNames takes the names of each face and writes it into the STEP model
     * as an advanced face property
     */
    void WriteStepFaceNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
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
    }

    void WriteStepNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        WriteStepFaceNames(FP, shape);
        WriteStepShellName(FP, shape);
        WriteStepSolidName(FP, shape);
        WriteStepProductName(FP, shape);
    }

} //namespace

namespace tigl 
{

// Constructor
CTiglExportStep::CTiglExportStep(CCPACSConfiguration& config)
:_config(config)
{
    SetGroupMode(NAMED_COMPOUNDS);
}

// Destructor
CTiglExportStep::~CTiglExportStep(void)
{
}

/**
 * @brief Adds a shape to the step file. All faces are named according to their face
 * traits. If there are no faces, the wires are named according to the shape name.
 */
void CTiglExportStep::AddToStep(PNamedShape shape, STEPControl_Writer& writer) const 
{
    if (!shape) {
        return;
    }
    
    std::string shapeName = shape->Name();
    Handle(Transfer_FinderProcess) FP = writer.WS()->TransferWriter()->FinderProcess();

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
    // any faces?
    if (faceMap.Extent() > 0) {
        int ret = writer.Transfer(shape->Shape(), STEP_WRITEMODE);
        if (ret > IFSelect_RetDone) {
            throw CTiglError("Error: Export to STEP file failed in CTiglExportStep. Could not translate shape " 
                             + shapeName + " to step entity,", TIGL_ERROR);
        }
        WriteStepNames(FP, shape);
    }
    else {
        // no faces, export edges as wires
        Handle(TopTools_HSequenceOfShape) Edges = new TopTools_HSequenceOfShape();
        TopExp_Explorer myEdgeExplorer (shape->Shape(), TopAbs_EDGE);
        while (myEdgeExplorer.More()) {
            Edges->Append(TopoDS::Edge(myEdgeExplorer.Current()));
            myEdgeExplorer.Next();
        }
        ShapeAnalysis_FreeBounds::ConnectEdgesToWires(Edges, 1e-7, false, Edges);
        for (int iwire = 1; iwire <= Edges->Length(); ++iwire) {
            int ret = writer.Transfer(Edges->Value(iwire), STEP_WRITEMODE);
            if (ret > IFSelect_RetDone) {
                throw CTiglError("Error: Export to STEP file failed in CTiglExportStep. Could not translate shape " 
                                 + shapeName + " to step entity,", TIGL_ERROR);
            }
            PNamedShape theWire(new CNamedShape(Edges->Value(iwire),shapeName.c_str()));
            WriteStepWireName(FP, theWire);
            WriteStepProductName(FP, theWire);
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
    for (int w = 1; w <= _config.GetWingCount(); w++) {
        CCPACSWing& wing = _config.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            shapes.push_back(loft);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= _config.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = _config.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            shapes.push_back(loft);
        }
    }

    CCPACSFarField& farfield = _config.GetFarField();
    if (farfield.GetFieldType() != NONE) {
        shapes.push_back(farfield.GetLoft());
    }

    // write step
    try {
        ExportShapes(shapes, filename);
    }
    catch (CTiglError&) {
        LOG(ERROR) << "Cannot export airplane in CTiglExportStep";
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

    PTiglFusePlane fuser = _config.AircraftFusingAlgo();
    fuser->SetResultMode(HALF_PLANE_TRIMMED_FF);
    assert(fuser);

    PNamedShape fusedAirplane = fuser->FusedPlane();
    PNamedShape farField      = fuser->FarField();
    if (!fusedAirplane) {
        LOG(ERROR) << "Error computing fused airplane.";
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
        LOG(ERROR) << "Cannot export fused Airplane as STEP";
        throw CTiglError("Cannot export fused Airplane as STEP", TIGL_ERROR);
    }
}




// Save a sequence of shapes in STEP Format
void CTiglExportStep::ExportShapes(const ListPNamedShape& shapes, const std::string& filename) const
{
    STEPControl_Controller::Init();
    // CAS_AES write step models in mm (default value)
//     Interface_Static::SetCVal("xstep.cascade.unit", "M");
//     Interface_Static::SetCVal("write.step.unit", "M");

    Interface_Static::SetIVal("write.surfacecurve.mode", 0);

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportShapes.";
       return;
    }

    ListPNamedShape::const_iterator it;
    ListPNamedShape list;
    for (it = shapes.begin(); it != shapes.end(); ++it) {
        ListPNamedShape templist = GroupFaces(*it, _groupMode);
        for (ListPNamedShape::iterator it2 = templist.begin(); it2 != templist.end(); ++it2) {
            list.push_back(*it2);
        }
    }

    STEPControl_Writer stepWriter;
    
    for (it = list.begin(); it != list.end(); ++it) {
        PNamedShape pshape = *it;
        AddToStep(pshape, stepWriter);
    }
    
    if (stepWriter.Write(const_cast<char*>(filename.c_str())) > IFSelect_RetDone) {
        LOG(ERROR) << "Error: Export of shapes to STEP file failed in CTiglExportStep::ExportShapes";
        throw CTiglError("Error: Export of shapes to STEP file failed in CTiglExportStep::ExportShapes", TIGL_ERROR);
    }
}

void CTiglExportStep::SetGroupMode(ShapeGroupMode mode)
{
    _groupMode = mode;
}

// [[CAS_AES]] added method for exporting geometry with structure
// Exports the whole configuration as Step file
// All wing- and fuselage segments are exported as single bodys
void CTiglExportStep::ExportStructureStep(const std::string& filename) const
{
//     // [[CAS_AES]] Required for constructor of Step writer, without we get errors on setting the units
    // old version
//     STEPControl_Controller::Init();
//     STEPControl_Writer stepWriter;
//     STEPControl_StepModelType mode = STEPControl_AsIs;
//     //STEPControl_StepModelType mode = STEPControl_ShellBasedSurfaceModel;
//     IFSelect_ReturnStatus status;
    
    ListPNamedShape shapes;

    Interface_Static::SetIVal("write.surfacecurve.mode", 0);

    // Export all wings of the configuration
    for (int w = 1; w <= _config.GetWingCount(); w++)
    {
        CCPACSWing& wing = _config.GetWing(w);

        // [[CAS_AES]] added for symmetry
        TiglSymmetryAxis symmetryAxis = wing.GetSymmetryAxis();
        // [[CAS_AES]] added segment type computation
        SegmentType segmentType = INNER_SEGMENT;

        for (int i = 1; i <= wing.GetSegmentCount(); i++)
        {
            // [[CAS_AES]] added segment type computation
            if (i == 1 && wing.GetSegmentCount() == 1) {
                segmentType = INNER_OUTER_SEGMENT;
            } else if (i > 1 && i < wing.GetSegmentCount()) {
                segmentType = MID_SEGMENT;
            } else if (i == wing.GetSegmentCount()) {
                segmentType = OUTER_SEGMENT;
            }
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            // [[CAS_AES]] export loft splitted with ribs/spars instead of plain loft
            TopoDS_Shape loft;
            loft = segment.GetSplittedLoft(segmentType);
            // [[CAS_AES]] added symmetry handling
            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                loft = CTiglCommon::mirrorShape(loft, symmetryAxis);
            }

            // [[CAS_AES]] added level information
            PNamedShape shape(new CNamedShape(loft, "Segment Loft"));
            shapes.push_back(shape);

        }
    }

    bool exportDoorGeometry = false;

    //Export all fuselages of the configuration
    for (int f = 1; f <= _config.GetFuselageCount(); f++)
    {
        CCPACSFuselage& fuselage = _config.GetFuselage(f);

        TiglSymmetryAxis symmetryAxis = fuselage.GetSymmetryAxis();

        TopoDS_Compound compound;
        BRep_Builder Builder;
        Builder.MakeCompound(compound);
        
        for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
        {
            // [[CAS_AES]] added segment type computation
            SegmentType segmentType = INNER_SEGMENT;
            if (i == 1 && fuselage.GetSegmentCount() == 1) {
                segmentType = INNER_OUTER_SEGMENT;
            } else if (i > 1 && i < fuselage.GetSegmentCount()) {
                segmentType = MID_SEGMENT;
            } else if (i == fuselage.GetSegmentCount()) {
                segmentType = OUTER_SEGMENT;
            }


            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);

            TopoDS_Shape loft = segment.GetSplittedLoft(segmentType);
            
            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                loft = tigl::CTiglCommon::mirrorShape(loft, symmetryAxis);
            }
            
            PNamedShape shape(new CNamedShape(loft, "Fuselage"));
            shapes.push_back(shape);

        }
    }

    //Write Step file
    try {
        ExportShapes(shapes, filename);
    }
    catch (CTiglError&) {
        LOG(ERROR) << "Cannot export airplane in CTiglExportStep";
        throw CTiglError("Cannot export airplane in CTiglExportStep", TIGL_ERROR);
    }

}

// [[CAS_AES]] added method for exporting cfd geometry splitted with internal structures
// Exports the whole configuration as Step file
// All wing- and fuselage segments are exported as single bodys
void CTiglExportStep::ExportCFDStep(const std::string& filename, bool nHalfModel, bool HisssFile) const
{
    // [[CAS_AES]] Required for constructor of Step writer, without we get errors on setting the units


    // Vectors for the Shapes ant the solids
    std::vector<AeroShape> vFuselages;
    std::vector<AeroShape> vWings;
    std::vector<AeroShape> vFreeFormSurfs;

    // variable declaration
    SegmentType segmentType = INNER_SEGMENT;
    BRep_Builder builder;
    TopTools_IndexedMapOfShape faceMap;

    // true if a fuselage is defined

    bool bFuselage = false;

    AeroShape nAS;

    //Export all fuselages of the configuration
    for (int f = 1; f <= _config.GetFuselageCount(); f++)
    {

        bFuselage = true;

        TopoDS_Compound compFuselage;
        TopoDS_Compound compSolidFuselage;

        CCPACSFuselage& fuselage = _config.GetFuselage(f);

        TiglSymmetryAxis symmetryAxis = fuselage.GetSymmetryAxis();

        builder.MakeCompound(compFuselage);
        builder.MakeCompound(compSolidFuselage);

        // get all fuselage segments
        for (int i = 1; i <= fuselage.GetSegmentCount(); i++)
        {
            // [[CAS_AES]] added segment type computation

            segmentType = INNER_SEGMENT;

            if (i == 1 && fuselage.GetSegmentCount() == 1) {
                segmentType = INNER_OUTER_SEGMENT;
            } else if (i > 1 && i < fuselage.GetSegmentCount()) {
                segmentType = MID_SEGMENT;
            } else if (i == fuselage.GetSegmentCount()) {
                segmentType = OUTER_SEGMENT;
            }

            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            TopoDS_Shape loft = segment.GetAeroLoft(segmentType, nHalfModel);

            //Transform loft by fuselage transformation => absolute world coordinates
            loft = fuselage.GetFuselageTransformation().Transform(loft);


            builder.Add(compFuselage, loft);

            builder.Add(compSolidFuselage, fuselage.GetFuselageTransformation().Transform(segment.GetAeroLoft(segmentType)));
        }

        // create a Solid out of the fuselage segments
        BRepBuilderAPI_Sewing sewMachine;
        sewMachine.Add(compSolidFuselage);


        if (sewMachine.NbFreeEdges() > 0)
        {
            LOG(ERROR) << "Error during Fuselage solid generation.\nFree edges dedected during sewing.";
            throw CTiglError("Error during Fuselage solid generation.\nFree edges dedected during sewing.");
        }

        
        sewMachine.Perform();
        
        if(TopoDS_Mismatch(sewMachine.SewedShape(), TopAbs_SHELL))
        {
            LOG(ERROR) << "Error during Wing solid generation.\nThe shape for solid generation is not connected.";
            throw CTiglError("Error during Wing solid generation.\nThe shape for solid generation is not connected.");
        }
        
        BRepBuilderAPI_MakeSolid makeSolid(TopoDS::Shell(sewMachine.SewedShape()));
        
        TopoDS_Shape solid = makeSolid.Solid();
        

        if (!BRepLib::OrientClosedSolid(TopoDS::Solid(solid)))
        {
            LOG(ERROR) << "Error during Wing solid generation.\nSolid is open or unorientable.";
            throw CTiglError("Error during Wing solid generation.\nSolid is open or unorientable.");
        }

        //         Add fuselage as shape and solid
        nAS.Solid = solid;
        nAS.Name = fuselage.GetName();
        nAS.Shape = compFuselage;
        vFuselages.push_back(nAS);
        
        if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel) 
        {

    //         Add fuselage as shape and solid
            AeroShape nASSym;
            nASSym.Solid = tigl::CTiglCommon::mirrorShape(solid, symmetryAxis, false);
            nASSym.Name = fuselage.GetUID();
            nASSym.Shape = tigl::CTiglCommon::mirrorShape(compFuselage, symmetryAxis, false);

            vFuselages.push_back(nASSym);
        }


    }

    // Export all wings of the configuration
    for (int w = 1; w <= _config.GetWingCount(); w++)
    {

        TopoDS_Compound compWing;
        TopoDS_Compound compSolidWing;
        TopoDS_Compound compFuselage;
        builder.MakeCompound(compWing);
        builder.MakeCompound(compSolidWing);

        CCPACSWing& wing = _config.GetWing(w);
        double wingYPos = wing.GetTranslation().y;
        // [[CAS_AES]] added for symmetry
        TiglSymmetryAxis symmetryAxis = wing.GetSymmetryAxis();

        //  Get all wing segments of the wing
        for (int i = 1; i <= wing.GetSegmentCount(); i++)
        {

            segmentType = INNER_SEGMENT;
            // [[CAS_AES]] added segment type computation
            if (i == 1 && wing.GetSegmentCount() == 1) {
                segmentType = INNER_OUTER_SEGMENT;
            } else if (i > 1 && i < wing.GetSegmentCount()) {
                segmentType = MID_SEGMENT;
            } else if (i == wing.GetSegmentCount()) {
                segmentType = OUTER_SEGMENT;
            }
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            // [[CAS_AES]] export loft splitted with ribs/spars instead of plain loft
            TopoDS_Shape loft;
            loft = segment.GetAeroLoft(segmentType, nHalfModel, false, HisssFile);
            // [[CAS_AES]] added symmetry handling
            if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel) {
                loft = CTiglCommon::mirrorShape(loft, symmetryAxis);
            }

            builder.Add(compWing, loft);

            
            if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel && (fabs(wingYPos) < 10e-3)) 
            {
                builder.Add(compSolidWing, loft);
            }
            else 
            {
                segment.Invalidate();
                builder.Add(compSolidWing, segment.GetAeroLoft(segmentType, false, true, HisssFile));
            }
        }

        // create a solid out of the wing
        BRepBuilderAPI_Sewing sewMachine;
        sewMachine.Add(compSolidWing);

        if (sewMachine.NbFreeEdges() > 0)
        {
            LOG(ERROR) << "Error during Wing solid generation.\nFree edges dedected during sewing.";
            throw CTiglError("Error during Wing solid generation.\nFree edges dedected during sewing.");
        }
        
        sewMachine.Perform();

        if(TopoDS_Mismatch(sewMachine.SewedShape(), TopAbs_SHELL))
        {
            LOG(ERROR) << "Error during Wing solid generation.\nThe shape for solid generation is not connected.";
            throw CTiglError("Error during Wing solid generation.\nThe shape for solid generation is not connected.");
        }

        BRepBuilderAPI_MakeSolid makeSolid(TopoDS::Shell(sewMachine.SewedShape()));
        TopoDS_Shape solid = makeSolid.Solid();
                

        if (!BRepLib::OrientClosedSolid(TopoDS::Solid(solid)))
        {
            LOG(ERROR) << "Error during Wing solid generation.\nSolid is open or unorientable.";
            throw CTiglError("Error during Wing solid generation.\nSolid is open or unorientable.");
        }

        // cut wing with fuselage
        // remove the wing surfaces in the fuselage

        double u_min = 0.0, u_max = 0.0, v_min = 0.0, v_max = 0.0;

        if (bFuselage)
        {

            for (int i = 0; i < vFuselages.size(); i++)
            {

//                 TopoDS_Shape splitLoft = CTiglCommon::splitShape(compWing, vFuselages[i].Solid);
                TopoDS_Shape splitLoft = CTiglCommon::splitShape(compWing, vFuselages[i].Shape);
    
                faceMap.Clear();
                TopExp::MapShapes(splitLoft, TopAbs_FACE, faceMap);
    
                builder.MakeCompound(compWing);
    
                BRepClass3d_SolidClassifier solidFuseClass(vFuselages[i].Solid);
    
                for (int f = 1; f <= faceMap.Extent(); f++) 
                {
                    const TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
                    gp_Pnt pnt;
                    Handle(Geom_Surface) surf = BRep_Tool::Surface(loftFace);
                    BRepTools::UVBounds(loftFace, u_min, u_max, v_min, v_max);
                    pnt = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));
                    solidFuseClass.Perform(pnt, 10e-6);
    
                    if (solidFuseClass.State() == TopAbs_OUT)
                    {
                        builder.Add(compWing, loftFace);
                    }
    
                    if (solidFuseClass.State() == TopAbs_UNKNOWN)
                    {
                        LOG(ERROR) << "Error during Wing cutting with the fuselage.";
                        throw CTiglError("Error during Wing cutting with the fuselage.");
                    }
                }
            }
        }

        nAS.Solid = solid;
        nAS.Name = wing.GetName();
        nAS.Shape = compWing;

        vWings.push_back(nAS);
        
        AeroShape nASSym;
        
        if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel && (fabs(wingYPos) > 10e-3)) 
        {
    
    //         Add fuselage as shape and solid
            nASSym.Solid = tigl::CTiglCommon::mirrorShape(solid, symmetryAxis, false);
            nASSym.Name = wing.GetUID();
            nASSym.Shape = tigl::CTiglCommon::mirrorShape(compWing, symmetryAxis, false);

            vWings.push_back(nASSym);

        }

        // cut fuselage with wing
        // remove surfaces of the fuselage, in the wing


        if (bFuselage)
        {
            for (int i = 0; i < vFuselages.size(); i++)
            {
                
                Bnd_Box bBox1, bBox2;
                BRepBndLib::Add(vFuselages[i].Shape, bBox1);
                BRepBndLib::Add(nAS.Solid, bBox2);
                if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel && (fabs(wingYPos) > 10e-3)) 
                    BRepBndLib::Add(nASSym.Solid, bBox2);
                
                if(bBox1.IsOut(bBox2))
                    continue;

//                 TopoDS_Shape fuseLoft = tigl::CTiglCommon::splitShape(vFuselages[i].Shape, compWing);
                TopoDS_Shape fuseLoft = tigl::CTiglCommon::splitShape(vFuselages[i].Shape, nAS.Solid);
                
    
                faceMap.Clear();
                TopExp::MapShapes(fuseLoft, TopAbs_FACE, faceMap);
    
                builder.MakeCompound(compFuselage);

                BRepClass3d_SolidClassifier solidFuseClass50(solid);
                BRepClass3d_SolidClassifier solidFuseClass25(solid);
                BRepClass3d_SolidClassifier solidFuseClass75(solid);
    
                for (int f = 1; f <= faceMap.Extent(); f++) 
                {
                    const TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
                    gp_Pnt pnt50, pnt25, pnt75;
                    Handle(Geom_Surface) surf = BRep_Tool::Surface(loftFace);
                    BRepTools::UVBounds(loftFace, u_min, u_max, v_min, v_max);
    
                    pnt50 = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));
                    pnt25 = surf->Value(u_min + ((u_max - u_min) / 4), v_min + ((v_max - v_min) / 4));
                    pnt75 = surf->Value(u_min + 3 * ((u_max - u_min) / 4), v_min + 3 * ((v_max - v_min) / 4));
    
                    solidFuseClass50.Perform(pnt50, 10e-6);
                    solidFuseClass25.Perform(pnt25, 10e-6);
//                     solidFuseClass75.Perform(pnt75, 10e-6);
    
                    if (solidFuseClass50.State() == TopAbs_OUT)
                    {
                        builder.Add(compFuselage, loftFace);
                    }
//                     else if (solidFuseClass75.State() == TopAbs_OUT)
//                     {
//                         builder.Add(compFuselage, loftFace);
//                     }
                    else if (solidFuseClass25.State() == TopAbs_OUT)
                    {
                        builder.Add(compFuselage, loftFace);
                    }

    
                    if (solidFuseClass50.State() == TopAbs_UNKNOWN)
                    {
                        LOG(ERROR) << "Error during Wing cutting with the fuselage.";
                        throw tigl::CTiglError("Error during Wing cutting with the fuselage.");
                    }

                }
                
                vFuselages[i].Shape = compFuselage;
                
                if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel && (fabs(wingYPos) > 10e-3)) 
                {

                    TopoDS_Shape fuseLoft = tigl::CTiglCommon::splitShape(vFuselages[i].Shape, nASSym.Solid);
        
                    faceMap.Clear();
                    TopExp::MapShapes(fuseLoft, TopAbs_FACE, faceMap);
        
                    builder.MakeCompound(compFuselage);

                    BRepClass3d_SolidClassifier solidFuseClass50(solid);
                    BRepClass3d_SolidClassifier solidFuseClass25(solid);
                    BRepClass3d_SolidClassifier solidFuseClass75(solid);
        
                    for (int f = 1; f <= faceMap.Extent(); f++) 
                    {
                        const TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
                        gp_Pnt pnt50, pnt25, pnt75;
                        Handle(Geom_Surface) surf = BRep_Tool::Surface(loftFace);
                        BRepTools::UVBounds(loftFace, u_min, u_max, v_min, v_max);
        
                        pnt50 = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));
                        pnt25 = surf->Value(u_min + ((u_max - u_min) / 4), v_min + ((v_max - v_min) / 4));
                        pnt75 = surf->Value(u_min + 3 * ((u_max - u_min) / 4), v_min + 3 * ((v_max - v_min) / 4));
        
                        solidFuseClass50.Perform(pnt50, 10e-6);
                        solidFuseClass25.Perform(pnt25, 10e-6);
    //                     solidFuseClass75.Perform(pnt75, 10e-6);
        
                        if (solidFuseClass50.State() == TopAbs_OUT)
                        {
                            builder.Add(compFuselage, loftFace);
                        }
    //                     else if (solidFuseClass75.State() == TopAbs_OUT)
    //                     {
    //                         builder.Add(compFuselage, loftFace);
    //                     }
                        else if (solidFuseClass25.State() == TopAbs_OUT)
                        {
                            builder.Add(compFuselage, loftFace);
                        }

        
                        if (solidFuseClass50.State() == TopAbs_UNKNOWN)
                        {
                            LOG(ERROR) << "Error during Wing cutting with the fuselage.";
                            throw tigl::CTiglError("Error during Wing cutting with the fuselage.");
                        }

                    }
                    
                    vFuselages[i].Shape = compFuselage;
                    
                }
    
            }
        }

    }


    // cut every Wing with every other wing
    // delete the surfaces of the first wing, which are in the second wing

    for (int a = 0; a < vWings.size(); a++)
        for (int b = 0; b < vWings.size(); b++)
            if (a == b)
                continue;
            else
            {
                Bnd_Box bBox1, bBox2;
                BRepBndLib::Add(vWings[a].Shape, bBox1);
                BRepBndLib::Add(vWings[b].Shape, bBox2);
                
                if(bBox1.IsOut(bBox2))
                    continue;
                
                TopoDS_Compound compWing;
                builder.MakeCompound(compWing);
                double u_min = 0.0, u_max = 0.0, v_min = 0.0, v_max = 0.0;

                TopoDS_Shape loft = CTiglCommon::splitShape(vWings[a].Shape, vWings[b].Shape);

                faceMap.Clear();
                TopExp::MapShapes(loft, TopAbs_FACE, faceMap);

                BRepClass3d_SolidClassifier solidFuseClass(vWings[b].Solid);

                for (int f = 1; f <= faceMap.Extent(); f++) 
                {
                    const TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
                    gp_Pnt pnt;
                    Handle(Geom_Surface) surf = BRep_Tool::Surface(loftFace);
                    BRepTools::UVBounds(loftFace, u_min, u_max, v_min, v_max);
                    pnt = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));
                    solidFuseClass.Perform(pnt, 10e-6);

                    if (solidFuseClass.State() == TopAbs_OUT)
                    {
                        builder.Add(compWing, loftFace);
                    }

                    if (solidFuseClass.State() == TopAbs_UNKNOWN)
                    {
                        LOG(ERROR) << "Error during Wing - Wing cut.";
                        throw CTiglError("Error during Wing - Wing cut.");
                    }
                }

                vWings[a].Shape = compWing;
            }

            // declare variables for step export

            ListPNamedShape shapes;

            for (int i = 0; i < vFuselages.size(); i++)
            {
                PNamedShape shape(new CNamedShape(vFuselages[i].Shape, vFuselages[i].Name.c_str()));
                shapes.push_back(shape);
            }

            // Write Wings with names to Step File

            for (int j = 0; j < vWings.size(); j++)
            {
                PNamedShape shape(new CNamedShape(vWings[j].Shape, vWings[j].Name.c_str()));
                shapes.push_back(shape);
            }

            // Write FreeFormSurfaces to Step File

            for (int k = 0; k < vFreeFormSurfs.size(); k++)
            {
                PNamedShape shape(new CNamedShape(vFreeFormSurfs[k].Shape, vFreeFormSurfs[k].Name.c_str()));
                shapes.push_back(shape);
            }

            // write step
            try {
                ExportShapes(shapes, filename);
            }
            catch (CTiglError&) 
            {
                LOG(ERROR) << "Cannot export airplane in CTiglExportStep";
                throw CTiglError("Cannot export airplane in CTiglExportStep", TIGL_ERROR);
            }

}


} // end namespace tigl
