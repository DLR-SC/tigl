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
#include "CTiglRelativelyPositionedComponent.h"
#include "CCPACSWing.h"
#include "CCPACSWingSegment.h"
#include "CCPACSConfiguration.h"
#include "CTiglFusePlane.h"

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Face.hxx>

#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_Tool.hxx>
#include <BRepGProp_Face.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepTools.hxx>

#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

#include <TColgp_Array1OfPnt2d.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <climits>

namespace 
{
    gp_Pnt mirrorPoint(gp_Pnt pin, TiglSymmetryAxis axis)
    {
        gp_Pnt p(pin);

        if (axis == TIGL_X_Y_PLANE) {
            p.SetZ(-p.Z());
        }
        else if (axis == TIGL_Y_Z_PLANE) {
            p.SetX(-p.X());
        }
        else if (axis == TIGL_X_Z_PLANE) {
            p.SetY(-p.Y());
        }
        return p;
    }
}

namespace tigl
{

CTiglTriangularizer::CTiglTriangularizer(PNamedShape pshape, double deflection, bool computeNormals)
    : m_computeNormals(computeNormals)
{
    if (!pshape) {
        throw CTiglError("Null pointer shape in CTiglTriangularizer", TIGL_NULL_POINTER);
    }

    triangularizeComponent(NULL, pshape, deflection, NO_INFO);
}

CTiglTriangularizer::CTiglTriangularizer(const CTiglUIDManager* uidMgr, PNamedShape shape, double deflection, ComponentTraingMode mode, bool computeNormals)
    : m_computeNormals(computeNormals)
{
    if (!shape) {
        throw CTiglError("Null pointer shape in CTiglTriangularizer", TIGL_NULL_POINTER);
    }

    triangularizeComponent(uidMgr, shape, deflection, mode);
    
}

bool isValidCoord(double c) 
{
    double tolerance = 2.e-4;
    if (c > -tolerance && c < 1+tolerance) {
        return true;
    }
    else {
        return false;
    }
}

void CTiglTriangularizer::writeFaceDummyMeta(unsigned long iPolyLower, unsigned long iPolyUpper)
{
    for (unsigned int iPoly = iPolyLower; iPoly <= iPolyUpper; iPoly++) {
        polys.currentObject().setPolyDataReal(iPoly, "is_upper", 0.);
        polys.currentObject().setPolyDataReal(iPoly, "segment_index", 0.);
        polys.currentObject().setPolyDataReal(iPoly, "eta", 0.);
        polys.currentObject().setPolyDataReal(iPoly, "xsi", 0.);

        polys.currentObject().setPolyMetadata(iPoly,"\"\" 0 0.0 0.0 0");
    }
}

bool CTiglTriangularizer::writeWingMeta(ITiglGeometricComponent& wingComponent, gp_Pnt centralP, unsigned long iPolyLower, unsigned long iPolyUpper)
{
    if (wingComponent.GetComponentType() == TIGL_COMPONENT_WING) {
        CCPACSWing& wing = dynamic_cast<CCPACSWing&>(wingComponent);
        for (int iSegment = 1 ; iSegment <= wing.GetSegmentCount(); ++iSegment) {
            CCPACSWingSegment& segment = (CCPACSWingSegment&) wing.GetSegment(iSegment);
            if (writeWingSegmentMeta(segment, centralP, iPolyLower, iPolyUpper)) {
                return true;
            }
        }
    }

    return false;
}

void CTiglTriangularizer::writeFaceMeta(const CTiglUIDManager* uidMgr, const std::string& componentUID,
                                        TopoDS_Face face, unsigned long iPolyLower, unsigned long iPolyUpper)
{
    if (!uidMgr) {
        return;
    }

    if (componentUID.empty()) {
        writeFaceDummyMeta(iPolyLower, iPolyUpper);
        return;
    }

    // compute a central point on the face
    BRepGProp_Face prop(face);
    Standard_Real umin, umax, vmin, vmax;
    prop.Bounds(umin, umax, vmin, vmax);

    Standard_Real umean = 0.5*(umin+umax);
    Standard_Real vmean = 0.5*(vmin+vmax);

    gp_Pnt centralP; gp_Vec n;
    prop.Normal(umean,vmean,centralP,n);

    try {
        ITiglGeometricComponent& component = uidMgr->GetGeometricComponent(componentUID);
        if (writeWingMeta(component, centralP, iPolyLower, iPolyUpper)) {
            return;
        }
        
        if (writeWingSegmentMeta(component, centralP, iPolyLower, iPolyUpper)) {
            return;
        }
    }
    catch(CTiglError&) {
        // uid is not a component. do nothing
    }
    writeFaceDummyMeta(iPolyLower, iPolyUpper);
}

int CTiglTriangularizer::triangularizeComponent(const CTiglUIDManager* uidMgr, PNamedShape pshape, double deflection, ComponentTraingMode mode)
{
    if (!pshape) {
        return TIGL_NULL_POINTER;
    }

    TopoDS_Shape shape = pshape->Shape();
    BRepTools::Clean (shape);
    BRepMesh_IncrementalMesh(shape, deflection);
    LOG(INFO) << "Done meshing";

    polys.currentObject().enableNormals(m_computeNormals);

    TopTools_IndexedMapOfShape faceMap;
    TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
    for (int iface = 1; iface <= faceMap.Extent(); ++iface) {
        TopoDS_Face face = TopoDS::Face(faceMap(iface));
        std::string componentUID = pshape->GetFaceTraits(iface-1).ComponentUID();
        unsigned long nVertices, iPolyLower, iPolyUpper;
        triangularizeFace(face, nVertices, iPolyLower, iPolyUpper);

        if (nVertices > 0 && mode ==  SEGMENT_INFO) {
            // write face metadata
            writeFaceMeta(uidMgr, componentUID, face, iPolyLower, iPolyUpper);
        }
    }

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
bool CTiglTriangularizer::writeWingSegmentMeta(tigl::ITiglGeometricComponent &segmentComponent, gp_Pnt pointOnSegmentFace, unsigned long iPolyLower, unsigned long iPolyUpper)
{
    if (!(segmentComponent.GetComponentType() == TIGL_COMPONENT_WINGSEGMENT)) {
        return false;
    }
    
    CCPACSWingSegment& segment = dynamic_cast<CCPACSWingSegment&>(segmentComponent);
    
    bool pointOnMirroredShape = false;
    if (segment.GetIsOn(pointOnSegmentFace) == true) {
        pointOnMirroredShape = false;
    }
    else if (segment.GetSymmetryAxis() != TIGL_NO_SYMMETRY && segment.GetIsOnMirrored(pointOnSegmentFace) == true) {
        pointOnMirroredShape = true;
    }
    else {
        // point is not on this segment
        return false;
    }
    
    if (pointOnMirroredShape) {
        pointOnSegmentFace = mirrorPoint(pointOnSegmentFace, segment.GetSymmetryAxis());
    }
    
    // GetIsOnTop is very slow, therefore we do it only once per face 
    bool isUpperFace = segment.GetIsOnTop(pointOnSegmentFace);
    
    for (unsigned long iPoly = iPolyLower; iPoly <= iPolyUpper; iPoly++) {
        polys.currentObject().setPolyDataReal(iPoly, "is_upper", (double) isUpperFace);
        polys.currentObject().setPolyDataReal(iPoly, "segment_index", (double) segment.GetSegmentIndex());
        
        unsigned long npoints = polys.currentObject().getNPointsOfPolygon(iPoly);
        
        CTiglPoint baryCenter(0.,0.,0.);
        for (unsigned long jPoint = 0; jPoint < npoints; ++jPoint) {
            unsigned long index = polys.currentObject().getVertexIndexOfPolygon(jPoint, iPoly);
            baryCenter += polys.currentObject().getVertexPoint(index);
        }
        baryCenter = baryCenter*(double)(1./(double)npoints);
        if (pointOnMirroredShape) {
            baryCenter = mirrorPoint(baryCenter.Get_gp_Pnt(), segment.GetSymmetryAxis()).XYZ();
        }
        
        double eta = 0., xsi = 0.;
        segment.GetEtaXsi(baryCenter.Get_gp_Pnt(), eta, xsi);
        polys.currentObject().setPolyDataReal(iPoly, "eta", eta);
        polys.currentObject().setPolyDataReal(iPoly, "xsi", xsi);
        
        // create metadata string
        std::stringstream stream;
        std::string symm = "";
        if (pointOnMirroredShape) {
            symm = "_sym";
        }
        stream << "\"" << segment.GetUID() << symm << "\" " << segment.GetSegmentIndex() << " " << eta << " " << xsi << " " << isUpperFace;
        polys.currentObject().setPolyMetadata(iPoly, stream.str().c_str());
    }
    
    return true;
}

int CTiglTriangularizer::triangularizeFace(const TopoDS_Face & face, unsigned long &nVertices, unsigned long &iPolyLower, unsigned long &iPolyUpper)
{
    TopLoc_Location location;
    std::vector<unsigned long> indexBuffer;

    const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);
    if (triangulation.IsNull()) {
        return 0;
    }

    gp_Trsf nodeTransformation = location;

    unsigned long ilower = 0;
    unsigned long iBufferSize = 0;
    
    if (triangulation->HasUVNodes() && m_computeNormals) {
        // we use the uv nodes to compute normal vectors for each point
        
        BRepGProp_Face prop(face);
        
        const TColgp_Array1OfPnt2d& uvnodes = triangulation->UVNodes(); // get (face-local) list of nodes
        ilower = uvnodes.Lower();
        
        iBufferSize = uvnodes.Upper()-uvnodes.Lower()+1;
        indexBuffer.reserve(iBufferSize);
        for (int inode = uvnodes.Lower(); inode <= uvnodes.Upper(); ++inode) {
            const gp_Pnt2d& uv_pnt = uvnodes(inode);
            gp_Pnt p; gp_Vec n;
            prop.Normal(uv_pnt.X(),uv_pnt.Y(),p,n);
            if (n.SquareMagnitude() > 0.) {
                n.Normalize();
            }
            if (face.Orientation() == TopAbs_INTERNAL) {
                n.Reverse();
            }
            indexBuffer.push_back(polys.currentObject().addPointNormal(p.XYZ(), n.XYZ()));
        }
    } 
    else {
        // we cannot compute normals
        
        const TColgp_Array1OfPnt& nodes = triangulation->Nodes(); // get (face-local) list of nodes
        ilower = nodes.Lower();

        iBufferSize = nodes.Upper()-nodes.Lower()+1;
        indexBuffer.reserve(iBufferSize);
        for (int inode = nodes.Lower(); inode <= nodes.Upper(); inode++) {
            const gp_Pnt& p = nodes(inode).Transformed(nodeTransformation);
            indexBuffer.push_back(polys.currentObject().addPointNormal(p.XYZ(), CTiglPoint(1,0,0)));
        }
    }

    const Poly_Array1OfTriangle& triangles = triangulation->Triangles();
    iPolyLower = ULONG_MAX;
    iPolyUpper = 0;
    // iterate over triangles in the array 
    for (int j = triangles.Lower(); j <= triangles.Upper(); j++) {
        const Poly_Triangle& triangle = triangles(j);
        int occindex1, occindex2, occindex3;
        triangle.Get(occindex1, occindex2, occindex3); // get indices into index1..3
        unsigned long index1, index2, index3;
        index1 = indexBuffer[occindex1-ilower];
        index2 = indexBuffer[occindex2-ilower];
        index3 = indexBuffer[occindex3-ilower];

        unsigned long iPolyIndex = 0;

        if (face.Orientation() != TopAbs_REVERSED && face.Orientation() != TopAbs_INTERNAL) {
            iPolyIndex = polys.currentObject().addTriangleByVertexIndex(index1, index2, index3);
        }
        else {
            iPolyIndex = polys.currentObject().addTriangleByVertexIndex(index1, index3, index2);
        }

        // In some rare cases, 2 indices are the same
        // which means, that we dont have a true triangle.
        // Ignore this triangle
        if (iPolyIndex == ULONG_MAX) {
            continue;
        }

        if (iPolyIndex > iPolyUpper) {
            iPolyUpper = iPolyIndex;
        }
        if (iPolyIndex < iPolyLower) {
            iPolyLower = iPolyIndex;
        }
        
    } // for triangles

    nVertices = iBufferSize;
    return 0;
}

} // namespace tigl
