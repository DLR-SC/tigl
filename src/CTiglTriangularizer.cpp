#include "CTiglTriangularizer.h"

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>

#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp_Face.hxx>

#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

#include <TColgp_Array1OfPnt2d.hxx>

namespace tigl{

CTiglTriangularizer::CTiglTriangularizer()
{
    useMultipleObjects(false);
}

CTiglTriangularizer::CTiglTriangularizer(TopoDS_Shape& shape, bool multipleObj) {
    useMultipleObjects(multipleObj);
    
    triangularizeShape(shape);
}

int CTiglTriangularizer::triangularizeShape(const TopoDS_Shape& shape){
    TopExp_Explorer shellExplorer;
    TopExp_Explorer faceExplorer;
    
    for (shellExplorer.Init(shape, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next()) {
        const TopoDS_Shell shell = TopoDS::Shell(shellExplorer.Current());
        
        for (faceExplorer.Init(shell, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
            TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
            TopLoc_Location location;
            const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
            if (triangulation.IsNull())
                continue;
            
            if(triangulation->HasUVNodes()){
                currentObject().enableNormals(true);
            }
            
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
                
                CTiglPolygon polygon;
                
                // determine unique point indices
                if ( face.Orientation() ==  TopAbs_FORWARD){
                    polygon.addPoint(tpoint1.XYZ());
                    polygon.addPoint(tpoint2.XYZ());
                    polygon.addPoint(tpoint3.XYZ());
                }
                else {
                    polygon.addPoint(tpoint1.XYZ());
                    polygon.addPoint(tpoint3.XYZ());
                    polygon.addPoint(tpoint2.XYZ());
                }
                
                // calculate face normals
                if(triangulation->HasUVNodes()){
                    const TColgp_Array1OfPnt2d& uvnodes = triangulation->UVNodes();
                    BRepGProp_Face prop(face);
                    
                    gp_Pnt2d uv1 = uvnodes(index1);
                    gp_Pnt2d uv2 = uvnodes(index2);
                    gp_Pnt2d uv3 = uvnodes(index3);
                    
                    gp_Pnt pnt;
                    gp_Vec n1, n2, n3;
                    prop.Normal(uv1.X(),uv1.Y(),pnt,n1);
                    prop.Normal(uv2.X(),uv2.Y(),pnt,n2);
                    prop.Normal(uv3.X(),uv3.Y(),pnt,n3);
                    
                    if(face.Orientation() == TopAbs_FORWARD){
                        polygon.addNormal(n1.XYZ());
                        polygon.addNormal(n2.XYZ());
                        polygon.addNormal(n3.XYZ());
                    }
                    else{
                        polygon.addNormal(n1.XYZ());
                        polygon.addNormal(n3.XYZ());
                        polygon.addNormal(n2.XYZ());
                    }
                }
                
                polygon.setMetadata("unknown 0 0.0 0.0 0");
                currentObject().addPolygon(polygon);
                
            } // for triangles
            //polyData.createNewSurface();
        } // for faces
        if(_useMultipleObjects) createNewObject();
    } // for shells
    
    return 0;
}

void CTiglTriangularizer::useMultipleObjects(bool use){
    _useMultipleObjects = use;
}

} // namespace tigl
