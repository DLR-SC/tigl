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
* @brief  Export routines for CPACS configurations.
*/

#include "CTiglExportIges.h"

#include "CCPACSImportExport.h"
#include "CCPACSConfiguration.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglError.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"
#include "CTiglFusePlane.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Edge.hxx"
#include "Standard_CString.hxx"
#include "IGESControl_Controller.hxx"
#include "IGESControl_Writer.hxx"
#include "IGESData_IGESModel.hxx"
#include "Interface_Static.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_HSequenceOfShape.hxx>

// IGES export
#include <TransferBRep_ShapeMapper.hxx>
#include <Transfer_FinderProcess.hxx>
#include <TransferBRep.hxx>
#include <IGESData_IGESEntity.hxx>

// [[CAS_AES]] added ribs definition
#include "CCPACSWingRibsDefinition.h"
// [[CAS_AES]] added for IGES level export
#include "CTiglIGESWriter.h"
// [[CAS_AES]] added for symmetry functionality
#include <BRepBuilderAPI_Transform.hxx>
// [[CAS_AES]] added include for common geometry functionality
#include "CTiglCommon.h"

#include <map>
#include <cassert>

namespace
{

    /**
     * @brief WriteIGESFaceNames takes the names of each face and writes it into the IGES model.
     */
    void WriteIGESFaceNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape, int level)
    {
        if (!shape) {
            return;
        }

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
        for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
            TopoDS_Face face = TopoDS::Face(faceMap(iface));
            std::string faceName = shape->GetFaceTraits(iface-1).Name();
            
            if (faceName == shape->Name()) {
                faceName = shape->ShortName();
            }
            
            PNamedShape origin = shape->GetFaceTraits(iface-1).Origin();
            if (origin && origin->Name() == faceName) {
                faceName = origin->ShortName();
            }
            
            // IGES allows entity names of at max 8 characters.
            // If the string is longer than 8 characters, the IGES exports might crash
            if (faceName.length() > 8) {
                faceName = faceName.substr(0,8);
            }

            // set face name
            Handle(IGESData_IGESEntity) entity;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, face );
            if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(IGESData_IGESEntity), entity ) ) {
                Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(faceName.c_str());
                entity->SetLabel(str);
                entity->InitLevel(0, level);
            }
        }
    }
    
    void WriteIGESShapeNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape, int level)
    {
        if (!shape) {
            return;
        }

        std::string shapeName = shape->ShortName();
        // IGES allows entity names of at max 8 characters.
        // If the string is longer than 8 characters, the IGES exports might crash
        if (shapeName.length() > 8) {
            shapeName = shapeName.substr(0,8);
        }

        // insert blanks
        int nblanks = 8 - shapeName.length();
        for (int i = 0; i < nblanks; ++i) {
            shapeName.insert(shapeName.begin(), ' ');
        }

        // set face name
        Handle(IGESData_IGESEntity) entity;
        Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, shape->Shape() );
        if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(IGESData_IGESEntity), entity ) ) {
            Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(shapeName.c_str());
            entity->SetLabel(str);
            entity->InitLevel(0, level);
        }
    }
    
    void WriteIgesWireName(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }
        
        TopTools_IndexedMapOfShape wireMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_EDGE, wireMap);
        for (int iwire = 1; iwire <= wireMap.Extent(); ++iwire) {
            TopoDS_Edge wire = TopoDS::Edge(wireMap(iwire));
            std::string wireName = shape->ShortName();

            // set wire name
            Handle(IGESData_IGESEntity) entity;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, wire );
            if (!FP->FindTypedTransient ( mapper, STANDARD_TYPE(IGESData_IGESEntity), entity ) ) {
                continue;
            }
            
            Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(wireName.c_str());
            entity->SetLabel(str);
        }
    }
    
    void WriteIgesNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape, int level)
    {
        WriteIGESFaceNames(FP, shape, level);
        WriteIGESShapeNames(FP, shape, level);
    }

} //namespace

namespace tigl
{

// Constructor
CTiglExportIges::CTiglExportIges(CCPACSConfiguration& config)
    : _config(config)
{
    _groupMode = NAMED_COMPOUNDS;
}

// Destructor
CTiglExportIges::~CTiglExportIges(void)
{
}

void CTiglExportIges::SetTranslationParameters() const
{
    Interface_Static::SetCVal("xstep.cascade.unit", "MM");
    Interface_Static::SetCVal("write.iges.unit", "MM");
    /*
     * BRep entities in IGES are experimental and untested.
     * They allow to specify things like shells and solids.
     * It seems, that CATIA does not support these entities.
     * Hence we stay the compatible way.
     */
    Interface_Static::SetIVal("write.iges.brep.mode", 0);
    Interface_Static::SetCVal("write.iges.header.author", "TiGL");
    Interface_Static::SetCVal("write.iges.header.company", "German Aerospace Center (DLR), SC");
}

// Exports the whole configuration as IGES file
// All wing- and fuselage segments are exported as single bodys
void CTiglExportIges::ExportIGES(const std::string& filename) const
{
    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportIGES.";
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

    // write iges
    try {
        ExportShapes(shapes, filename);
    }
    catch (CTiglError&) {
        throw CTiglError("Cannot export airplane in CTiglExportIges", TIGL_ERROR);
    }
}


// Exports the whole configuration as one fused part to an IGES file
void CTiglExportIges::ExportFusedIGES(const std::string& filename)
{
    if (filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportFusedIGES.";
       return;
    }

    PTiglFusePlane fuser = _config.AircraftFusingAlgo();
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
        throw CTiglError("Cannot export fused Airplane as IGES", TIGL_ERROR);
    }
}

// [[CAS_AES]] Export the whole configuration including structure
void CTiglExportIges::ExportStructureIGES(const std::string& filename) const
{
    // [[CAS_AES]] Required for constructor of IGES writer, without we get errors on setting the units
    IGESControl_Controller::Init();
    // [[CAS_AES]] Use CTiglIGESWriter
    CTiglIGESWriter igesWriter;

    Interface_Static::SetIVal("write.surfacecurve.mode", 0);
    Interface_Static::SetIVal("write.iges.brep.mode", 1);
    Interface_Static::SetCVal("write.iges.header.author", "Descartes");
    Interface_Static::SetCVal("write.iges.header.company", "Airbus Defence and Space");

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
//                 TopoDS_Shape loft = segment.GetLoft();
            TopoDS_Shape loft = segment.GetSplittedLoft(segmentType);
            // [[CAS_AES]] added symmetry handling
            if (symmetryAxis != TIGL_NO_SYMMETRY) {
                loft = CTiglCommon::mirrorShape(loft, symmetryAxis);
            }

            // [[CAS_AES]] added level information
            igesWriter.AddShape(loft, 2*w + 2);
        }

        // [[CAS_AES]] export componentSegments
        for (int i=1; i <= wing.GetComponentSegmentCount(); i++)
        {
            tigl::CCPACSWingComponentSegment& segment = (tigl::CCPACSWingComponentSegment &) wing.GetComponentSegment(i);
            // [[CAS_AES]] export of spars
            for (int j = 1; j <= segment.GetSparSegmentCount(); j++) {
                tigl::CCPACSWingSparSegment& sparSegment = segment.GetSparSegment(j);
                // get spar geometry splitted with ribs
                TopoDS_Shape sparSegmentGeometry = sparSegment.GetSplittedSparGeometry();
                if (symmetryAxis != TIGL_NO_SYMMETRY) {
                    sparSegmentGeometry = CTiglCommon::mirrorShape(sparSegmentGeometry, symmetryAxis);
                }

                igesWriter.AddShape(sparSegmentGeometry, 2*w + 1);

                if (sparSegment.hasCaps())
                {
                    // get spar caps geometry
                    TopoDS_Shape sparCapsGeometry = sparSegment.GetSparCapsGeometry(sparSegment.UPPER);
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        sparCapsGeometry = CTiglCommon::mirrorShape(sparCapsGeometry, symmetryAxis);
                    }

                    igesWriter.AddShape(sparCapsGeometry, 2*w + 1);

                    // get spar caps geometry
                    sparCapsGeometry = sparSegment.GetSparCapsGeometry(sparSegment.LOWER);
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        sparCapsGeometry = CTiglCommon::mirrorShape(sparCapsGeometry, symmetryAxis);
                    }

                    igesWriter.AddShape(sparCapsGeometry, 2*w + 1);
                }

                // get spar reinforcements
                if (sparSegment.HasStringerReinforcements())
                {
                    TopoDS_Shape sparStringer = sparSegment.GetStringerReinforcementGeometry();
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        sparStringer = CTiglCommon::mirrorShape(sparStringer, symmetryAxis);
                    }
                    igesWriter.AddShape(sparStringer, 2*w + 1);
                }

                if (sparSegment.HasFramesReinforcements())
                {
                    TopoDS_Shape sparFrames = sparSegment.GetFramesReinforcementGeometry();
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        sparFrames = CTiglCommon::mirrorShape(sparFrames, symmetryAxis);
                    }
                    igesWriter.AddShape(sparFrames, 2*w + 1);
                }
            }
            // [[CAS_AES]] export of ribs
            for (int j = 1; j <= segment.GetRibsDefinitionCount(); j++) {
                tigl::CCPACSWingRibsDefinition& ribsDefinition = segment.GetRibsDefinition(j);
                // get ribs geometry splitted with spars
                TopoDS_Shape ribsGeometry = ribsDefinition.GetSplittedRibsGeometry();
                if (symmetryAxis != TIGL_NO_SYMMETRY) {
                    ribsGeometry = CTiglCommon::mirrorShape(ribsGeometry, symmetryAxis);
                }

                igesWriter.AddShape(ribsGeometry, 2*w + 1);

                // get ribs cap geometry
                if (ribsDefinition.HasCaps())
                {
                    TopoDS_Shape ribCapsGeometry = ribsDefinition.GetRibCapsGeometry(ribsDefinition.UPPER);
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        ribCapsGeometry = CTiglCommon::mirrorShape(ribCapsGeometry, symmetryAxis);
                    }

                    igesWriter.AddShape(ribCapsGeometry, 2*w + 1);

                    ribCapsGeometry = ribsDefinition.GetRibCapsGeometry(ribsDefinition.LOWER);
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        ribCapsGeometry = CTiglCommon::mirrorShape(ribCapsGeometry, symmetryAxis);
                    }

                    igesWriter.AddShape(ribCapsGeometry, 2*w + 1);
                }

                // get ribs reinforcements
                if (ribsDefinition.HasStringerReinforcements())
                {
                    TopoDS_Shape ribStringer = ribsDefinition.GetStringerReinforcementGeometry();
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        ribStringer = CTiglCommon::mirrorShape(ribStringer, symmetryAxis);
                    }
                    igesWriter.AddShape(ribStringer, 2*w + 1);
                }

                if (ribsDefinition.HasFramesReinforcements())
                {

                    TopoDS_Shape ribFrames = ribsDefinition.GetFramesReinforcementGeometry();
                    if (symmetryAxis != TIGL_NO_SYMMETRY) {
                        ribFrames = CTiglCommon::mirrorShape(ribFrames, symmetryAxis);
                    }
                    igesWriter.AddShape(ribFrames, 2*w + 1);

                }
            }

            // [[CAS_AES]] Stringer geometry for the upper shell
            tigl::CCPACSWingShell& upperShell = segment.GetUpperShell();
            if (upperShell.HasStringer()) 
            {
                TopoDS_Shape upperStringerGeometry = upperShell.GetStringerGeometry();
                
                if (symmetryAxis != TIGL_NO_SYMMETRY) 
                {
                    upperStringerGeometry = CTiglCommon::mirrorShape(upperStringerGeometry, symmetryAxis);
                }

                igesWriter.AddShape(upperStringerGeometry,  2*w + 1);
    }

            // [[CAS_AES]] Stringer geometry for the lower shell
            tigl::CCPACSWingShell& lowerShell = segment.GetLowerShell();
            if (lowerShell.HasStringer()) 
            {
                TopoDS_Shape lowerStringerGeometry = lowerShell.GetStringerGeometry();

                if (symmetryAxis != TIGL_NO_SYMMETRY) 
                {
                    lowerStringerGeometry = CTiglCommon::mirrorShape(lowerStringerGeometry, symmetryAxis);
                }
                igesWriter.AddShape(lowerStringerGeometry, 2*w + 1);
            }
        }
    }
    
    bool exportDoorGeometry = false;

    // Export all fuselages of the configuration
    for (int f = 1; f <= _config.GetFuselageCount(); f++)
    {
        CCPACSFuselage& fuselage = _config.GetFuselage(f);

        TiglSymmetryAxis symmetryAxis = fuselage.GetSymmetryAxis();

        TopoDS_Compound compound;
        BRep_Builder Builder;
        Builder.MakeCompound(compound);

        // [[CAS_AES]] added support for fuselage structure
        CCPACSFuselageStructure& structure = fuselage.GetFuselageStructure();

        
        for(int i = 1;i <= structure.GetBulkheadCount();i++)            //pressureBulkheads
        {
            tigl::CCPACSPressureBulkhead* pressureBulkhead = structure.getPressureBulkhead(i);

            TopoDS_Shape verticalReinforcementShape = pressureBulkhead->getSplittedVerticalReinforcementGeometry();     //pressureBulkheadVerticalReinforcement
            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                verticalReinforcementShape = tigl::CTiglCommon::mirrorShape(verticalReinforcementShape, symmetryAxis);
            }

            igesWriter.AddShape(verticalReinforcementShape, 1);
            
            TopoDS_Shape horizontalReinforcementShape = pressureBulkhead->getSplittedHorizontalReinforcementGeometry();     //pressureBulkheadHorizontalReinforcement
            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                horizontalReinforcementShape = tigl::CTiglCommon::mirrorShape(horizontalReinforcementShape, symmetryAxis);
            }

            igesWriter.AddShape(horizontalReinforcementShape, 1);
            
            TopoDS_Shape bulkheadShape = pressureBulkhead->getSplitGeometry();      //pressureBulkHead

            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                bulkheadShape = tigl::CTiglCommon::mirrorShape(bulkheadShape, symmetryAxis);
            }

            igesWriter.AddShape(bulkheadShape, 1);
            

        }
        
        // [[CAS_AES]] added support for fuselage stringer
        for (int i = 1; i <= structure.GetStringerCount(); i++)                 //stringers
        {
            // get the stringer
            CCPACSFuselageStringer* stringer = structure.GetStringer(i);
            // get the geometric structure of the stringer
            TopoDS_Shape shape = stringer->GetSplitGeometry(!stringer->GeomState_3D());

            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                shape = tigl::CTiglCommon::mirrorShape(shape, symmetryAxis);
            }

            igesWriter.AddShape(shape, 1);
        }
        for (int i = 1; i <= structure.GetCargoCrossBeamStrutCount(); i++)          //CrossbeamStrut
        {
            // get the CrossbeamStrut
            tigl::CCPACSCrossBeamStrut* crossBeamStrut =  structure.GetCargoCrossBeamStrut(i);
            // get the geometric structure of the CrossbeamStrut
            TopoDS_Shape shape = crossBeamStrut->getGeometry(!crossBeamStrut->GeomState_3D());

            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                shape = tigl::CTiglCommon::mirrorShape(shape, symmetryAxis);
            }

            igesWriter.AddShape(shape, 1);
        }
        
        for (int i = 1; i <= structure.GetLongFloorBeamCount(); i++)          //longFloorBeam
        {
            // get the longFloorBeam
            tigl::CCPACSLongFloorBeam* longFloorBeam =  structure.GetLongFloorBeam(i);
            // get the geometric structure of the longFloorBeam
            TopoDS_Shape shape =  longFloorBeam->getGeometry(!longFloorBeam->GeomState_3D());

            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                shape = tigl::CTiglCommon::mirrorShape(shape, symmetryAxis);
            }

            igesWriter.AddShape(shape, 1);
        }
        
        for (int i = 1; i <= structure.GetCargoCrossBeamCount(); i++)          //Crossbeam
        {
            // get the Crossbeam
            tigl::CCPACSCrossBeam* crossBeam =  structure.GetCargoCrossBeam(i);
            // get the geometric structure of the Crossbeam
            TopoDS_Shape shape =  crossBeam->getCuttedGeometry(!crossBeam->GeomState_3D());

            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                shape = tigl::CTiglCommon::mirrorShape(shape, symmetryAxis);
            }

            igesWriter.AddShape(shape, 1);
        }
        
        for (int i = 1; i <= structure.GetFrameCount(); i++)            //frames
        {
            // get the frame
            CCPACSFuselageFrame* frame = structure.GetFrame(i);
            // get the geometric structure of the frame
            TopoDS_Shape shape = frame->GetSplitGeometry(!frame->GeomState_3D());

            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                shape = tigl::CTiglCommon::mirrorShape(shape, symmetryAxis);
            }

            igesWriter.AddShape(shape, 1);
        }
        
        
        for (int i = 1; i <= fuselage.GetSegmentCount(); i++)           //fuselageLoft
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

            for(int i = 1;i <= structure.GetDoorCount();i++)
            {
                tigl::CCPACSDoorType* door = structure.getDoorType(i);

                // the next lines display the door geometry
                if (exportDoorGeometry)
                {
                    TopoDS_Shape doorShape = door->getGeometry();
                    
                    if (symmetryAxis != TIGL_NO_SYMMETRY) 
                    {
                        doorShape = tigl::CTiglCommon::mirrorShape(doorShape, symmetryAxis);
                    }

                    igesWriter.AddShape(doorShape, 1);
                }

                // Here we cut the doors out of the fuselage geometry
                loft = door->removeDoor(loft);

            }

            if (symmetryAxis != TIGL_NO_SYMMETRY) 
            {
                loft = tigl::CTiglCommon::mirrorShape(loft, symmetryAxis);
            }
            
            // [[CAS_AES]] added level information
            igesWriter.AddShape(loft, 2);
        }
        
    }

    // [[CAS_AES]] Export FreeFormSurfaces of the configuration
    for (int r = 1; r<= _config.GetFFFSCount(); r++)
    {
        tigl::CCPACSFreeFormSurface& FFS = _config.GetFFFSbyIndex(r);
        // boolean for half model
        igesWriter.AddShape(FFS.getShape(false), 90+r);
    }

    // Write IGES file
    igesWriter.ComputeModel();
    if (igesWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True)
        throw CTiglError("Error: Export to IGES file failed in CTiglExportIges::ExportIGES", TIGL_ERROR);
}


// Save a sequence of shapes in IGES Format
void CTiglExportIges::ExportShapes(const ListPNamedShape& shapes, const std::string& filename) const
{
    IGESControl_Controller::Init();

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportShapes.";
       return;
    }

    ListPNamedShape::const_iterator it;
    // scale all shapes to mm
    ListPNamedShape shapeScaled;
    for (it = shapes.begin(); it != shapes.end(); ++it) {
        PNamedShape pshape = *it;
        if (pshape) {
            CTiglTransformation trafo;
            trafo.AddScaling(1000,1000,1000);
            PNamedShape pScaledShape(new CNamedShape(*pshape));
            pScaledShape->SetShape(trafo.Transform(pshape->Shape()));
            shapeScaled.push_back(pScaledShape);
        }
    }
    
    ListPNamedShape list;
    for (it = shapeScaled.begin(); it != shapeScaled.end(); ++it) {
        ListPNamedShape templist = GroupFaces(*it, _groupMode);
        for (ListPNamedShape::iterator it2 = templist.begin(); it2 != templist.end(); ++it2) {
            list.push_back(*it2);
        }
    }
    
    SetTranslationParameters();

    IGESControl_Writer igesWriter("MM", 0);
    igesWriter.Model()->ApplyStatic();

    int level = 0;
    for (it = list.begin(); it != list.end(); ++it) {
        PNamedShape pshape = *it;
        AddToIges(pshape, igesWriter, level++);
    }

    igesWriter.ComputeModel();

    if (igesWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True) {
        throw CTiglError("Error: Export of shapes to IGES file failed in CCPACSImportExport::SaveIGES", TIGL_ERROR);
    }
}

void CTiglExportIges::SetGroupMode(ShapeGroupMode mode)
{
    _groupMode = mode;
}

/**
 * @briefAdds a shape to the IGES file. All faces are named according to their face
 * traits. If there are no faces, the wires are named according to the shape name.
 * 
 * The level parameter defines the iges layer/level, which is another way of grouping faces.
 */
void CTiglExportIges::AddToIges(PNamedShape shape, IGESControl_Writer& writer, int level) const 
{
    if (!shape) {
        return;
    }
    
    std::string shapeName = shape->Name();
    std::string shapeShortName = shape->ShortName();
    Handle(Transfer_FinderProcess) FP = writer.TransferProcess();
    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
    // any faces?
    if (faceMap.Extent() > 0) {
        int ret = writer.AddShape(shape->Shape());
        if (ret > IFSelect_RetDone) {
            throw CTiglError("Error: Export to IGES file failed in CTiglExportStep. Could not translate shape " 
                             + shapeName + " to iges entity,", TIGL_ERROR);
        }
        WriteIgesNames(FP, shape, level);
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
            int ret = writer.AddShape(Edges->Value(iwire));
            if (ret > IFSelect_RetDone) {
                throw CTiglError("Error: Export to IGES file failed in CTiglExportIges. Could not translate shape " 
                                 + shapeName + " to iges entity,", TIGL_ERROR);
            }
            PNamedShape theWire(new CNamedShape(Edges->Value(iwire),shapeName.c_str()));
            theWire->SetShortName(shapeShortName.c_str());
            WriteIGESShapeNames(FP, theWire, level);
            WriteIgesWireName(FP, theWire);
        }
    }
}

} // end namespace tigl
