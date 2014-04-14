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
#endif // TIGL_USE_XCAF

#include <map>
#include <cassert>

namespace
{

    /**
     * @brief WriteIGESFaceNames takes the names of each face and writes it into the IGES model.
     */
    void WriteIGESFaceNames(Handle_Transfer_FinderProcess FP, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
        for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
            TopoDS_Face face = TopoDS::Face(faceMap(iface));
            std::string faceName = shape->GetFaceTraits(iface-1).Name();
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
            }
        }
    }

} //namespace

namespace tigl
{

// Constructor
CTiglExportIges::CTiglExportIges(CCPACSConfiguration& config)
    : myConfig(config)
{
    myStoreType = NAMED_COMPOUNDS;
}

// Destructor
CTiglExportIges::~CTiglExportIges(void)
{
}

void CTiglExportIges::SetTranslationParameters() const
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
    if ( filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in ExportIGES.";
       return;
    }

    ListPNamedShape shapes;

    // Export all wings of the configuration
    for (int w = 1; w <= myConfig.GetWingCount(); w++) {
        CCPACSWing& wing = myConfig.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();
            // make short name
            std::stringstream s;
            s << "W" << w  << "S" << i;
            PNamedShape shape(new CNamedShape(loft, s.str().c_str()));
            shapes.push_back(shape);
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= myConfig.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = myConfig.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();
            // make short name
            std::stringstream s;
            s << "F" << f  << "S" << i;
            PNamedShape shape(new CNamedShape(loft, s.str().c_str()));
            shapes.push_back(shape);
        }
    }

    CCPACSFarField& farfield = myConfig.GetFarField();
    if (farfield.GetFieldType() != NONE) {
        PNamedShape shape(new CNamedShape(farfield.GetLoft(), farfield.GetUID().c_str()));
        shapes.push_back(shape);
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
        throw CTiglError("Cannot export fused Airplane as IGES", TIGL_ERROR);
    }
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
    SetTranslationParameters();
#ifdef TIGL_USE_XCAF
    // create the xde document
    Handle(XCAFApp_Application) hApp = XCAFApp_Application::GetApplication();
    Handle(TDocStd_Document) hDoc;
    hApp->NewDocument("MDTV-XCAF", hDoc);
    Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(hDoc->Main());

    IGESCAFControl_Writer igesWriter;

    ListPNamedShape list;
    for (it = shapes.begin(); it != shapes.end(); ++it) {
        ListPNamedShape templist = GroupFaces(*it, myStoreType);
        for (ListPNamedShape::iterator it2 = templist.begin(); it2 != templist.end(); ++it2) {
            list.push_back(*it2);
        }
    }

    for (it = list.begin(); it != list.end(); ++it) {
        InsertShapeToCAF(myAssembly, *it, true);
    }

    igesWriter.Model()->ApplyStatic(); // apply set parameters
    if (igesWriter.Transfer(hDoc) == Standard_False) {
        throw CTiglError("Cannot export fused airplane as IGES", TIGL_ERROR);
    }
#else
    IGESControl_Writer igesWriter;
    igesWriter.Model()->ApplyStatic();

    for (it = shapes.begin(); it != shapes.end(); ++it) {
        PNamedShape pshape = *it;
        igesWriter.AddShape (pshape->Shape());
    }

    igesWriter.ComputeModel();
#endif

    // write face entity names
    for (it = shapes.begin(); it != shapes.end(); ++it) {
        PNamedShape pshape = *it;
        WriteIGESFaceNames(igesWriter.TransferProcess(), pshape);
    }

    if (igesWriter.Write(const_cast<char*>(filename.c_str())) != Standard_True) {
        throw CTiglError("Error: Export of shapes to IGES file failed in CCPACSImportExport::SaveIGES", TIGL_ERROR);
    }
}

void CTiglExportIges::SetOCAFStoreType(ShapeStoreType type)
{
    myStoreType = type;
}

} // end namespace tigl
