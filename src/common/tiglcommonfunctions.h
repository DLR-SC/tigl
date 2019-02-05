/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-06-01 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef TIGLCOMMONFUNCTIONS_H
#define TIGLCOMMONFUNCTIONS_H

#include "tigl_internal.h"
#include "CCPACSImportExport.h"
#include "Standard.hxx"
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
#include "gp_Pln.hxx"
#include "TopoDS_Shape.hxx"
#include <TopoDS_Edge.hxx>
#include "PNamedShape.h"
#include "ListPNamedShape.h"
#include "CTiglIntersectionPoint.h"
#include "CCPACSConfiguration.h"
#include "CTiglRelativelyPositionedComponent.h"
#include <TopoDS_Edge.hxx>
#include <Geom_BSplineCurve.hxx>
#include <TopTools_ListOfShape.hxx>
#include "TColgp_HArray1OfPnt.hxx"

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include "UniquePtr.h"

typedef std::map<std::string, PNamedShape> ShapeMap;

// helper function for std::find
struct IsInsideTolerance
{
    IsInsideTolerance(double value, double tolerance = 1e-15)
        : _a(value), _tol(tolerance)
    {}

    bool operator()(double v)
    {
        return (fabs(_a - v) <= _tol);
    }

    double _a;
    double _tol;
};

// calculates a wire's circumfence
TIGL_EXPORT Standard_Real GetLength(const TopoDS_Wire& wire);

TIGL_EXPORT Standard_Real GetLength(const TopoDS_Edge& edge);

// returns a point on the wire (0 <= alpha <= 1)
TIGL_EXPORT gp_Pnt WireGetPoint(const TopoDS_Wire& wire, double alpha);
TIGL_EXPORT void WireGetPointTangent(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& normal);

// returns the starting point of the wire/edge
TIGL_EXPORT gp_Pnt GetFirstPoint(const TopoDS_Shape& wireOrEdge);
TIGL_EXPORT gp_Pnt GetFirstPoint(const TopoDS_Wire& w);
TIGL_EXPORT gp_Pnt GetFirstPoint(const TopoDS_Edge& e);

// returns the end point of the wire/edge
TIGL_EXPORT gp_Pnt GetLastPoint(const TopoDS_Shape& wireOrEdge);
TIGL_EXPORT gp_Pnt GetLastPoint(const TopoDS_Wire& w);
TIGL_EXPORT gp_Pnt GetLastPoint(const TopoDS_Edge& e);

TIGL_EXPORT gp_Pnt EdgeGetPoint(const TopoDS_Edge& edge, double alpha);
TIGL_EXPORT void EdgeGetPointTangent(const TopoDS_Edge& edge, double alpha, gp_Pnt& point, gp_Vec& normal);

// calculates the alpha value for a given point on a wire
TIGL_EXPORT Standard_Real ProjectPointOnWire(const TopoDS_Wire& wire, gp_Pnt p);

// projects a point onto the line (lineStart<->lineStop) and returns the projection parameter
TIGL_EXPORT Standard_Real ProjectPointOnLine(gp_Pnt p, gp_Pnt lineStart, gp_Pnt lineStop);

enum IntStatus
{
    BetweenPoints, // The intersection point lies between p1 and p2
    OutsideBefore, // The intersection point lies before p1
    OutsideAfter,  // The intersection point lies after p2
    NoIntersection // the plane and the line are parallel to each other
};

// returns the intersection point between a line (p1-p2) and the plane
TIGL_EXPORT IntStatus IntersectLinePlane(gp_Pnt p1, gp_Pnt p2, gp_Pln plane, gp_Pnt& result);

// returns the number of edges of the current shape
TIGL_EXPORT unsigned int GetNumberOfEdges(const TopoDS_Shape& shape);

// returns the number of faces of the current shape
TIGL_EXPORT unsigned int GetNumberOfFaces(const TopoDS_Shape& shape);

TIGL_EXPORT TopoDS_Edge GetEdge(const TopoDS_Shape& shape, int iEdge);

TIGL_EXPORT TopoDS_Face GetFace(const TopoDS_Shape& shape, int iFace);

TIGL_EXPORT Handle(Geom_BSplineCurve) GetBSplineCurve(const TopoDS_Edge& e);

// Returns the number of subshapes, if the shape is a compound
TIGL_EXPORT unsigned int GetNumberOfSubshapes(const TopoDS_Shape& shape);

// returns the central point of the face
TIGL_EXPORT gp_Pnt GetCentralFacePoint(const class TopoDS_Face& face);

// puts all faces with the same origin to one TopoDS_Compound
// Maps all compounds with its name in the map
TIGL_EXPORT ListPNamedShape GroupFaces(const PNamedShape shape, tigl::ShapeGroupMode groupType);

TIGL_EXPORT TopoDS_Shape GetFacesByName(const PNamedShape shape, const std::string& name);

// Returns the coordinates of the bounding box of the shape
TIGL_EXPORT void GetShapeExtension(const TopoDS_Shape& shape,
                                   double& minx, double& maxx,
                                   double& miny, double& maxy,
                                   double& minz, double& maxz);

// Returns a unique Hashcode for a specific geometric component based on its loft
TIGL_EXPORT int GetComponentHashCode(tigl::ITiglGeometricComponent&);

// Creates an Edge from the given Points by B-Spline interpolation
TIGL_EXPORT TopoDS_Edge EdgeSplineFromPoints(const std::vector<gp_Pnt>& points);

// Computes the intersection point of a face and an edge
TIGL_EXPORT bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Edge& edge, gp_Pnt& dst, double tolerance = Precision::Confusion());

// Computes the intersection point of a face and a wire
TIGL_EXPORT bool GetIntersectionPoint(const TopoDS_Face& face, const TopoDS_Wire& wire, gp_Pnt& dst, double tolerance = Precision::Confusion());

// Comuptes the intersection points of two wires
TIGL_EXPORT bool GetIntersectionPoint(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, intersectionPointList& intersectionPoints, const double tolerance=Precision::SquareConfusion());

// Checks, whether a points lies inside a given shape, which must be a solid
TIGL_EXPORT bool IsPointInsideShape(const TopoDS_Shape& solid, gp_Pnt point);

// Returns the single face contained in the passed shape
// Throws an exception when number of faces != 1
TIGL_EXPORT TopoDS_Face GetSingleFace(const TopoDS_Shape& shape);

// Builds a face out of 4 points
TIGL_EXPORT TopoDS_Face BuildFace(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3, const gp_Pnt& p4);

// Returns true, if a path is relative
TIGL_EXPORT bool IsPathRelative(const std::string&);

// Returns true, if a file is readable
TIGL_EXPORT bool IsFileReadable(const std::string& filename);

TIGL_EXPORT std::string FileExtension(const std::string& filename);

// get the continuity of two edges which share a common vertex
TIGL_EXPORT TiglContinuity getEdgeContinuity(const TopoDS_Edge& edge1, const TopoDS_Edge& edge2);

TIGL_EXPORT Standard_Boolean IsEqual(const TopoDS_Shape& s1, const TopoDS_Shape& s2);

// Method for building a face out of two wires
TIGL_EXPORT TopoDS_Face BuildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

// Method for building a face out of two wires and a direction vector
TIGL_EXPORT TopoDS_Face BuildFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir);

// Method for building a face out of closed wire.
// The algorithm tries to build a face using a plane as surface, in case this fails
// the BRepFill_Filling class is used.
TIGL_EXPORT TopoDS_Face BuildFace(const TopoDS_Wire& wire);

// Method for building a ruled face between the two wires. The method
// approximates the two wires by curves, and generates a single face
// between these curves.
// This can be used for generating non-planar faces. For planar faces
// see buildFace method
TIGL_EXPORT TopoDS_Face BuildRuledFace(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

// Method for building a wire out of two points
TIGL_EXPORT TopoDS_Wire BuildWire(const gp_Pnt& p1, const gp_Pnt& p2);

// Method for building a wire out of the edges from the passed geometry
TIGL_EXPORT TopoDS_Wire BuildWireFromEdges(const TopoDS_Shape& edges);

// Returns a list of wires built from all connected edges in the passed shape
TIGL_EXPORT void BuildWiresFromConnectedEdges(const TopoDS_Shape& shape, TopTools_ListOfShape& wireList);

// Method for creating a face from an opened wire
TIGL_EXPORT TopoDS_Wire CloseWire(const TopoDS_Wire& wire);

// Method for closing two wires to a single one, 
// The method determines a direction vector based on the end vertices of wire1
// and calls the second closeWires method
TIGL_EXPORT TopoDS_Wire CloseWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2);

// Method for closing two wires to a single one, 
// the passed vector is used to define the upper and lower end vertices of the wires
TIGL_EXPORT TopoDS_Wire CloseWires(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2, const gp_Vec& dir);

// Method for sorting the edges of a wire
TIGL_EXPORT TopoDS_Wire SortWireEdges(const TopoDS_Wire& wire);

// Returns the first and last vertex of the passed shape along the passed 
// direction
TIGL_EXPORT bool GetMinMaxPoint(const TopoDS_Shape& shape, const gp_Vec& dir, gp_Pnt& minPnt, gp_Pnt& maxPnt);

// Returns the list of shapes of the passed type from the passed shape
TIGL_EXPORT void GetListOfShape(const TopoDS_Shape& shape, TopAbs_ShapeEnum type, TopTools_ListOfShape& result);

// Returns all shapes with the given type contained in the given shape
TIGL_EXPORT std::vector<TopoDS_Shape> GetSubShapes(const TopoDS_Shape& shape, TopAbs_ShapeEnum type);

// Cuts two shapes and returns the common geometry (e.g. intersection edges)
// Throws an exception in case the interesection failed
TIGL_EXPORT TopoDS_Shape CutShapes(const TopoDS_Shape& shape1, const TopoDS_Shape& shape2);

// Helper for splitting a shape by another shape
TIGL_EXPORT TopoDS_Shape SplitShape(const TopoDS_Shape& src, const TopoDS_Shape& tool);

// Method for finding all directly and indirectly connected edges
// The method loops over the passed edgeList and checks for each element if it
// is connected to the passed edge. When an edge is found it is removed from 
// the edgeList and added to the targetList. Additionally for this edge all
// connected edges are also added to the targetList by recursively calling this
// method. Finally all directly or indirectly connected edges to the passed
// edge are moved from the edgeList to the targetList
TIGL_EXPORT void FindAllConnectedEdges(const TopoDS_Edge& edge, TopTools_ListOfShape& edgeList, TopTools_ListOfShape& targetList);

// Method for checking if two edges have a common vertex (same position)
TIGL_EXPORT bool CheckCommonVertex(const TopoDS_Edge& e1, const TopoDS_Edge& e2);

// Method for searching all vertices which are only connected to a single edge
TIGL_EXPORT void GetEndVertices(const TopoDS_Shape& shape, TopTools_ListOfShape& endVertices);

// Method for finding the face which has the lowest distance to the passed point
TIGL_EXPORT TopoDS_Face GetNearestFace(const TopoDS_Shape& src, const gp_Pnt& pnt);

// Method for finding the center of mass of a shape
TIGL_EXPORT gp_Pnt GetCenterOfMass(const TopoDS_Shape& shape);

// Method for finding the area of a shape or the area that is framed by the shape
TIGL_EXPORT double GetArea(const TopoDS_Shape &shape);

// Method for checking for duplicate edges in the passed shape.
// The method returns a shape with only unique edges
// NOTE: THIS METHOD ONLY CHECKS THE VERTEX POSITIONS, AND THE MIDDLE POINT 
//       OF THE EDGES, BUT DOES NOT COMPARE THE CURVES EXACTLY
TIGL_EXPORT TopoDS_Shape RemoveDuplicateEdges(const TopoDS_Shape& shape);

inline double Radians(double degree)
{
    return degree / 180. * M_PI;
}

inline double Degrees(double radians)
{
    return 180.*radians / M_PI;
}

// Clamps val between min and max
TIGL_EXPORT int Clamp(int val, int min, int max);
TIGL_EXPORT double Clamp(double val, double min, double max);
TIGL_EXPORT size_t Clamp(size_t val, size_t min, size_t max);

// Creates a linear spaces array but with some additional breaking points
// If the breaking points are very close to a point, the point will be replaced
// Else, the breaking point will be inserted
TIGL_EXPORT std::vector<double> LinspaceWithBreaks(double umin, double umax, size_t n_values, const std::vector<double>& breaks);

// Transforms a shape accourding to the given coordinate transformation
TIGL_EXPORT TopoDS_Shape TransformedShape(const tigl::CTiglTransformation& transformationToGlobal, TiglCoordinateSystem cs, const TopoDS_Shape& shape);
TIGL_EXPORT TopoDS_Shape TransformedShape(const tigl::CTiglRelativelyPositionedComponent& component, TiglCoordinateSystem cs, const TopoDS_Shape& shape);

TIGL_EXPORT Handle(TColgp_HArray1OfPnt) OccArray(const std::vector<gp_Pnt>& pnts);

template <typename T>
size_t IndexFromUid(const std::vector<tigl::unique_ptr<T> >& vectorOfPointers, const std::string& uid)
{
    struct is_uid { 
        is_uid(const std::string& uid) : m_uid(uid){}
        bool operator()(const tigl::unique_ptr<T>& ptr)
        { 
            return ptr->GetUID() == m_uid;
        }
        std::string m_uid;
    }; 
    
    typename std::vector<tigl::unique_ptr<T> >::const_iterator found = std::find_if(vectorOfPointers.begin(), vectorOfPointers.end(), is_uid(uid));
    return found - vectorOfPointers.begin();
}

#endif // TIGLCOMMONFUNCTIONS_H
