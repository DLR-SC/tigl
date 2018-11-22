/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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

#include "CCPACSConfiguration.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglError.h"
#include "CNamedShape.h"
#include "tiglcommonfunctions.h"
#include "CTiglFusePlane.h"
#include "CTiglExporterFactory.h"
#include "CTiglTypeRegistry.h"

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
#include <IGESBasic_Group.hxx>
#include <IGESGeom_TrimmedSurface.hxx>
#include <IGESGeom_CompositeCurve.hxx>
#include <IGESBasic_Name.hxx>
#include <IGESGeom_CurveOnSurface.hxx>
#include <IGESSelect_WorkLibrary.hxx>

#include <map>
#include <cassert>

namespace
{

    template <class T>
    bool toBool(T v)
    {
        return !!v;
    }

    tigl::FaceNameSettings GetFaceNameMode(const tigl::ExporterOptions& options)
    {
        return options.HasOption("FaceNames")
                ? options.Get<tigl::FaceNameSettings>("FaceNames")
                : tigl::IgesOptions().Get<tigl::FaceNameSettings>("FaceNames");
    }

    void AssignLevelToAllEntities(Handle(IGESData_IGESEntity) ent, int level)
    {
        // assign level in case of trimmed surfaces type
        Handle(IGESData_LevelListEntity) lle;
        ent->InitLevel(lle, level);
        if (ent->IsInstance(STANDARD_TYPE(IGESBasic_Group))) {
            // recurse down into single groups until trimmed surface is found
            Handle(IGESBasic_Group) group = Handle(IGESBasic_Group)::DownCast(ent);
            int numEntities = group->NbEntities();
            for (int i=1; i <= numEntities; i++) {
                AssignLevelToAllEntities(group->Entity(i), level);
            }
        }
        else if (ent->IsInstance(STANDARD_TYPE(IGESGeom_TrimmedSurface))) {
            Handle(IGESGeom_TrimmedSurface) trimmedSurf = Handle(IGESGeom_TrimmedSurface)::DownCast(ent);
            AssignLevelToAllEntities(trimmedSurf->Surface(), level);
            if (trimmedSurf->HasOuterContour()) {
                AssignLevelToAllEntities(trimmedSurf->OuterContour(), level);
            }
            for (int i=1; i <= trimmedSurf->NbInnerContours(); i++) {
                AssignLevelToAllEntities(trimmedSurf->InnerContour(i), level);
            }
        }
        else if (ent->IsInstance(STANDARD_TYPE(IGESGeom_CurveOnSurface))) {
            Handle(IGESGeom_CurveOnSurface) curveOnSurf = Handle(IGESGeom_CurveOnSurface)::DownCast(ent);
            if (!curveOnSurf->CurveUV().IsNull()) {
                AssignLevelToAllEntities(curveOnSurf->CurveUV(), level);
            }
            if (!curveOnSurf->Curve3D().IsNull()) {
                AssignLevelToAllEntities(curveOnSurf->Curve3D(), level);
            }
        }
        else if (ent->IsInstance(STANDARD_TYPE(IGESGeom_CompositeCurve))) {
            Handle(IGESGeom_CompositeCurve) compositeCurve = Handle(IGESGeom_CompositeCurve)::DownCast(ent);
            for (int i=1; i <= compositeCurve->NbCurves(); i++) {
                AssignLevelToAllEntities(compositeCurve->Curve(i), level);
            }
        }
    }

    /**
     * @brief Allows setting IGES names of more than 8 characters by appending an IGES 406 entity
     */
    void SetLongEntityName(IGESControl_Writer& writer, Handle(IGESData_IGESEntity) entity, const std::string& name)
    {
        if (name.empty()) {
            return;
        }

        Handle(IGESBasic_Name) nameEntity = new IGESBasic_Name;
        nameEntity->Init(1, new TCollection_HAsciiString(name.c_str()));
        entity->AddProperty(nameEntity);
        writer.AddEntity(nameEntity);
    }

    std::string ExportedLongFaceName(const std::string& shapeName, const std::string& faceName, tigl::FaceNameSettings settings)
    {
        switch (settings) {
        case tigl::UIDOnly:
            return shapeName;
        case tigl::FaceNameOnly:
            return faceName;
        case tigl::UIDandFaceName:
            return shapeName + "::" + faceName;
        case tigl::None:
            return "";
        }
    }

    /**
     * @brief WriteIGESFaceNames takes the names of each face and writes it into the IGES model.
     */
    void WriteIGESFaceNames(IGESControl_Writer& writer, const PNamedShape shape, const tigl::ExporterOptions& globalOptions, int level)
    {
        if (!shape) {
            return;
        }

        Handle(Transfer_FinderProcess) FP = writer.TransferProcess();

        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape->Shape(),   TopAbs_FACE, faceMap);
        for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
            TopoDS_Face face = TopoDS::Face(faceMap(iface));
            std::string faceName = shape->GetFaceTraits(iface-1).Name();
            std::string shortFaceName = faceName;
            
            if (shortFaceName == shape->Name()) {
                shortFaceName = shape->ShortName();
            }
            
            PNamedShape origin = shape->GetFaceTraits(iface-1).Origin();
            if (origin && origin->Name() == shortFaceName) {
                shortFaceName = origin->ShortName();
            }
            
            // IGES allows entity names of at max 8 characters.
            // If the string is longer than 8 characters, the IGES exports might crash
            if (shortFaceName.length() > 8) {
                shortFaceName = shortFaceName.substr(0,8);
            }

            // set face name
            Handle(IGESData_IGESEntity) entity;
            Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, face );
            if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(IGESData_IGESEntity), entity ) ) {
                Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(shortFaceName.c_str());
                entity->SetLabel(str);
                SetLongEntityName(writer, entity, ExportedLongFaceName(shape->Name(), faceName, GetFaceNameMode(globalOptions)));
                AssignLevelToAllEntities(entity, level);
            }
        }
    }
    
    void WriteIGESShapeNames(IGESControl_Writer& writer, const PNamedShape shape, int level)
    {
        if (!shape) {
            return;
        }

        Handle(Transfer_FinderProcess) FP = writer.TransferProcess();

        std::string shortName = shape->ShortName();
        std::string shapeName = shape->Name();
        // IGES allows entity names of at max 8 characters.
        // If the string is longer than 8 characters, the IGES exports might crash
        if (shortName.length() > 8) {
            shortName = shortName.substr(0,8);
        }

        // insert blanks
        int nblanks = 8 - static_cast<int>(shortName.length());
        for (int i = 0; i < nblanks; ++i) {
            shortName.insert(shortName.begin(), ' ');
        }

        // set shape name
        Handle(IGESData_IGESEntity) entity;
        Handle(TransferBRep_ShapeMapper) mapper = TransferBRep::ShapeMapper ( FP, shape->Shape() );
        if ( FP->FindTypedTransient ( mapper, STANDARD_TYPE(IGESData_IGESEntity), entity ) ) {
            Handle(TCollection_HAsciiString) str = new TCollection_HAsciiString(shortName.c_str());
            entity->SetLabel(str);
            SetLongEntityName(writer, entity, shapeName);
            AssignLevelToAllEntities(entity, level);
        }
    }
    
    void WriteIgesWireName(IGESControl_Writer& writer, const PNamedShape shape)
    {
        if (!shape) {
            return;
        }

        Handle(Transfer_FinderProcess) FP = writer.TransferProcess();
        
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
    
    void WriteIgesNames(IGESControl_Writer& writer, const PNamedShape shape, const tigl::ExporterOptions& globalOptions, int level)
    {
        WriteIGESFaceNames(writer, shape, globalOptions, level);
        WriteIGESShapeNames(writer, shape, level);
    }

    int GetBRepMode(const tigl::CTiglExportIges& writer)
    {
        return writer.GlobalExportOptions().HasOption("IGES5.3")
                ? writer.GlobalExportOptions().Get<bool>("IGES5.3")
                : tigl::IgesOptions().Get<bool>("IGES5.3");
    }
    
} //namespace

namespace tigl
{

AUTORUN(CTiglExportIges)
{
    static CCADExporterBuilder<CTiglExportIges> igesExporterBuilder;
    CTiglExporterFactory::Instance().RegisterExporter(&igesExporterBuilder, IgesOptions());
    return true;
}

// Constructor
CTiglExportIges::CTiglExportIges(const ExporterOptions& opt)
    : CTiglCADExporter(opt)
{
}

ExporterOptions CTiglExportIges::GetDefaultOptions() const
{
    return IgesOptions();
}

ShapeExportOptions CTiglExportIges::GetDefaultShapeOptions() const
{
    return IgesShapeOptions();
}

void CTiglExportIges::SetTranslationParameters() const
{
    Interface_Static::SetCVal("xstep.cascade.unit", "MM");
    Interface_Static::SetCVal("write.iges.unit", "MM");
    /*
     * BRep entities in IGES are experimental and untested.
     * They allow to specify things like shells and solids.
     * It seems, that CATIA does not support these entities.
     * However, in order to fix Issue #182, BRep entities are
     * necessary.
     */
    Interface_Static::SetIVal("write.iges.brep.mode", GetBRepMode(*this));
    Interface_Static::SetCVal("write.iges.header.author", "TiGL");
    Interface_Static::SetCVal("write.iges.header.company", "German Aerospace Center (DLR), SC");
}


// Save a sequence of shapes in IGES Format
bool CTiglExportIges::WriteImpl(const std::string& filename) const
{
    if (filename.empty()) {
       LOG(ERROR) << "Error: Empty filename in CTiglExportIges::Write.";
       return false;
    }

    IGESControl_Controller::Init();

    // scale all shapes to mm
    ListPNamedShape shapeScaled;
    ListPNamedShape::const_iterator it;
    for (size_t ishape = 0; ishape < NShapes(); ++ishape) {
        PNamedShape pshape = GetShape(ishape);
        if (pshape) {
            CTiglTransformation trafo;
            trafo.AddScaling(1000,1000,1000);
            PNamedShape pScaledShape(new CNamedShape(*pshape));
            pScaledShape->SetShape(trafo.Transform(pshape->Shape()));
            shapeScaled.push_back(pScaledShape);
        }
    }

    size_t iShape = 0;
    size_t iLevel = 0;
    ListPNamedShape list;
    std::vector<size_t> levels;
    for (it = shapeScaled.begin(); it != shapeScaled.end(); ++it) {
        ShapeGroupMode groupMode = GlobalExportOptions().GroupMode();
        ListPNamedShape templist = GroupFaces(*it, groupMode);

        if (GetOptions(iShape).HasOption("Layer")) {
            int level = GetOptions(iShape).Get<int>("Layer");
            if (level >= 0) {
                iLevel = static_cast<size_t>(level);
            }
        }

        for (ListPNamedShape::iterator it2 = templist.begin(); it2 != templist.end(); ++it2) {
            list.push_back(*it2);
            levels.push_back(iLevel);
        }
        iShape++;
        iLevel++;
    }

    IGESControl_Writer igesWriter("MM", GetBRepMode(*this));
    SetTranslationParameters();
    igesWriter.Model()->ApplyStatic();

    for (size_t i = 0; i < list.size(); ++i) {
        AddToIges(list[i], igesWriter, static_cast<int>(levels[i]));
    }

    igesWriter.ComputeModel();

    return toBool(igesWriter.Write(const_cast<char*>(filename.c_str())));
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
            throw CTiglError("Export to IGES file failed in CTiglExportStep. Could not translate shape " 
                             + shapeName + " to iges entity,", TIGL_ERROR);
        }
        WriteIgesNames(writer, shape, GlobalExportOptions(), level);
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
                throw CTiglError("Export to IGES file failed in CTiglExportIges. Could not translate shape " 
                                 + shapeName + " to iges entity,", TIGL_ERROR);
            }
            PNamedShape theWire(new CNamedShape(Edges->Value(iwire),shapeName.c_str()));
            theWire->SetShortName(shapeShortName.c_str());
            WriteIGESShapeNames(writer, theWire, level);
            WriteIgesWireName(writer, theWire);
        }
    }
}

} // end namespace tigl
