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
#include "TopoDS_Shape.hxx"
#include <TopoDS_Edge.hxx>
#include "PNamedShape.h"
#include "ListPNamedShape.h"
#include "CCPACSConfiguration.h"
#include "CTiglAbstractPhysicalComponent.h"
#include <TopoDS_Edge.hxx>
#include <Geom_BSplineCurve.hxx>

#include <map>
#include <string>

typedef std::map<std::string, PNamedShape> ShapeMap;

// calculates a wire's circumfence
TIGL_EXPORT Standard_Real GetWireLength(const class TopoDS_Wire& wire);

TIGL_EXPORT Standard_Real GetEdgeLength(const class TopoDS_Edge& edge);

// returns a point on the wire (0 <= alpha <= 1)
TIGL_EXPORT gp_Pnt WireGetPoint(const TopoDS_Wire& wire, double alpha);
TIGL_EXPORT void WireGetPointTangent(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& normal);

TIGL_EXPORT gp_Pnt EdgeGetPoint(const TopoDS_Edge& edge, double alpha);
TIGL_EXPORT void EdgeGetPointTangent(const TopoDS_Edge& edge, double alpha, gp_Pnt& point, gp_Vec& normal);

// calculates the alpha value for a given point on a wire
TIGL_EXPORT Standard_Real ProjectPointOnWire(const TopoDS_Wire& wire, gp_Pnt p);

// projects a point onto the line (lineStart<->lineStop) and returns the projection parameter
TIGL_EXPORT Standard_Real ProjectPointOnLine(gp_Pnt p, gp_Pnt lineStart, gp_Pnt lineStop);

// returns the number of edges of the current shape
TIGL_EXPORT unsigned int GetNumberOfEdges(const TopoDS_Shape& shape);

// returns the number of faces of the current shape
TIGL_EXPORT unsigned int GetNumberOfFaces(const TopoDS_Shape& shape);

TIGL_EXPORT TopoDS_Edge GetEdge(const TopoDS_Shape& shape, int iEdge);

TIGL_EXPORT Handle_Geom_BSplineCurve GetBSplineCurve(const TopoDS_Edge& e);

// Returns the number of subshapes, if the shape is a compound
TIGL_EXPORT unsigned int GetNumberOfSubshapes(const TopoDS_Shape& shape);

// returns the central point of the face
TIGL_EXPORT gp_Pnt GetCentralFacePoint(const class TopoDS_Face& face);

// puts all faces with the same origin to one TopoDS_Compound
// Maps all compounds with its name in the map
TIGL_EXPORT ListPNamedShape GroupFaces(const PNamedShape shape, tigl::ShapeGroupMode groupType);

// Returns the coordinates of the bounding box of the shape
TIGL_EXPORT void GetShapeExtension(const TopoDS_Shape& shape,
                                   double& minx, double& maxx,
                                   double& miny, double& maxy,
                                   double& minz, double& maxz);

// Returns a unique Hashcode for a specific geometric component based on its loft
TIGL_EXPORT int GetComponentHashCode(tigl::ITiglGeometricComponent&);

// Creates an Edge from the given Points by B-Spline interpolation
TIGL_EXPORT TopoDS_Edge EdgeSplineFromPoints(const std::vector<gp_Pnt>& points);

// Returns true, if a path is relative
TIGL_EXPORT bool IsPathRelative(const std::string&);

// Returns true, if a file is readable
TIGL_EXPORT bool IsFileReadable(const std::string& filename);

#endif // TIGLCOMMONFUNCTIONS_H
