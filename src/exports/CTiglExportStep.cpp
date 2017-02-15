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

#define STEP_WRITEMODE STEPControl_AsIs

namespace
{

    /**
     * @brief WriteSTEPProductName writes the shape names as the step product identifier
     */
    void WriteStepProductName(Handle(Transfer_FinderProcess) FP, const PNamedShape shape)
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
    
    void WriteStepSolidName(Handle(Transfer_FinderProcess) FP, const PNamedShape shape)
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
    
    void WriteStepWireName(Handle(Transfer_FinderProcess) FP, const PNamedShape shape)
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
                Handle(StepGeom_Curve) curve = Handle(StepGeom_Curve)::DownCast(elem.Value());
                
                if (!curve.IsNull()) {
                    curve->SetName(str);
                }
                
                // CATIA does only show the basis curves. Hence we must name them too
                Handle(StepGeom_TrimmedCurve) tcurve = Handle(StepGeom_TrimmedCurve)::DownCast(elem.Value());
                if (!tcurve.IsNull() && !tcurve->BasisCurve().IsNull()) {
                    tcurve->BasisCurve()->SetName(str);
                }
            }
        }
    }
    
    void WriteStepShellName(Handle(Transfer_FinderProcess) FP, const PNamedShape shape)
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
    void WriteStepFaceNames(Handle(Transfer_FinderProcess) FP, const PNamedShape shape)
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

    void WriteStepNames(Handle(Transfer_FinderProcess) FP, const PNamedShape shape)
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
CTiglExportStep::CTiglExportStep()
{
    SetGroupMode(NAMED_COMPOUNDS);
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


// Save a sequence of shapes in STEP Format
bool CTiglExportStep::WriteImpl(const std::string& filename) const
{
    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in CTiglExportStep::Write.";
       return false;
    }

    STEPControl_Controller::Init();
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    Interface_Static::SetCVal("write.step.unit", "M");

    ListPNamedShape list;
    for (size_t ishape = 0; ishape < NShapes(); ++ishape) {
        ListPNamedShape templist = GroupFaces(GetShape(ishape), _groupMode);
        for (ListPNamedShape::iterator it2 = templist.begin(); it2 != templist.end(); ++it2) {
            list.push_back(*it2);
        }
    }

    STEPControl_Writer stepWriter;

    for (ListPNamedShape::iterator it = list.begin(); it != list.end(); ++it) {
        PNamedShape pshape = *it;
        AddToStep(pshape, stepWriter);
    }

    return stepWriter.Write(const_cast<char*>(filename.c_str())) <= IFSelect_RetDone;
}

void CTiglExportStep::SetGroupMode(ShapeGroupMode mode)
{
    _groupMode = mode;
}

} // end namespace tigl
