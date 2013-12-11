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
#include "CTiglAbstractPhysicalComponent.h"
#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"

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

namespace {
gp_Pnt mirrorPoint(gp_Pnt pin, TiglSymmetryAxis axis){
    gp_Pnt p(pin);

    if(axis == TIGL_X_Y_PLANE){
        p.SetZ(-p.Z());
    }
    else if(axis == TIGL_Y_Z_PLANE){
        p.SetX(-p.X());
    }
    else if(axis == TIGL_X_Z_PLANE){
        p.SetY(-p.Y());
    }
    return p;
}
}

namespace tigl{

CTiglTriangularizer::CTiglTriangularizer()
{
    useMultipleObjects(false);
}

CTiglTriangularizer::CTiglTriangularizer(TopoDS_Shape& shape, double deflection, bool multipleObj) {
    useMultipleObjects(multipleObj);
    
    BRepMesh::Mesh(shape,deflection);
    triangularizeShape(shape);
}

int CTiglTriangularizer::triangularizeShape(const TopoDS_Shape& shape){
    TopExp_Explorer shellExplorer;
    TopExp_Explorer faceExplorer;
    
    for (shellExplorer.Init(shape, TopAbs_SHELL); shellExplorer.More(); shellExplorer.Next()) {
        const TopoDS_Shell shell = TopoDS::Shell(shellExplorer.Current());
        
        for (faceExplorer.Init(shell, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
            TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
            unsigned long nVertices, iPolyLower, iPolyUpper;
            triangularizeFace(face, nVertices, iPolyLower, iPolyUpper);
        } // for faces
        if(_useMultipleObjects) createNewObject();
    } // for shells
    
    return 0;
}

CTiglTriangularizer::CTiglTriangularizer(CTiglAbstractPhysicalComponent& comp, double deflection, ComponentTraingMode mode) {
    useMultipleObjects(false);
    LOG(INFO) << "Calculating fused plane";
    triangularizeComponent(comp, false, comp.GetLoft(), deflection, mode);
}

CTiglTriangularizer::CTiglTriangularizer(CCPACSConfiguration &config, bool fuseShapes, double deflection, ComponentTraingMode mode) {
    if(fuseShapes){
        CTiglAbstractPhysicalComponent* pRoot =  config.GetUIDManager().GetRootComponent();
        TopoDS_Shape& planeShape = config.GetFusedAirplane();
        useMultipleObjects(false);
        triangularizeComponent(*pRoot, true, planeShape, deflection, mode);
    }
    else {
        useMultipleObjects(false);
        for(int iWing = 1; iWing <= config.GetWingCount(); ++iWing){
            CCPACSWing& wing = config.GetWing(iWing);

            TopoDS_Shape& wshape = wing.GetLoft();
            BRepMesh::Mesh(wshape,deflection);
            triangularizeShape(wshape);

            if(wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY)
                continue;

            TopoDS_Shape wshape_m = wing.GetMirroredLoft();
            BRepMesh::Mesh(wshape_m,deflection);
            triangularizeShape(wshape_m);
        }

        for(int iFuselage = 1; iFuselage <= config.GetFuselageCount(); ++iFuselage){
            CCPACSFuselage& fuselage = config.GetFuselage(iFuselage);

            TopoDS_Shape& wshape = fuselage.GetLoft();
            BRepMesh::Mesh(wshape,deflection);
            triangularizeShape(wshape);

            if(fuselage.GetSymmetryAxis() == TIGL_NO_SYMMETRY)
                continue;

            TopoDS_Shape wshape_m = fuselage.GetMirroredLoft();
            BRepMesh::Mesh(wshape_m,deflection);
            triangularizeShape(wshape_m);
        }
    }
}

bool isValidCoord(double c){
    double tolerance = 2.e-4;
    if(c > -tolerance && c < 1+tolerance)
        return true;
    else 
        return false;
}

int CTiglTriangularizer::triangularizeComponent(CTiglAbstractPhysicalComponent & component, bool include_childs, TopoDS_Shape& shape, double deflection, ComponentTraingMode mode){
    // create list of child components
    CTiglAbstractPhysicalComponent::ChildContainerType allcomponents = component.GetChildren(true);
    allcomponents.push_front(&component);
    
    BRepTools::Clean (shape);
    BRepMesh::Mesh(shape, deflection);
    LOG(INFO) << "Done meshing";
    
    TopExp_Explorer faceExplorer;
    for (faceExplorer.Init(shape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
        TopoDS_Face face = TopoDS::Face(faceExplorer.Current());
        unsigned long nVertices, iPolyLower, iPolyUpper;
        triangularizeFace(face, nVertices, iPolyLower, iPolyUpper);
        
        // find to which segment the face belongs
        if(nVertices > 0  &&  mode ==  SEGMENT_INFO){
            // compute a central point on the face
            BRepGProp_Face prop(face);
            Standard_Real umin, umax, vmin, vmax;
            prop.Bounds(umin, umax, vmin, vmax);
            
            Standard_Real umean = 0.5*(umin+umax);
            Standard_Real vmean = 0.5*(vmin+vmax);
            
            gp_Pnt centralP; gp_Vec n;
            prop.Normal(umean,vmean,centralP,n);
            
            // search to which component the current face belongs to
            bool found = false;
            CTiglAbstractPhysicalComponent::ChildContainerType::iterator compit;
            for (compit = allcomponents.begin(); compit != allcomponents.end(); ++compit){
                CTiglAbstractPhysicalComponent& curcomp = *(*compit);
                if (curcomp.GetComponentType() & TIGL_COMPONENT_WING){
                    // check to which segment this face belongs
                    CCPACSWing& wing = dynamic_cast<CCPACSWing&>(curcomp);
                    int iSegmentFound = 0;
                    bool pointOnMirroredShape = false;
                    for(int iSegment = 1 ; iSegment <= wing.GetSegmentCount(); ++iSegment){
                        CCPACSWingSegment& segment = (CCPACSWingSegment&) wing.GetSegment(iSegment);
                        if(segment.GetIsOn(centralP) == true){
                            iSegmentFound = iSegment;
                            break;
                        }
                        else if(wing.GetSymmetryAxis() != TIGL_NO_SYMMETRY && segment.GetIsOnMirrored(centralP) == true){
                            iSegmentFound = iSegment;
                            pointOnMirroredShape = true;
                            break;
                        }
                    }
                    if (iSegmentFound > 0){
                        CCPACSWingSegment& segment = (CCPACSWingSegment&) wing.GetSegment(iSegmentFound);
                        annotateWingSegment(segment, centralP, pointOnMirroredShape, iPolyLower, iPolyUpper);
                        found = true;
                        break;
                    }
                } // is wing
            } // for components
            if ( !found ){
                //make dummy annotation for non wing faces
                for(unsigned int iPoly = iPolyLower; iPoly <= iPolyUpper; iPoly++){
                    currentObject().setPolyMetadata(iPoly,"\"\" 0 0.0 0.0 0");
                }
            } // ! found
        }
    }
    if(_useMultipleObjects) createNewObject();

    return TIGL_SUCCESS;
}


/**
 * @brief CTiglTriangularizer::annotateWingSegment Determines for polys in range iPolyLower...iPolyUpper the segment information, including segment index,
 * eta/xsi coordinates and if the point is on the upper or lower side of the wing segment.
 * @param segment All polygons must lie on the specified segment, else we will get wrong data.
 * @param centralP A point on the face of the segment. Is used to determine if the face in an upper or lower face.
 * @param iPolyLower Lower index of the polygons to annotate.
 * @param iPolyUpper Upper index of the polygons to annotate.
 */
void CTiglTriangularizer::annotateWingSegment(tigl::CCPACSWingSegment &segment, gp_Pnt pointOnSegmentFace, bool pointOnMirroredShape, unsigned long iPolyLower, unsigned long iPolyUpper){
    if(pointOnMirroredShape)
        pointOnSegmentFace = mirrorPoint(pointOnSegmentFace, segment.GetSymmetryAxis());
    
    // GetIsOnTop is very slow, therefore we do it only once per face 
    bool isUpperFace = segment.GetIsOnTop(pointOnSegmentFace);
    
    for(unsigned long iPoly = iPolyLower; iPoly <= iPolyUpper; iPoly++){
        currentObject().setPolyDataReal(iPoly, "is_upper", (double) isUpperFace);
        currentObject().setPolyDataReal(iPoly, "segment_index", (double) segment.GetSegmentIndex());
        
        unsigned long npoints = currentObject().getNPointsOfPolygon(iPoly);
        
        CTiglPoint baryCenter(0.,0.,0.);
        for(unsigned long jPoint = 0; jPoint < npoints; ++jPoint){
            unsigned long index = currentObject().getVertexIndexOfPolygon(jPoint, iPoly);
            baryCenter += currentObject().getVertexPoint(index);
        }
        baryCenter = baryCenter*(double)(1./(double)npoints);
        if(pointOnMirroredShape)
            baryCenter = mirrorPoint(baryCenter.Get_gp_Pnt(), segment.GetSymmetryAxis()).XYZ();
        
        double eta = 0., xsi = 0.;
        segment.GetEtaXsi(baryCenter.Get_gp_Pnt(), false, eta, xsi);
        currentObject().setPolyDataReal(iPoly, "eta", eta);
        currentObject().setPolyDataReal(iPoly, "xsi", xsi);
        
        // create metadata string
        std::stringstream stream;
        std::string symm = "";
        if(pointOnMirroredShape)
            symm = "_sym";
        stream << "\"" << segment.GetUID() << symm << "\" " << segment.GetSegmentIndex() << " " << eta << " " << xsi << " " << isUpperFace;
        currentObject().setPolyMetadata(iPoly, stream.str().c_str());
    }
}

int CTiglTriangularizer::triangularizeFace(const TopoDS_Face & face, unsigned long &nVertices, unsigned long &iPolyLower, unsigned long &iPolyUpper){
    TopLoc_Location location;
    unsigned long* indexBuffer = NULL;
    
    const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
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
            if(n.SquareMagnitude() > 0.)
                n.Normalize();
            if(face.Orientation() == TopAbs_INTERNAL)
                n.Reverse();
            
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
        
        if(face.Orientation() != TopAbs_REVERSED && face.Orientation() != TopAbs_INTERNAL)
            iPolyIndex = currentObject().addTriangleByVertexIndex(index1, index2, index3);
        else
            iPolyIndex = currentObject().addTriangleByVertexIndex(index1, index3, index2);
        
        if(iPolyIndex > iPolyUpper) iPolyUpper = iPolyIndex;
        if(iPolyIndex < iPolyLower) iPolyLower = iPolyIndex;
        
    } // for triangles

    delete[] indexBuffer;
    nVertices = iBufferSize;
    return 0;
}

void CTiglTriangularizer::useMultipleObjects(bool use){
    _useMultipleObjects = use;
}

} // namespace tigl
