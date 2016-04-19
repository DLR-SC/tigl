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

#include "CTiglExportStl.h"
#include "CCPACSConfiguration.h"

#include "TopoDS_Shape.hxx"
#include "Standard_CString.hxx"
#include "ShapeFix_Shape.hxx"
#include "BRep_Builder.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "StlAPI_Writer.hxx"
#include "Interface_Static.hxx"
#include "StlAPI.hxx"
#include "CTiglFusePlane.h"

#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepLib.hxx>
#include <BRepTools.hxx>

#include "CTiglCommon.h"
#include "TopExp.hxx"
#include "TopTools_IndexedMapOfShape.hxx"

#include "CCPACSFuselageSegment.h"
#include "CCPACSWingSegment.h"

#include "TopoDS_Shape.hxx"
#include "CTiglExportStep.h"

#include <cassert>

namespace tigl 
{

// Constructor
CTiglExportStl::CTiglExportStl(CCPACSConfiguration& config)
:myConfig(config)
{
}


// Exports a selected wing, boolean fused and meshed, as STL file
void CTiglExportStl::ExportMeshedWingSTL(int wingIndex, const std::string& filename, double deflection)
{
    CCPACSWing& wing = myConfig.GetWing(wingIndex);
    PNamedShape loft = wing.GetLoft();

    BRepMesh_IncrementalMesh(loft->Shape(), deflection);
    StlAPI_Writer *StlWriter = new StlAPI_Writer();
    StlWriter->Write(loft->Shape(), const_cast<char*>(filename.c_str()));
}


// Exports a selected fuselage, boolean fused and meshed, as STL file
void CTiglExportStl::ExportMeshedFuselageSTL(int fuselageIndex, const std::string& filename, double deflection)
{
    CCPACSFuselage& fuselage = myConfig.GetFuselage(fuselageIndex);
    PNamedShape loft = fuselage.GetLoft();

    BRepMesh_IncrementalMesh(loft->Shape(), deflection);
    StlAPI_Writer *StlWriter = new StlAPI_Writer();
    StlWriter->Write(loft->Shape(), const_cast<char*>(filename.c_str()));
}

// Exports a whole geometry, boolean fused and meshed, as STL file
void CTiglExportStl::ExportMeshedGeometrySTL(const std::string& filename, double deflection)
{
    PTiglFusePlane fuser = myConfig.AircraftFusingAlgo();
    assert(fuser);
    fuser->SetResultMode(FULL_PLANE);

    // get/compute shape
    PNamedShape ac = fuser->FusedPlane();
    if (!ac) {
        throw CTiglError("Error computing fused geometry in CTiglExportStl::ExportMeshedGeometrySTL", TIGL_ERROR);
    }

    TopoDS_Shape loft = ac->Shape();

    BRepMesh_IncrementalMesh(loft, deflection);
    StlAPI_Writer *StlWriter = new StlAPI_Writer();
    StlWriter->Write(loft, const_cast<char*>(filename.c_str()));
}

void CTiglExportStl::ExportWettedSurfaceSTL(const std::string& filename, double deflection)
{

    bool nHalfModel = false;
    bool closedTE = true;

    // Vectors for the Shapes ant the solids
    std::vector<AeroShape> vFuselages;
    std::vector<AeroShape> vWings;

    vFuselages.resize(myConfig.GetFuselageCount());
    vWings.resize(myConfig.GetWingCount());

    // variable declaration
    SegmentType segmentType = INNER_SEGMENT;
    BRep_Builder builder;
    TopTools_IndexedMapOfShape faceMap;

    // true if a fuselage is defined

    bool bFuselage = false;

    AeroShape* nAS;

    //Export all fuselages of the configuration
    for (int f = 1; f <= myConfig.GetFuselageCount(); f++)
    {

        bFuselage = true;

        TopoDS_Compound compFuselage;
        TopoDS_Compound compSolidFuselage;

        CCPACSFuselage& fuselage = myConfig.GetFuselage(f);

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

            TopoDS_Shape splittedLoft = segment.GetSplittedLoft(segmentType);

            if (symmetryAxis != TIGL_NO_SYMMETRY && !nHalfModel) 
            {
                loft = tigl::CTiglCommon::mirrorShape(loft, symmetryAxis);
                splittedLoft = tigl::CTiglCommon::mirrorShape(splittedLoft, symmetryAxis);
            }

            //Transform loft by fuselage transformation => absolute world coordinates
            loft = fuselage.GetFuselageTransformation().Transform(loft);

            builder.Add(compFuselage, loft);
            builder.Add(compSolidFuselage, splittedLoft);
        }

        // create a Solid out of the fuselage segments
        BRepBuilderAPI_Sewing sewMachine;
        sewMachine.Add(compSolidFuselage);


        if (sewMachine.NbFreeEdges() > 0)
        {
            throw CTiglError("Error during Fuselage solid generation.\nFree edges dedected during sewing.");
        }

        sewMachine.Perform();
        BRepBuilderAPI_MakeSolid makeSolid(TopoDS::Shell(sewMachine.SewedShape()));

        TopoDS_Shape solid = makeSolid.Solid();

        if (!BRepLib::OrientClosedSolid(TopoDS::Solid(solid)))
            throw CTiglError("Error during Wing solid generation.\nSolid is open or unorientable.");

        //         Add fuselage as shape and solid
        nAS = new AeroShape;
        nAS->Solid = solid;
        nAS->Name = fuselage.GetName();
        nAS->Shape = compFuselage;
        vFuselages[f-1] = *nAS;


    }

    // Export all wings of the configuration
    for (int w = 1; w <= myConfig.GetWingCount(); w++)
    {

        TopoDS_Compound compWing;
        TopoDS_Compound compSolidWing;
        TopoDS_Compound compFuselage;
        builder.MakeCompound(compWing);
        builder.MakeCompound(compSolidWing);

        CCPACSWing& wing = myConfig.GetWing(w);
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
            loft = segment.GetAeroLoft(segmentType, nHalfModel, false, closedTE);
            // [[CAS_AES]] added symmetry handling
            if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel) {
                loft = CTiglCommon::mirrorShape(loft, symmetryAxis);
            }

            builder.Add(compWing, loft);

            if ((symmetryAxis != TIGL_NO_SYMMETRY) && !nHalfModel) {
                builder.Add(compSolidWing, loft);
            }
            else {
                segment.Invalidate();
                builder.Add(compSolidWing, segment.GetAeroLoft(segmentType, false, true, closedTE));
            }


        }

        // create a solid out of the wing
        BRepBuilderAPI_Sewing sewMachine;
        sewMachine.Add(compSolidWing);

        if (sewMachine.NbFreeEdges() > 0)
        {
            throw CTiglError("Error during Wing solid generation.\nFree edges dedected during sewing.");
        }

        sewMachine.Perform();
        BRepBuilderAPI_MakeSolid makeSolid(TopoDS::Shell(sewMachine.SewedShape()));
        TopoDS_Shape solid = makeSolid.Solid();

        if (!BRepLib::OrientClosedSolid(TopoDS::Solid(solid)))
            throw CTiglError("Error during Wing solid generation.\nSolid is open or unorientable.");

        // cut wing with fuselage
        // remove the wing surfaces in the fuselage

        double u_min = 0.0, u_max = 0.0, v_min = 0.0, v_max = 0.0;

        if (bFuselage)
        {

            for (int i = 0; i < vFuselages.size(); i++)
            {

                TopoDS_Shape loft = CTiglCommon::splitShape(compWing, vFuselages[i].Shape);
    
                faceMap.Clear();
                TopExp::MapShapes(loft, TopAbs_FACE, faceMap);
    
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
                        throw CTiglError("Error during Wing cutting with the fuselage.");
                    }
                }
            }
        }

        nAS = new AeroShape;
        nAS->Solid = solid;
        nAS->Name = wing.GetName();
        nAS->Shape = compWing;

        vWings[w-1] = *nAS;

        // cut fuselage with wing
        // remove surfaces of the fuselage, in the wing


        if (bFuselage)
        {
            for (int i = 0; i < vFuselages.size(); i++)
            {

                TopoDS_Shape fuseLoft = tigl::CTiglCommon::splitShape(vFuselages[i].Shape, compWing);
    
                faceMap.Clear();
                TopExp::MapShapes(fuseLoft, TopAbs_FACE, faceMap);
    
                builder.MakeCompound(compFuselage);

                BRepClass3d_SolidClassifier solidFuseClass50(solid);
    
                for (int f = 1; f <= faceMap.Extent(); f++) 
                {
                    const TopoDS_Face loftFace = TopoDS::Face(faceMap(f));
                    gp_Pnt pnt50;
                    Handle(Geom_Surface) surf = BRep_Tool::Surface(loftFace);
                    BRepTools::UVBounds(loftFace, u_min, u_max, v_min, v_max);
    
                    pnt50 = surf->Value(u_min + ((u_max - u_min) / 2), v_min + ((v_max - v_min) / 2));
    
                    solidFuseClass50.Perform(pnt50, 10e-6);
    
                    if (solidFuseClass50.State() == TopAbs_OUT)
                    {
                        builder.Add(compFuselage, loftFace);
                    }

    
                    if (solidFuseClass50.State() == TopAbs_UNKNOWN)
                    {
                        throw tigl::CTiglError("Error during Wing cutting with the fuselage.");
                    }

                }
    
    
                vFuselages[i].Shape = compFuselage;

            }
        }

    }


    // cut everey Wing with everey other wing
    // delete the surfaces of the first wing, which are in the second wing

    for (int a = 0; a < vWings.size(); a++)
        for (int b = 0; b < vWings.size(); b++)
            if (a == b)
                continue;
            else
            {
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
                        throw CTiglError("Error during Wing - Wing cut.");
                    }
                }

                vWings[a].Shape = compWing;
            }

            // declare variables for step export

    TopoDS_Compound wettedSurface;

    builder.MakeCompound(wettedSurface);

    for (int i = 0; i < vFuselages.size(); i++)
    {
        builder.Add(wettedSurface, vFuselages[i].Shape);
    }

    // Write Wings with names to Step File

    for (int j = 0; j < vWings.size(); j++)
    {
        builder.Add(wettedSurface, vWings[j].Shape);
    }

    TopoDS_Shape ACloft = wettedSurface;


    StlAPI_Writer *StlWriter = new StlAPI_Writer();
    StlWriter->Write(ACloft, const_cast<char*>(filename.c_str()));
}

} // end namespace tigl
