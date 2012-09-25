/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
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

// standard libraries
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <string>

#include "CTiglExportVtk.h"
#include "CCPACSConfiguration.h"
#include "ITiglGeometricComponent.h"
#include "ITiglSegment.h"
#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSWingSegment.h"

// open cascade basics
#include "gp_Pnt.hxx"
#include "gp_Lin.hxx"
#include "gp_Vec.hxx"
#include "Precision.hxx"
#include "Geom_Line.hxx"
#include "Geom_Curve.hxx"
#include "Geom_Surface.hxx"
#include "Geom_BSplineSurface.hxx"
#include "GProp_PGProps.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Wire.hxx"
#include "TColgp_Array1OfPnt.hxx"

// creators
#include "ShapeFix_Shape.hxx"
#include "BRep_Builder.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeShape.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"

// algorithms
#include "BRepMesh.hxx"
#include "BRep_Tool.hxx"
#include "BRepLib_FindSurface.hxx"
#include "BRepExtrema_DistShapeShape.hxx"
#include "Poly_Triangulation.hxx"
#include "TopOpeBRepDS_SurfaceExplorer.hxx"
#include "TopExp_Explorer.hxx"
#include "GeomAPI_IntCS.hxx"
#include "GeomAPI_IntSS.hxx"
#include "ShapeAnalysis_Surface.hxx"
#include "TopOpeBRepDS_DataStructure.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"


namespace tigl {

    // Constructor
    CTiglExportVtk::CTiglExportVtk(CCPACSConfiguration& config)
        :myConfig(config)
    {
    }

    // Destructor
    CTiglExportVtk::~CTiglExportVtk(void)
    {
    }
	
    
    // Exports a by index selected wing, boolean fused and meshed, as STL file
    void CTiglExportVtk::ExportMeshedWingVTKByIndex(const int wingIndex, const std::string& filename, const double deflection)
    {
        //exportMode = TIGL_VTK_COMPLEX;
        ITiglGeometricComponent & component = myConfig.GetWing(wingIndex);
        TopoDS_Shape loft = component.GetLoft();

        VtkWriter *vtkWriter = new VtkWriter(myConfig);
        vtkWriter->Write(loft, component, const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_COMPLEX);
    }

    // Exports a by UID selected wing, boolean fused and meshed, as STL file
    void CTiglExportVtk::ExportMeshedWingVTKByUID(const std::string wingUID, const std::string& filename, const double deflection)
    {
        ITiglGeometricComponent & component = myConfig.GetWing(wingUID);
        TopoDS_Shape loft = component.GetLoft();

        VtkWriter *vtkWriter = new VtkWriter(myConfig);
        vtkWriter->Write(loft, component, const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_COMPLEX);
    }



    // Exports a by index selected fuselage, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedFuselageVTKByIndex(const int fuselageIndex, const std::string& filename, const double deflection)
    {
        ITiglGeometricComponent & component = myConfig.GetFuselage(fuselageIndex);
        TopoDS_Shape loft = component.GetLoft();

        VtkWriter *vtkWriter = new VtkWriter(myConfig);
        vtkWriter->Write(loft, component, const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_COMPLEX);
    }

    // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedFuselageVTKByUID(const std::string fuselageUID, const std::string& filename, const double deflection)
    {
        ITiglGeometricComponent & component = myConfig.GetFuselage(fuselageUID);
        TopoDS_Shape loft = component.GetLoft();

        VtkWriter *vtkWriter = new VtkWriter(myConfig);
        vtkWriter->Write(loft, component, const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_COMPLEX);
    }
    

    // Exports a whole geometry, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedGeometryVTK(const std::string& filename, const double deflection)
    {
        //const TopoDS_Shape loft = myConfig.GetFusedAirplane();
        //VtkWriter *vtkWriter = new VtkWriter(myConfig);
		//// TODO: we need an idea for the complete geometry component.
        //vtkWriter->Write(loft, myConfig.GetFuselage(1), const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_COMPLEX);
        ExportMeshedWingVTKByIndex(1, filename, deflection);
    }

    /************* Simple ones *************************/
    // Exports a by UID selected wing, boolean fused and meshed, as STL file
    void CTiglExportVtk::ExportMeshedWingVTKSimpleByUID(const std::string wingUID, const std::string& filename, const double deflection)
    {
        ITiglGeometricComponent & component = myConfig.GetWing(wingUID);
        TopoDS_Shape loft;

        VtkWriter *vtkWriter = new VtkWriter(myConfig);
        vtkWriter->Write(loft, component, const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_SIMPLE);
    }

    // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedFuselageVTKSimpleByUID(const std::string fuselageUID, const std::string& filename, const double deflection)
    {
        ITiglGeometricComponent & component = myConfig.GetFuselage(fuselageUID);
        TopoDS_Shape loft;

        VtkWriter *vtkWriter = new VtkWriter(myConfig);
        vtkWriter->Write(loft, component, const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_SIMPLE);
    }


    // Exports a whole geometry, boolean fused and meshed, as VTK file
    void CTiglExportVtk::ExportMeshedGeometryVTKSimple(const std::string& filename, const double deflection)
    {
        //const TopoDS_Shape loft = myConfig.GetFusedAirplane();
        //VtkWriter *vtkWriter = new VtkWriter(myConfig);
        //// TODO: we need an idea for the complete geometry component.
        //vtkWriter->Write(loft, myConfig.GetFuselage(1), const_cast<char*>(filename.c_str()), deflection, TIGL_VTK_SIMPLE);
        //ExportMeshedWingVTKSimpleByUID(filename, filename, deflection);
		std::cerr << "tiglExportMeshedGeometryVTKSimple not yet implemented!!" << std::endl;
    }

    // constructor
    VtkWriter::VtkWriter(CCPACSConfiguration& config)
        :myConfig(config)
    {
    }
    
    // Write out the shape to a VTK file
    void VtkWriter::Write(const TopoDS_Shape & shape, ITiglGeometricComponent & component, const std::string& filename, const double deflection, VTK_EXPORT_MODE mode)
    {
        pointsMin = 0;
        pointsMax = 0;
        unsigned int points;
		if(mode == TIGL_VTK_SIMPLE) {
            points = SimpleTriangulation(shape, component, deflection);
		}
		else {
            points = FindUniquePoints(shape, component, deflection);
		}

        const unsigned int triangles = triangleList1.size();
        ofstream file;
        file.open(const_cast<char*>(filename.c_str()) , ios::out);
        file << "<?xml version=\"1.0\"?>" << endl << "<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\" compressor=\"vtkZLibDataCompressor\">" << endl << "  <PolyData>" << endl;
        file << "    <Piece NumberOfPoints=\"" << points << "\" NumberOfVerts=\"0\" NumberOfLines=\"0\" NumberOfStrips=\"0\" NumberOfPolys=\"" << triangles << "\">" << endl << "      <Points>" << endl;
        file << "        <DataArray type=\"Float64\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\" RangeMin=\"" << pointsMin << "\" RangeMax=\"" << pointsMax << "\">" << endl;
        for (unsigned int i = 0; i < points; i ++)
        {
            file << "          " << pointArray[i].X() << " " << pointArray[i].Y() << " " << pointArray[i].Z() << endl;
        }
        file << "        </DataArray>" << endl << "      </Points>" << endl << "      <Polys>" << endl << "        <DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\" RangeMin=\"0\" RangeMax=\"" << points - 1 << "\">" << endl;
        for (unsigned int i = 0; i < triangles; i ++)
        {
            file << "          " << triangleList1[i] << " "<< triangleList2[i] << " "<< triangleList3[i] << endl;
        }
        file << "        </DataArray>" << endl << "        <DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\" RangeMin=\"3\" RangeMax=\"" << (triangles * 3) << "\">" << endl << "         ";
        int next = 3;
        for (unsigned int i = 0; i < triangles; i ++)
        {
            file << " " << next;
            if ((i % 6 == 0) && (i != (triangles - 1)))
            {
                file << endl << "         ";
            }
            next += 3;
        }
        file << endl << "        </DataArray>" << endl;
		file << "        <MetaData elements=\"uID segmentIndex eta xsi isOnTop\">" << endl;
        for (unsigned int i = 0; i < triangles; i ++)
        {
            file << "          \"" << triangleUID[i] << "\" " << triangleSegment[i] << " " << triangleEta[i] << " " << triangleXsi[i] << " " << triangleOnTop[i] << endl;
        }
        file << "        </MetaData>" << endl;
		file << "      </Polys>" << endl << "    </Piece>" << endl << "  </PolyData>" << endl << "</VTKFile>" << endl;
        file.close();
        cout << "TIGL VTK export with " << points << " unique points and " << triangles << " triangles." << endl;
		//delete(pointArray);
    }



    // Helper function to detect unique points in all triangles
    unsigned int VtkWriter::FindUniquePoints(const TopoDS_Shape & shape, ITiglGeometricComponent & component, const double deflection) {
        TopExp_Explorer shellExplorer;
        TopExp_Explorer faceExplorer;
        TopOpeBRepDS_SurfaceExplorer surfaceExplorer;
        BRep_Builder builder;
        GProp_PGProps props;
        const int segmentCount = component.GetSegmentCount();

        // the surfaces of our shape
        const BRepLib_FindSurface findSurface(shape, /* tolerance */1);
        const Handle(Geom_Surface) & surface = findSurface.Surface();

        // 1. mesh
        BRepMesh::Mesh(shape, deflection);

        for (shellExplorer.Init(shape, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next()) {
            const TopoDS_Shell shell = TopoDS::Shell(shellExplorer.Current());

            for (faceExplorer.Init(shell, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
                TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
                TopLoc_Location location;
                const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
                if (triangulation.IsNull())
                    continue;

                gp_Trsf nodeTransformation = location;
                const TColgp_Array1OfPnt& nodes = triangulation->Nodes(); // get (face-local) list of nodes

                int index1, index2, index3;
                const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
                for (int j = triangles.Lower(); j <= triangles.Upper(); j++) // iterate over triangles in the array
                {
                    const Poly_Triangle& triangle = triangles(j);
                    triangle.Get(index1, index2, index3); // get indices into index1..3
                    const gp_Pnt tpoint1 = nodes(index1).Transformed(nodeTransformation);
                    const gp_Pnt tpoint2 = nodes(index2).Transformed(nodeTransformation);
                    const gp_Pnt tpoint3 = nodes(index3).Transformed(nodeTransformation);

                    // determine unique point indices
                    FindOrCreatePointIndex(triangleList1, tpoint1);
                    FindOrCreatePointIndex(triangleList2, tpoint2);
                    FindOrCreatePointIndex(triangleList3, tpoint3);

                    // determine surface point of triangle center, normal vector and surface point of the normal on the wing segment, then eta and xi
                    TColgp_Array1OfPnt triangle1Array(0, 2);
                    triangle1Array(0) = tpoint1;
                    triangle1Array(1) = tpoint2;
                    triangle1Array(2) = tpoint3;

                    // 2. a. center of gravity of triangle, b. normal vector from inside -> outside
                    const gp_Pnt baryCenter = props.Barycentre(triangle1Array); // get center of triangle
                    const gp_Vec normal = FindNormal(tpoint1, tpoint2, tpoint3); // find normal vector

                    // determine intersection of normal vector with fused geometry
                    const gp_Lin line(baryCenter, normal);
                    Geom_Line tempLine(line);
                    const Handle(Geom_Curve) & curve((const Handle(Geom_Curve) &) tempLine);

                    // 3. intersection with surface to find a "real" surface point.
                    GeomAPI_ProjectPointOnSurf project1(baryCenter, surface);
                    if (project1.IsDone()) {
                        int i = project1.NbPoints(); // else try next surface

                        // find nearest intersection point (point on the correct side of the surface)
                        Standard_Real minimumDist = 1e8; // use an arbitrary high number...
                        Standard_Boolean found = Standard_False;
                        int foundShapeIndex = 0;

                        const BRepBuilderAPI_MakeVertex foundVertex(project1.NearestPoint());
                        for (int s = 1; s <= segmentCount; s++) // find segment belonging to the intersection point on the explored surface (may always be just one, but we don't know for sure)
                        {
                            ITiglSegment & segment = component.GetSegment(s);
                            TopoDS_Shape segmentLoft = segment.GetLoft(); // get 3d shape of the segment

                            // 4. find minimum distance to determine the linked segment of the surface intersection
                            const BRepExtrema_DistShapeShape dist(segmentLoft, foundVertex);
                            if (dist.IsDone() && (dist.NbSolution() > 0)) {
                                for (int p = 0; p < dist.NbSolution(); p++) {
                                    const Standard_Real currentDist = dist.Value();
                                    if (currentDist < minimumDist) {
                                        minimumDist = currentDist;
                                        foundShapeIndex = s;
                                        found = Standard_True;
                                    }
                                }
                            }
                        }
                        if (found) // a minimum distance was found: we know the segment the intersecting point comes from! What we don't know is if we're on uppe or lower side, but currently we don't need this information for the projection
                        {
							// Gather metadata needed for VTK metadata-node
                            bool isUpperSide = true;
                            double eta = 0.0;
                            double xsi = 0.0;
							std::string uid = component.GetSegment(foundShapeIndex).GetUID();

					        // Important: works only for wings because on fuselage there is not lower side (?)!
							if(component.GetComponentType() == TIGL_COMPONENT_WING)
							{
							    CCPACSWingSegment& wingSegment = (CCPACSWingSegment&)component.GetSegment(foundShapeIndex);
								isUpperSide = wingSegment.GetIsOnTop(project1.NearestPoint());
								eta = wingSegment.GetEta(project1.NearestPoint(), isUpperSide);
                                xsi = wingSegment.GetXsi(project1.NearestPoint(), isUpperSide);
							}

							// set data
							triangleUID.push_back(uid);
                            triangleSegment.push_back(foundShapeIndex);
                            triangleOnTop.push_back(isUpperSide);
                            triangleEta.push_back(eta);
                            triangleXsi.push_back(xsi);
                        }
                    } // if surface intersect worked
                } // for triangles
            } // for faces
        } // for shells

        // copy map to enumerated array
        pointArray = new gp_Pnt[pointMap.size()];
        const unsigned int points = pointMap.size();
        for (PointMapType::iterator it = pointMap.begin(); it != pointMap.end(); ++it) {
			pointArray[(it->second)] = (it->first);// copy keys (points) to the array element of the value (index number)
        }
        pointMap.clear();
        return points;
    }


    unsigned int VtkWriter::SimpleTriangulation(const TopoDS_Shape & shape, ITiglGeometricComponent & component, const double deflection) {
            TopExp_Explorer shellExplorer;
            TopExp_Explorer faceExplorer;
            const int segmentCount = component.GetSegmentCount();

            for(int y=1; y <= segmentCount; y++ )
            { 
				TopoDS_Shape segmentLoft;

                if(component.GetComponentType() == TIGL_COMPONENT_WING) {
                    ITiglSegment & segment = component.GetSegment(y);
                    segmentLoft = segment.GetLoft(); // get 3d shape of the segment
					segmentLoft = component.GetTransformation().Transform(segmentLoft);
                }
                if(component.GetComponentType() == TIGL_COMPONENT_FUSELAGE) {
                    segmentLoft = component.GetLoft();
                }
                BRepMesh::Mesh(segmentLoft, deflection);

                for (shellExplorer.Init(segmentLoft, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next()) {
                    const TopoDS_Shell shell = TopoDS::Shell(shellExplorer.Current());

                    for (faceExplorer.Init(shell, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
                        TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
                        TopLoc_Location location;
                        const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
                        if (triangulation.IsNull())
                            continue;

                        gp_Trsf nodeTransformation = location;
                        const TColgp_Array1OfPnt& nodes = triangulation->Nodes(); // get (face-local) list of nodes

                        int index1, index2, index3;
                        const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
                        for (int j = triangles.Lower(); j <= triangles.Upper(); j++) // iterate over triangles in the array
                        {
                            const Poly_Triangle& triangle = triangles(j);
                            triangle.Get(index1, index2, index3); // get indices into index1..3
                            const gp_Pnt tpoint1 = nodes(index1).Transformed(nodeTransformation);
                            const gp_Pnt tpoint2 = nodes(index2).Transformed(nodeTransformation);
                            const gp_Pnt tpoint3 = nodes(index3).Transformed(nodeTransformation);

                            // determine unique point indices
                            FindOrCreatePointIndex(triangleList1, tpoint1);
                            FindOrCreatePointIndex(triangleList2, tpoint2);
                            FindOrCreatePointIndex(triangleList3, tpoint3);

                            // set data
                            triangleUID.push_back("");
                            triangleSegment.push_back(0);
                            triangleOnTop.push_back(0);
                            triangleEta.push_back(0.0);
                            triangleXsi.push_back(0.0);

                        } // for triangles
                    } // for faces
                } // for shells
                if(component.GetComponentType() == TIGL_COMPONENT_FUSELAGE) break;
            } // loop over segments

            // copy map to enumerated array
            pointArray = new gp_Pnt[pointMap.size()];
            const unsigned int points = pointMap.size();
            for (PointMapType::iterator it = pointMap.begin(); it != pointMap.end(); ++it) {
                pointArray[(it->second)] = (it->first);// copy keys (points) to the array element of the value (index number)
            }
            pointMap.clear();

            return points;
        }

    void VtkWriter::FindOrCreatePointIndex(std::vector<unsigned int> & list, gp_Pnt point)
    {
		int existsAt = -1;
		for (PointMapType::iterator it = pointMap.begin(); it != pointMap.end(); ++it) {
			gp_Pnt pnt = (it->first);

			if (pnt.IsEqual(point, 0.001)) {
				existsAt = (it->second);
				break;
			}
        }

		if (existsAt != -1) {
            list.push_back(existsAt);   // store existing index
        } else {
            unsigned int newIndex = pointMap.size();
            pointMap.insert(std::pair<gp_Pnt,int>(point,newIndex)); // store a new point and the new index
            list.push_back(newIndex);
        }
        SetMinValue(pointsMin, point);
        SetMaxValue(pointsMax, point);
    }



    void VtkWriter::SetMinValue(double& old, const gp_Pnt& point)
    {
        const double x = point.X();
        const double y = point.Y();
        const double z = point.Z();
        if (x < old) { old = x; }
        if (y < old) { old = y; }
        if (z < old) { old = z; }
    }

    void VtkWriter::SetMaxValue(double& old, const gp_Pnt& point)
    {
        const double x = point.X();
        const double y = point.Y();
        const double z = point.Z();
        if (x > old) { old = x; }
        if (y > old) { old = y; }
        if (z > old) { old = z; }
    }

    gp_Vec VtkWriter::FindNormal(const gp_Pnt P1, const gp_Pnt P2, const gp_Pnt P3) // taken from http://www.opencascade.org/org/forum/thread_1778/
    {
        gp_Vec V1(P1,P2); // V1=(P1,P2)
        gp_Vec V2(P2,P3); // V2=(P2,P3)
        gp_Vec V3(P3,P1); // V3=(P3,P1)
        if ((V1.SquareMagnitude() > Precision::Confusion())
            && (V2.SquareMagnitude() > Precision::Confusion())
            && (V3.SquareMagnitude() > Precision::Confusion()))
        {
            V1.Cross(V2); // V1 = Normal
        }
        return V1;
    }

} // end namespace tigl
