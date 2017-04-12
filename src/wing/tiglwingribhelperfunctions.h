/*
* Copyright (C) 2016 Airbus Defence and Space
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

#ifndef TIGLWINGRIBHELPERFUNCTIONS_H
#define TIGLWINGRIBHELPERFUNCTIONS_H

#include <string>

#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>


namespace tigl
{

// forward declarations
class CCPACSWingCSStructure;
class CCPACSWingSparPosition;

// Applies the wing transformation to the passed Shape and returns the
// the transformed shape
// TODO: const correctness of CCPACSWingCSStructure
TopoDS_Shape ApplyWingTransformation(const CCPACSWingCSStructure& structure, const TopoDS_Shape& shape);

// Cuts the passed shape with all spars found in the passed structure and
// returns the splitted shape
TopoDS_Shape CutShapeWithSpars(const TopoDS_Shape& shape, const CCPACSWingCSStructure& structure);

// Computes the point for the passed eta on the passed reference line
gp_Pnt GetReferencePoint(const CCPACSWingCSStructure& structure, const std::string& reference, double currentEta);

// Returns the length of the reference wire
double GetRibReferenceLength(const std::string& reference, const CCPACSWingCSStructure& structure);

// Checks whether the passed outer eta point (0 or 1) of the spar lies
// within a section element or not
bool IsOuterSparPointInSection(const std::string& sparUid, double eta, const CCPACSWingCSStructure& structure);

// Computes the rib up vector for the passed eta and start point
gp_Vec GetUpVectorWithoutXRotation(const std::string& ribReference, double currentEta, const gp_Pnt& startPnt,
                                   const std::string& sparPositionUID, const CCPACSWingCSStructure& structure);

// Rotates the passed up-vector by the passed angle around the passed rib direction
void ApplyXRotationToUpVector(double angle, gp_Vec& upVec, const gp_Vec& ribDir);

// Returns the size of the axis aligned bounding box
double GetBoundingBoxSize(const TopoDS_Shape& shape);

// Cuts the passed face with the passed spar geometry and returns the 
// new corner-points p1 and p2 for the new rib cut face
TopoDS_Wire CutFaceWithSpar(TopoDS_Shape& ribCutFace, const TopoDS_Shape& sparGeometry,
                            double bboxSize, TopoDS_Wire& changedWire, const TopoDS_Wire& wire2,
                            const gp_Vec& upVec);

// returns the intersection point of the definition line and the untrimmed rib cut face
gp_Pnt GetRibDefinitionPoint(const std::string& definition, const TopoDS_Face& ribCutFace,
                             const CCPACSWingCSStructure& structure);

// returns the midplane point of the passed spar position
gp_Pnt GetSparMidplanePoint(const CCPACSWingSparPosition& sparPos, const CCPACSWingCSStructure& structure);

// checks whether the passed spar position is part of the reference line
// and throws an exception if not
void CheckSparPositionOnReference(const std::string& sparPositionUID, const std::string& ribReference,
                                  const CCPACSWingCSStructure& structure);

} // namespace

#endif // TIGLWINGRIBHELPERFUNCTIONS_H
