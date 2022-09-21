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
* @brief  Class to compute intersection algorithms.
*/

#include "CTiglIntersectionCalculationBase.h"
#include "CTiglError.h"
#include "CTiglShapeCache.h"
#include "tiglcommonfunctions.h"

#include "GeomAPI_IntSS.hxx"
#include "BRep_Tool.hxx"
#include "BRep_Builder.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Solid.hxx"
#include "TopoDS_Compound.hxx"
#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "Geom2d_Line.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI.hxx"
#include "gp_Pln.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "GeomAdaptor_Curve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeVertex.hxx"
#include "BRepAlgoAPI_Section.hxx"
#include "ShapeAnalysis_Wire.hxx"
#include "TopTools_HSequenceOfShape.hxx"
#include "ShapeAnalysis_FreeBounds.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepPrimAPI_MakeHalfSpace.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "BRepProj_Projection.hxx"

#include <sstream>
#include <boost/functional/hash.hpp>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) > (b)) ? (b) : (a))
#endif

//// WARNING: boost::hash gets incorrectly compiled with gcc 4.3.4 - the optimization will produce indeterministic hash values
//// To workaround the problem, we have to avoid optimization of this file

namespace
{
    class HashablePlane
    {
    public:
        HashablePlane(const gp_Pnt& p, const gp_Dir& n)
        {
            gp_Dir nlocal(n);
            // compute hesse normal form
            double d = gp_Vec(n)*gp_Vec(p.XYZ());
            if (d < 0) {
                d *= -1.;
                nlocal.Reverse();
            }

            hash = boost::hash<double>()(d);
            boost::hash_combine(hash, nlocal.X());
            boost::hash_combine(hash, nlocal.Y());
            boost::hash_combine(hash, nlocal.Z());
        }

        size_t HashValue()
        {
            return hash;
        }

    private:
        size_t hash;
    };

    class HashableProjection
    {
    public:
        HashableProjection(const gp_Pnt& x, const gp_Dir& d)
        {
            gp_Vec xvec = gp_Vec(x.XYZ());
            gp_Vec dvec = gp_Vec(d);
            double coeff  = -dvec.Dot(xvec)/dvec.Dot(dvec);
            gp_Vec result = xvec + coeff*dvec;
            hash = boost::hash<double>()(result.X());
            boost::hash_combine(hash, result.Y());
            boost::hash_combine(hash, result.Z());
        }

        size_t HashValue()
        {
            return hash;
        }
    private:
        size_t hash;
    };

    template <typename L, typename R>
    size_t hash_combine_symmetric( L const& LHS, R const& RHS)
    {
        size_t lhs = boost::hash_value(LHS);
        size_t rhs = boost::hash_value(RHS);

        if ( lhs != rhs ) {
            size_t result = max(lhs, rhs);
            boost::hash_combine(result, min(lhs, rhs));
            return result;
        }
        else {
            return lhs;
        }
    }
}


namespace tigl
{

CTiglIntersectionCalculationBase::CTiglIntersectionCalculationBase(CTiglShapeCache& cache,
                                                           const std::string& intersectionID)
    : tolerance(1.0e-7)
{
    // check that intersectionID is in cache
    if (!cache.HasShape(intersectionID)) {
        throw CTiglError("The given intersectionID can not be found.", TIGL_NOT_FOUND);
    }

    try {
        intersectionResult = TopoDS::Compound(cache.GetShape(intersectionID));
    }
    catch (Standard_TypeMismatch&) {
        throw CTiglError("The given intersectionID is invalid.", TIGL_NOT_FOUND);
    }

    id = intersectionID;
}

// Destructor
CTiglIntersectionCalculationBase::~CTiglIntersectionCalculationBase( void )
{
}

// returns total number of intersection lines
int CTiglIntersectionCalculationBase::GetCountIntersectionLines()
{
    TopExp_Explorer wireExplorer(intersectionResult, TopAbs_WIRE);
    int nwires = 0;
    for (; wireExplorer.More(); wireExplorer.Next()) {
        nwires++;
    }
    return nwires;
}

// returns total number of intersection points
int CTiglIntersectionCalculationBase::GetCountIntersectionPoints()
{
    TopExp_Explorer vertexExplorer(intersectionResult, TopAbs_VERTEX);
    int npoints = 0;
    for (; vertexExplorer.More(); vertexExplorer.Next()) {
        npoints++;
    }
    return npoints;
}

// returns total number of intersection faces
int CTiglIntersectionCalculationBase::GetCountIntersectionFaces()
{
    TopExp_Explorer faceExplorer(intersectionResult, TopAbs_FACE);
    int nfaces = 0;
    for (; faceExplorer.More(); faceExplorer.Next()) {
        nfaces++;
    }
    return nfaces;
}

// Gets a point on the intersection line in dependence of a parameter zeta with
// 0.0 <= zeta <= 1.0. For zeta = 0.0 this is the line starting point,
// for zeta = 1.0 the last point on the intersection line.
// numIntersecLine is the number of the Intersection line.
gp_Pnt CTiglIntersectionCalculationBase::GetPoint(double zeta, int wireID = 1)
{
    if (zeta < 0.0 || zeta > 1.0) {
        throw CTiglError("Parameter zeta not in the range 0.0 <= zeta <= 1.0 in CTiglIntersectionCalculation::GetPoint", TIGL_ERROR);
    }

    TopoDS_Wire wire = GetWire(wireID);
    return WireGetPoint(wire, zeta);
}

TopoDS_Wire CTiglIntersectionCalculationBase::GetWire(int wireID)
{
    if (wireID > GetCountIntersectionLines() || wireID < 1){
        throw CTiglError("Invalid wireID in CTiglIntersectionCalculation::GetWire", TIGL_INDEX_ERROR);
    }

    TopExp_Explorer wireExplorer(intersectionResult, TopAbs_WIRE);
    int currentWireID = 1;
    for (; wireExplorer.More(); wireExplorer.Next()) {
        if (wireID == currentWireID) {
            return TopoDS::Wire(wireExplorer.Current());
        }
        currentWireID++;
    }

    throw CTiglError("Cannot retrieve intersection wire in CTiglIntersectionCalculation::GetWire", TIGL_ERROR);
}

TopoDS_Vertex CTiglIntersectionCalculationBase::GetVertex(int vertexID)
{
    if (vertexID > GetCountIntersectionPoints() || vertexID < 1){
        throw CTiglError("Invalid vertexID in CTiglIntersectionCalculation::GetVertex", TIGL_INDEX_ERROR);
    }

    TopExp_Explorer vertexExplorer(intersectionResult, TopAbs_VERTEX);
    int currentVertexID = 1;
    for (; vertexExplorer.More(); vertexExplorer.Next()) {
        if (vertexID == currentVertexID) {
            return TopoDS::Vertex(vertexExplorer.Current());
        }
        currentVertexID++;
    }

    throw CTiglError("Cannot retrieve intersection vertex in CTiglIntersectionCalculation::GetVertex", TIGL_ERROR);
}
///////////////
TopoDS_Face CTiglIntersectionCalculationBase::GetFace(int faceID)
{
    if (faceID > GetCountIntersectionFaces() || faceID < 1){
        throw CTiglError("Invalid faceID in CTiglIntersectionCalculationBase::GetFace", TIGL_INDEX_ERROR);
    }

    TopExp_Explorer faceExplorer(intersectionResult, TopAbs_FACE);
    int currentFaceID = 1;
    for (; faceExplorer.More(); faceExplorer.Next()) {
        if (faceID == currentFaceID) {
            return TopoDS::Face(faceExplorer.Current());
        }
        currentFaceID++;
    }

    throw CTiglError("Cannot retrieve intersection face in CTiglIntersectionCalculationBase::GetFace", TIGL_ERROR);
}
////////////////
////////////////

const std::string& CTiglIntersectionCalculationBase::GetID()
{
    return id;
}

} // end namespace tigl

