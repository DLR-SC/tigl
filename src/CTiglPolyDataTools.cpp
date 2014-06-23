/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-12 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglPolyDataTools.h"
#include "CTiglLogging.h"

// OCCT includes
#include <BRep_Builder.hxx>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <Precision.hxx>

#include <Poly_Triangulation.hxx>
#include <TShort_HArray1OfShortReal.hxx>

namespace 
{
    void AddTriangle(TopoDS_Compound& compound, gp_Pnt p1, gp_Pnt p2, gp_Pnt p3) 
    {
        double eps = Precision::Confusion();
        BRep_Builder builder;
        if (p1.Distance(p2) > eps &&
            p2.Distance(p3) > eps &&
            p3.Distance(p1) > eps) {

            BRepBuilderAPI_MakePolygon poly;
            poly.Add(p1);
            poly.Add(p2);
            poly.Add(p3);
            poly.Add(p1);
    
            try {
                BRepBuilderAPI_MakeFace facemaker(poly.Wire(), Standard_False);
                if (facemaker.IsDone()) {
                    TopoDS_Face triangleFace = facemaker.Face();
                    builder.Add(compound, triangleFace);
                }
            }
            catch(...) {
                // ignore error
            }
        }
    }
}

namespace tigl 
{

CTiglPolyDataTools::CTiglPolyDataTools()
{
}

TopoDS_Shape CTiglPolyDataTools::MakeTopoDS(CTiglPolyData& mesh) 
{
    BRep_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    tigl::CTiglPolyObject& co = mesh.currentObject();
    for (unsigned long ipoly = 0; ipoly < co.getNPolygons(); ++ipoly) {
        if (co.getNPointsOfPolygon(ipoly) == 3) {
            long index1 = co.getVertexIndexOfPolygon(0, ipoly);
            long index2 = co.getVertexIndexOfPolygon(1, ipoly);
            long index3 = co.getVertexIndexOfPolygon(2, ipoly);

            AddTriangle(compound,
                        co.getVertexPoint(index1).Get_gp_Pnt(),
                        co.getVertexPoint(index2).Get_gp_Pnt(),
                        co.getVertexPoint(index3).Get_gp_Pnt());
        }
        else if (co.getNPointsOfPolygon(ipoly) == 4) {
            long index1 = co.getVertexIndexOfPolygon(0, ipoly);
            long index2 = co.getVertexIndexOfPolygon(1, ipoly);
            long index3 = co.getVertexIndexOfPolygon(2, ipoly);
            long index4 = co.getVertexIndexOfPolygon(3, ipoly);

            AddTriangle(compound,
                        co.getVertexPoint(index1).Get_gp_Pnt(),
                        co.getVertexPoint(index2).Get_gp_Pnt(),
                        co.getVertexPoint(index3).Get_gp_Pnt());

            AddTriangle(compound,
                        co.getVertexPoint(index3).Get_gp_Pnt(),
                        co.getVertexPoint(index4).Get_gp_Pnt(),
                        co.getVertexPoint(index1).Get_gp_Pnt());
        }
        else {
            LOG(WARNING) << "Illegal number of vertices in polygon, nvertex = " << co.getNPointsOfPolygon(ipoly);
        }
    }

    return compound;
}

Handle(Poly_Triangulation) CTiglPolyDataTools::MakePoly_Triangulation(CTiglPolyData &mesh)
{
    tigl::CTiglPolyObject& co = mesh.currentObject();

    // count number of vertices
    unsigned long nverts = 0;
    unsigned long ntria  = 0;
    for (unsigned int ipoly = 0; ipoly < co.getNPolygons(); ++ipoly) {
        unsigned long npoints = co.getNPointsOfPolygon(ipoly);
        if (npoints == 3) {
            nverts += 3;
            ntria+=1;
        }
        if (npoints == 4) {
            // make 2 triangles
            nverts += 6;
            ntria+=2;
        }
    }


    TColgp_Array1OfPnt Nodes(1,nverts);
    Poly_Array1OfTriangle Triangles(1,ntria);
    Handle(TShort_HArray1OfShortReal) normals = new TShort_HArray1OfShortReal(1,3*nverts);
    Standard_Integer iNode = 1;
    Standard_Integer iTria = 1;
    Standard_Integer iNormal = 1;
    for (unsigned int ipoly = 0; ipoly < co.getNPolygons(); ++ipoly) {
        unsigned long npoints = co.getNPointsOfPolygon(ipoly);
        if (npoints == 3) {
            unsigned long index1 = co.getVertexIndexOfPolygon(0, ipoly);
            unsigned long index2 = co.getVertexIndexOfPolygon(1, ipoly);
            unsigned long index3 = co.getVertexIndexOfPolygon(2, ipoly);
            unsigned long iNode_old = iNode;

            CTiglPoint v1, v2, v3;
            v1 = co.getVertexPoint(index1);
            v2 = co.getVertexPoint(index2);
            v3 = co.getVertexPoint(index3);

            // add vertices
            Nodes.SetValue(iNode++, v1.Get_gp_Pnt());
            Nodes.SetValue(iNode++, v2.Get_gp_Pnt());
            Nodes.SetValue(iNode++, v3.Get_gp_Pnt());

            // add triangle
            Triangles.SetValue(iTria++, Poly_Triangle(iNode_old, iNode_old+1, iNode_old+2));

            CTiglPoint normal = CTiglPoint::cross_prod(v2-v1, v3-v1);
            normal = normal*(1./normal.norm2());

            // normal at v1
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.z);
            // normal at v2
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.z);
            // normal at v3
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal.z);

        }
        if (npoints == 4){
            unsigned long index1 = co.getVertexIndexOfPolygon(0, ipoly);
            unsigned long index2 = co.getVertexIndexOfPolygon(1, ipoly);
            unsigned long index3 = co.getVertexIndexOfPolygon(2, ipoly);
            unsigned long index4 = co.getVertexIndexOfPolygon(3, ipoly);
            unsigned long iNode_old = iNode;

            CTiglPoint v1, v2, v3, v4;
            v1 = co.getVertexPoint(index1);
            v2 = co.getVertexPoint(index2);
            v3 = co.getVertexPoint(index3);
            v4 = co.getVertexPoint(index4);

            // add triangle 1
            // add vertices
            Nodes.SetValue(iNode++, v1.Get_gp_Pnt());
            Nodes.SetValue(iNode++, v2.Get_gp_Pnt());
            Nodes.SetValue(iNode++, v3.Get_gp_Pnt());

            // add triangle
            Triangles.SetValue(iTria++, Poly_Triangle(iNode_old, iNode_old+1, iNode_old+2));

            CTiglPoint normal1 = CTiglPoint::cross_prod(v2-v1, v3-v1);
            normal1 = normal1*(1./normal1.norm2());

            // normal at v1
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.z);
            // normal at v2
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.z);
            // normal at v3
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal1.z);



            // add triangle 2
            iNode_old = iNode;
            // add vertices
            Nodes.SetValue(iNode++, v3.Get_gp_Pnt());
            Nodes.SetValue(iNode++, v4.Get_gp_Pnt());
            Nodes.SetValue(iNode++, v1.Get_gp_Pnt());

            // add triangle
            Triangles.SetValue(iTria++, Poly_Triangle(iNode_old, iNode_old+1, iNode_old+2));

            CTiglPoint normal2 = CTiglPoint::cross_prod(v4-v3, v1-v3);
            normal2 = normal2*(1./normal2.norm2());

            // normal at v1
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.z);
            // normal at v2
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.z);
            // normal at v3
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.x);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.y);
            normals->SetValue(iNormal++, (Standard_ShortReal) normal2.z);
        }
    }

    Handle(Poly_Triangulation) triangulation = new Poly_Triangulation(Nodes, Triangles);
    triangulation->SetNormals(normals);

    return triangulation;
}

} // namespace tigl
