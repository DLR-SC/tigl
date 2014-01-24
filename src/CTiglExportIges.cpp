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

// IGES export
#include <TransferBRep_ShapeMapper.hxx>
#include <Transfer_FinderProcess.hxx>
#include <TransferBRep.hxx>
#include <IGESData_IGESEntity.hxx>

#ifdef TIGL_USE_XCAF
// OCAF
#include <TDocStd_Document.hxx>
#include <TDF_Label.hxx>
#include <TDataStd_Name.hxx>
// XCAF, TODO: Get rid of xcaf
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include "IGESCAFControl_Writer.hxx"
#endif

#include <map>
#include <cassert>

namespace {

    /**
     * @brief WriteIGESFaceNames takes the names of each face and writes it into the IGES model.
     */
    void WriteIGESFaceNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape) {
        if(!shape) {
            return;
        }

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
        for(int iface = 1; iface <= faceMap.Extent(); ++iface) {
            TopoDS_Face face = TopoDS::Face(faceMap(iface));
            std::string name = shape->ShortName();
            PNamedShape origin = shape->GetFaceTraits(iface-1).Origin();
            if (origin) {
                name = origin->ShortName();
            }
            // set face name
            Handle(IGESData_IGESEntity) entity;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, face );
            if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(IGESData_IGESEntity), entity ) ) {
                Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(name.c_str());
                entity->SetLabel(str);
            }
        }
    }

} //namespace

namespace tigl {

// Constructor
CTiglExportIges::CTiglExportIges(CCPACSConfiguration& config)
:myConfig(config)
{
    myStoreType = NAMED_COMPOUNDS;
}

// Destructor
CTiglExportIges::~CTiglExportIges(void)
{
}

void CTiglExportIges::SetTranslationParamters() const
{
    Interface_Static::SetCVal("xstep.cascade.unit", "M");
    Interface_Static::SetCVal("write.iges.unit", "M");
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
    IGESControl_Controller::Init();

    IGESControl_Writer igesWriter;
    SetTranslationParamters();
    igesWriter.Model()->ApplyStatic(); // apply set parameters

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportFusedIGES.";
       return;
    }

    // Export all wings of the configuration
    for (int w = 1; w <= myConfig.GetWingCount(); w++) {
        CCPACSWing& wing = myConfig.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();

            igesWriter.AddShape(loft);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= myConfig.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = myConfig.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();

            // Transform loft by fuselage transformation => absolute world coordinates
            loft = fuselage.GetFuselageTransformation().Transform(loft);

            igesWriter.AddShape(loft);
        }
    }

    if (myConfig.GetFarField().GetFieldType() != NONE) {
        igesWriter.AddShape(myConfig.GetFarField().GetLoft());
    }

    // Write IGES file
    igesWriter.ComputeModel();
    if (igesWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True) {
        throw CTiglError("Error: Export to IGES file failed in CTiglExportIges::ExportIGES", TIGL_ERROR);
    }
}


// Exports the whole configuration as one fused part to an IGES file
void CTiglExportIges::ExportFusedIGES(const std::string& filename)
{
    if (filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportFusedIGES.";
       return;
    }

    PTiglFusePlane fuser = myConfig.AircraftFusingAlgo();
    fuser->SetResultMode(HALF_PLANE);
    assert(fuser);

    PNamedShape fusedAirplane = fuser->NamedShape();
    if (!fusedAirplane) {
        throw CTiglError("Error computing fused airplane.", TIGL_NULL_POINTER);
    }

    IGESControl_Controller::Init();
    SetTranslationParamters();
#ifdef TIGL_USE_XCAF
    // create the xde document
    Handle(XCAFApp_Application) hApp = XCAFApp_Application::GetApplication();
    Handle(TDocStd_Document) hDoc;
    hApp->NewDocument("MDTV-XCAF", hDoc);
    Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(hDoc->Main());

    IGESCAFControl_Writer igesWriter;
    GroupAndInsertShapeToCAF(myAssembly, fusedAirplane);

    igesWriter.Model()->ApplyStatic(); // apply set parameters
    if (igesWriter.Transfer(hDoc) == Standard_False) {
        throw CTiglError("Cannot export fused airplane as IGES", TIGL_ERROR);
    }
#else
    IGESControl_Writer igesWriter;
    igesWriter.Model()->ApplyStatic();
    igesWriter.AddShape(fusedAirplane->Shape());
#endif

    WriteIGESFaceNames(igesWriter.TransferProcess(), fusedAirplane);

    if (igesWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True) {
        throw CTiglError("Error: Export fused shapes to IGES file failed in CTiglExportIges::ExportFusedIGES", TIGL_ERROR);
    }
}




// Save a sequence of shapes in IGES Format
void CTiglExportIges::ExportShapes(const Handle(TopTools_HSequenceOfShape)& aHSequenceOfShape, const std::string& filename)
{
    IGESControl_Controller::Init();
    IGESControl_Writer igesWriter;
    SetTranslationParamters();
    igesWriter.Model()->ApplyStatic(); // apply set parameters

    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportShapes.";
       return;
    }

    for (Standard_Integer i=1;i<=aHSequenceOfShape->Length();i++) {
        igesWriter.AddShape (aHSequenceOfShape->Value(i));
    }

    igesWriter.ComputeModel();
    if (igesWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True)
        throw CTiglError("Error: Export of shapes to IGES file failed in CCPACSImportExport::SaveIGES", TIGL_ERROR);
}

void CTiglExportIges::SetOCAFStoreType(IgesOCAFStoreType type)
{
    myStoreType = type;
}

#ifdef TIGL_USE_XCAF
// Saves as iges, with cpacs metadata information in it
void CTiglExportIges::ExportIgesWithCPACSMetadata(const std::string& filename)
{
       if( filename.empty()) {
           LOG(ERROR) << "Error: Empty filename in ExportIgesWithCPACSMetadata.";
           return;
       }

       CCPACSImportExport generator(myConfig);
       Handle(TDocStd_Document) hDoc = generator.buildXDEStructure();

       IGESControl_Controller::Init();
       IGESCAFControl_Writer writer;
       SetTranslationParamters();
       writer.Model()->ApplyStatic(); // apply set parameters

       writer.Transfer(hDoc);
       writer.Write(filename.c_str());
   }



void CTiglExportIges::GroupAndInsertShapeToCAF(Handle(XCAFDoc_ShapeTool) myAssembly, const PNamedShape shape)
{
    if(!shape) {
        return;
    }

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
    if (faceMap.Extent() > 0) {
        if (myStoreType == WHOLE_SHAPE){
            TDF_Label shapeLabel = myAssembly->NewShape();
            myAssembly->SetShape(shapeLabel, shape->Shape());
            TDataStd_Name::Set(shapeLabel, shape->Name());
        }
        else if (myStoreType == NAMED_COMPOUNDS) {
            // create compounds with the same name as origin
            ShapeMap map =  MapFacesToShapeGroups(shape);
            // add compounds to document
            ShapeMap::iterator it = map.begin();
            for (; it != map.end(); ++it){
                TDF_Label faceLabel = myAssembly->NewShape();
                myAssembly->SetShape(faceLabel, it->second);
                TDataStd_Name::Set(faceLabel, it->first.c_str());
            }
        }
        else if (myStoreType == FACES) {
            for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
                TopoDS_Face face = TopoDS::Face(faceMap(iface));
                std::string name = shape->ShortName();
                PNamedShape origin = shape->GetFaceTraits(iface-1).Origin();
                if(origin){
                    name = origin->ShortName();
                }
                TDF_Label faceLabel = myAssembly->NewShape();
                myAssembly->SetShape(faceLabel, face);
                TDataStd_Name::Set(faceLabel, name.c_str());
            }
        }
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
            TDF_Label wireLabel = myAssembly->NewShape();
            myAssembly->SetShape(wireLabel, Edges->Value(iwire));
            TDataStd_Name::Set(wireLabel, shape->Name());
        }
    }
}

#endif

} // end namespace tigl
