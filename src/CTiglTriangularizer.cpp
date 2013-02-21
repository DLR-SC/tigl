/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-18 Martin Siggel <Martin.Siggel@dlr.de>
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
            
            gp_Trsf nodeTransformation = location;
            
            unsigned int * indexBuffer = NULL;
            unsigned int ilower = 0;
            
            if(triangulation->HasUVNodes()){
                // we use the uv nodes to compute normal vectors for each point
                
                BRepGProp_Face prop(face);
                currentObject().enableNormals(true);
                
                const TColgp_Array1OfPnt2d& uvnodes = triangulation->UVNodes(); // get (face-local) list of nodes
                ilower = uvnodes.Lower();
                
                indexBuffer = new unsigned int [uvnodes.Upper()-uvnodes.Lower()+1];
                unsigned int * pIndexBuf = indexBuffer;
                for(int inode = uvnodes.Lower(); inode <= uvnodes.Upper(); ++inode){
                    const gp_Pnt2d& uv_pnt = uvnodes(inode);
                    gp_Pnt p; gp_Vec n;
                    prop.Normal(uv_pnt.X(),uv_pnt.Y(),p,n);
                    
                    *pIndexBuf++ = currentObject().addPointNormal(p.XYZ(), n.XYZ());
                }
            } 
            else {
                // we cannot compute normals
                
                const TColgp_Array1OfPnt& nodes = triangulation->Nodes(); // get (face-local) list of nodes
                ilower = nodes.Lower();
                
                indexBuffer = new unsigned int [nodes.Upper()-nodes.Lower()+1];
                unsigned int * pIndexBuf = indexBuffer;
                for(int inode = nodes.Lower(); inode <= nodes.Upper(); inode++){
                    const gp_Pnt& p = nodes(inode).Transformed(nodeTransformation);
                    *pIndexBuf++ = currentObject().addPointNormal(p.XYZ(), CTiglPoint(1,0,0));
                }
            }

            
            const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
            for (int j = triangles.Lower(); j <= triangles.Upper(); j++) // iterate over triangles in the array
            {
                const Poly_Triangle& triangle = triangles(j);
                int occindex1, occindex2, occindex3;
                triangle.Get(occindex1, occindex2, occindex3); // get indices into index1..3
                unsigned int index1, index2, index3;
                index1 = indexBuffer[occindex1-ilower];
                index2 = indexBuffer[occindex2-ilower];
                index3 = indexBuffer[occindex3-ilower];
                
                if(face.Orientation() == TopAbs_FORWARD)
                    currentObject().addTriangleByVertexIndex(index1, index2, index3);
                else
                    currentObject().addTriangleByVertexIndex(index1, index3, index2);
                
            } // for triangles
            delete[] indexBuffer;
        } // for faces
        if(_useMultipleObjects) createNewObject();
    } // for shells
    
    return 0;
}

void CTiglTriangularizer::useMultipleObjects(bool use){
    _useMultipleObjects = use;
}

} // namespace tigl
