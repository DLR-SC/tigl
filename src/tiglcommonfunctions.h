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
#include "Standard.hxx"
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
#include "TopoDS_Shape.hxx"
#include "PNamedShape.h"

#include <map>
#include <string>

typedef std::map<std::string, TopoDS_Shape> ShapeMap;

TIGL_EXPORT Standard_Real GetWireLength(const class TopoDS_Wire& wire);

// returns a point on the wire (0 <= alpha <= 1)
TIGL_EXPORT gp_Pnt WireGetPoint(const TopoDS_Wire& wire, double alpha);
TIGL_EXPORT void WireGetPointNormal(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& normal);

TIGL_EXPORT gp_Pnt WireGetPoint2(const TopoDS_Wire& wire, double alpha);
TIGL_EXPORT void WireGetPointNormal2(const TopoDS_Wire& wire, double alpha, gp_Pnt& point, gp_Vec& normal);

// calculates the alpha value for a given point on a wire
TIGL_EXPORT Standard_Real ProjectPointOnWire(const TopoDS_Wire& wire, gp_Pnt p);

// projects a point onto the line (lineStart<->lineStop) and returns the projection parameter
TIGL_EXPORT Standard_Real ProjectPointOnLine(gp_Pnt p, gp_Pnt lineStart, gp_Pnt lineStop);

// returns the number of edges of the current shape
TIGL_EXPORT unsigned int GetNumberOfEdges(const TopoDS_Shape& shape);

// returns the central point of the face
TIGL_EXPORT gp_Pnt GetCentralFacePoint(const class TopoDS_Face& face);

// puts all faces with the same origin to one TopoDS_Compound
// Maps all compounds with its name in the map
TIGL_EXPORT ShapeMap MapFacesToShapeGroups(const PNamedShape shape);
#endif // TIGLCOMMONFUNCTIONS_H
