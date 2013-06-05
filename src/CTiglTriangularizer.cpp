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
#include "ITiglGeometricComponent.h"
#include "CCPACSWing.h"

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>

#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp_Face.hxx>
#include <BRepMesh.hxx>
#include <BRepTools.hxx>

#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

#include <TColgp_Array1OfPnt2d.hxx>
#include <climits>

namespace tigl{

CTiglTriangularizer::CTiglTriangularizer()
{
    useMultipleObjects(false);
}

CTiglTriangularizer::CTiglTriangularizer(TopoDS_Shape& shape, double deflection, bool multipleObj) {
    useMultipleObjects(multipleObj);
    
    triangularizeShape(shape, deflection);
}

int CTiglTriangularizer::triangularizeShape(const TopoDS_Shape& shape, double deflection){
    BRepTools::Clean (shape);
    BRepMesh::Mesh(shape,deflection);
    
    TopExp_Explorer shellExplorer;
    TopExp_Explorer faceExplorer;
    
    for (shellExplorer.Init(shape, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next()) {
        const TopoDS_Shell shell = TopoDS::Shell(shellExplorer.Current());
        
        for (faceExplorer.Init(shell, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
            TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
            unsigned long *vertexList = NULL;
            unsigned long nVertices, iPolyLower, iPolyUpper;
            triangularizeFace(face,vertexList, nVertices, iPolyLower, iPolyUpper);
            if(vertexList) delete[] vertexList;
        } // for faces
        if(_useMultipleObjects) createNewObject();
    } // for shells
    
    return 0;
}

CTiglTriangularizer::CTiglTriangularizer(ITiglGeometricComponent& comp, double deflection, ComponentTraingMode mode) {
    useMultipleObjects(false);
    
    triangularizeComponent(comp, deflection, mode);
}

bool isValidCoord(double c){
    double tolerance = 2.e-4;
    if(c > -tolerance && c < 1+tolerance)
        return true;
    else 
        return false;
}

int CTiglTriangularizer::triangularizeComponent(ITiglGeometricComponent & component, double deflection, ComponentTraingMode mode){
    if (component.GetComponentType() & TIGL_COMPONENT_WING){
        CCPACSWing& wing = dynamic_cast<CCPACSWing&>(component);
        if(int ret=triangularizeWingFace(wing, wing.GetLoftWithLeadingEdge(), deflection, mode)!=0){
            return ret;
        }
    }
    else {
        TopoDS_Shape& shape = component.GetLoft();
        return triangularizeShape(shape, deflection);
    }
    return TIGL_SUCCESS;
}

int CTiglTriangularizer::triangularizeWingFace(CCPACSWing &wing, const TopoDS_Shape& wingShape, double deflection, ComponentTraingMode mode){
    BRepTools::Clean (wingShape);
    BRepMesh::Mesh(wingShape, deflection);
    
    TopExp_Explorer faceExplorer;
    
    for (faceExplorer.Init(wingShape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
        unsigned long * vertexIndexList = NULL;
        unsigned long nVertices, iPolyLower, iPolyUpper;
        triangularizeFace(face, vertexIndexList, nVertices, iPolyLower, iPolyUpper);
        if(nVertices > 0 && vertexIndexList != NULL &&  mode ==  SEGMENT_INFO){
            // compute a central point on the face
            BRepGProp_Face prop(face);
            Standard_Real umin, umax, vmin, vmax;
            prop.Bounds(umin, umax, vmin, vmax);
            
            Standard_Real umean = 0.5*(umin+umax);
            Standard_Real vmean = 0.5*(vmin+vmax);
            
            gp_Pnt centralP; gp_Vec n;
            prop.Normal(umean,vmean,centralP,n);
            
            // check to which segment this face belongs
            int iSegmentFound = 0;
            for(int iSegment = 1 ; iSegment <= wing.GetSegmentCount(); ++iSegment){
                CCPACSWingSegment& segment = (CCPACSWingSegment&) wing.GetSegment(iSegment);
                if(segment.GetIsOn(centralP) == true){
                    iSegmentFound = iSegment;
                    break;
                }
            }
            
            currentObject().setMetadataElements("uID segmentIndex eta xsi isOnTop");
            if(iSegmentFound > 0){
                // determine if this face is an upper segment face
                CCPACSWingSegment& segment = (CCPACSWingSegment&) wing.GetSegment(iSegmentFound);
                
                // GetIsOnTop is very slow, therefore we do it only once per face 
                bool isUpperFace = segment.GetIsOnTop(centralP);
                
                for(unsigned int iPoly = iPolyLower; iPoly <= iPolyUpper; iPoly++){
                    currentObject().setPolyDataReal(iPoly, "is_upper", (double) isUpperFace);
                    currentObject().setPolyDataReal(iPoly, "segment_index", (double) iSegmentFound);
                    
                    unsigned long npoints = currentObject().getNPointsOfPolygon(iPoly);
                    
                    CTiglPoint baryCenter(0.,0.,0.);
                    for(unsigned long jPoint = 0; jPoint < npoints; ++jPoint){
                        unsigned long index = currentObject().getVertexIndexOfPolygon(jPoint, iPoly);
                        baryCenter += currentObject().getVertexPoint(index);
                    }
                    baryCenter = baryCenter*(double)(1./(double)npoints);
                    
                    double eta = 0., xsi = 0.;
                    segment.GetEtaXsi(baryCenter.Get_gp_Pnt(), false, eta, xsi);
                    currentObject().setPolyDataReal(iPoly, "eta", eta);
                    currentObject().setPolyDataReal(iPoly, "xsi", xsi);
                    
                    // create metadata string
                    std::stringstream stream;
                    stream << "\"" << segment.GetUID() << "\" " << iSegmentFound << " " << eta << " " << xsi << " " << isUpperFace;
                    currentObject().setPolyMetadata(iPoly, stream.str().c_str());
                }
            }
            else {
                for(unsigned int iPoly = iPolyLower; iPoly <= iPolyUpper; iPoly++){
                    currentObject().setPolyMetadata(iPoly,"\"\" 0 0.0 0.0 0");
                }
            } // iSegmentFound
        } // advanced mode
        
        if(vertexIndexList) delete[] vertexIndexList;
    } // for faces
        if(_useMultipleObjects) createNewObject();
    
    return 0;
}

int CTiglTriangularizer::triangularizeFace(const TopoDS_Face & face, unsigned long* &indexBuffer, unsigned long &nVertices, unsigned long &iPolyLower, unsigned long &iPolyUpper){
    TopLoc_Location location;
    const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
    
    indexBuffer = NULL;
    if (triangulation.IsNull())
        return 0;
    
    gp_Trsf nodeTransformation = location;
    
    
    unsigned long ilower = 0;
    unsigned long iBufferSize = 0;
    
    if(triangulation->HasUVNodes()){
        // we use the uv nodes to compute normal vectors for each point
        
        BRepGProp_Face prop(face);
        currentObject().enableNormals(true);
        
        const TColgp_Array1OfPnt2d& uvnodes = triangulation->UVNodes(); // get (face-local) list of nodes
        ilower = uvnodes.Lower();
        
        iBufferSize = uvnodes.Upper()-uvnodes.Lower()+1;
        indexBuffer = new unsigned long [iBufferSize];
        unsigned long * pIndexBuf = indexBuffer;
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
        
        iBufferSize = nodes.Upper()-nodes.Lower()+1;
        indexBuffer = new unsigned long [iBufferSize];
        unsigned long * pIndexBuf = indexBuffer;
        for(int inode = nodes.Lower(); inode <= nodes.Upper(); inode++){
            const gp_Pnt& p = nodes(inode).Transformed(nodeTransformation);
            *pIndexBuf++ = currentObject().addPointNormal(p.XYZ(), CTiglPoint(1,0,0));
        }
    }

    
    const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
    iPolyLower = ULONG_MAX;
    iPolyUpper = 0;
    for (int j = triangles.Lower(); j <= triangles.Upper(); j++) // iterate over triangles in the array
    {
        const Poly_Triangle& triangle = triangles(j);
        int occindex1, occindex2, occindex3;
        triangle.Get(occindex1, occindex2, occindex3); // get indices into index1..3
        unsigned long index1, index2, index3;
        index1 = indexBuffer[occindex1-ilower];
        index2 = indexBuffer[occindex2-ilower];
        index3 = indexBuffer[occindex3-ilower];
        
        // @TODO: in some rare cases, 2 indices are the same
        // which means, that we dont have a true triangle.
        // This behaviour might break some export functions.
        // What should we do?
        
        unsigned int iPolyIndex = 0;
        
        if(face.Orientation() == TopAbs_FORWARD)
            iPolyIndex = currentObject().addTriangleByVertexIndex(index1, index2, index3);
        else
            iPolyIndex = currentObject().addTriangleByVertexIndex(index1, index3, index2);
        
        if(iPolyIndex > iPolyUpper) iPolyUpper = iPolyIndex;
        if(iPolyIndex < iPolyLower) iPolyLower = iPolyIndex;
        
    } // for triangles

    nVertices = iBufferSize;
    return 0;
}

void CTiglTriangularizer::useMultipleObjects(bool use){
    _useMultipleObjects = use;
}

} // namespace tigl
