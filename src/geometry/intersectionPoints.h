/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-07-27 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#ifndef INTERSECTIONPOINTS_H
#define INTERSECTIONPOINTS_H

#include <vector>

//This struct is used by GetIntersectionPoint(const TopoDS_Wire& wire1, const TopoDS_Wire& wire2,[...]) in tiglcommonfunctions.h
struct IntersectionPoint {
    double SquareDistance;
    gp_Pnt Center;
};

typedef std::vector<IntersectionPoint> intersectionPointList;

#endif // PNAMEDSHAPE_H
