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

#include "Standard.hxx"
#include "gp_Pnt.hxx"

Standard_Real GetWireLength(const class TopoDS_Wire& wire);

// returns a point on the wire (0 <= alpha <= 1)
gp_Pnt WireGetPoint(const TopoDS_Wire& wire, double alpha);

gp_Pnt WireGetPoint2(const TopoDS_Wire& wire, double alpha);

// calculates the alpha value for a given point on a wire
Standard_Real ProjectPointOnWire(const TopoDS_Wire& wire, gp_Pnt p);

#endif // TIGLCOMMONFUNCTIONS_H
