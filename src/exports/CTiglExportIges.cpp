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
CTiglExportIges::CTiglExportIges()
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
void CTiglExportIges::AddConfiguration(CCPACSConfiguration& config)
{
    // Export all wings of the configuration
    for (int w = 1; w <= config.GetWingCount(); w++) {
        CCPACSWing& wing = config.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            AddShape(loft);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= config.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = config.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            AddShape(loft);
        }
    }

    CCPACSFarField& farfield = config.GetFarField();
    if (farfield.GetFieldType() != NONE) {
        AddShape(farfield.GetLoft());
    }
}


// Exports the whole configuration as one fused part to an STEP file
void CTiglExportIges::AddFusedConfiguration(CCPACSConfiguration& config)
{
    PTiglFusePlane fuser = config.AircraftFusingAlgo();
    fuser->SetResultMode(HALF_PLANE_TRIMMED_FF);
    assert(fuser);

    PNamedShape fusedAirplane = fuser->FusedPlane();
    PNamedShape farField      = fuser->FarField();
    if (!fusedAirplane) {
        throw CTiglError("Error computing fused airplane.", TIGL_NULL_POINTER);
    }

    AddShape(fusedAirplane);
    AddShape(farField);

    // add intersections
    const ListPNamedShape& ints = fuser->Intersections();
    ListPNamedShape::const_iterator it;
    for (it = ints.begin(); it != ints.end(); ++it) {
        AddShape(*it);
    }
}

void CTiglExportIges::AddShape(PNamedShape shape)
{
    if (shape) {
        _shapes.push_back(shape);
    }
}


// Save a sequence of shapes in IGES Format
bool CTiglExportIges::Write(const std::string& filename) const
{
    if (filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in CTiglExportIges::Write.";
       return false;
    }

    IGESControl_Controller::Init();

    // scale all shapes to mm
    ListPNamedShape shapeScaled;
    ListPNamedShape::const_iterator it;
    for (it = _shapes.begin(); it != _shapes.end(); ++it) {
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

    return igesWriter.Write(const_cast<char*>(filename.c_str()));
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
