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

#pragma once

namespace tigl
{
class CCPACSWingComponentSegment;

class CCPACSWingComponentSegmentLineFunctionality
{
public:
    // Getter for midplane point
    TIGL_EXPORT gp_Pnt GetMidplanePoint(double eta, double xsi);

    // Getter for the extended eta line
    TIGL_EXPORT const TopoDS_Wire& GetEtaLine();

    // Getter for the extended eta line
    TIGL_EXPORT const TopoDS_Wire& GetExtendedEtaLine();

    // Getter for the leading edge line
    TIGL_EXPORT const TopoDS_Wire& GetLeadingEdgeLine();

    // Getter for the trailing edge line
    TIGL_EXPORT const TopoDS_Wire& GetTrailingEdgeLine();

    // Getter for the extended leading edge line
    TIGL_EXPORT const TopoDS_Wire& GetExtendedLeadingEdgeLine();

    // Getter for the extended trailing edge line
    TIGL_EXPORT const TopoDS_Wire& GetExtendedTrailingEdgeLine();

    // Getter for leading edge point
    TIGL_EXPORT gp_Pnt GetLeadingEdgePoint(double eta);

    // Getter for trailing edge point
    TIGL_EXPORT gp_Pnt GetTrailingEdgePoint(double eta);

    // Getter for inner chordline point
    TIGL_EXPORT gp_Pnt GetInnerChordlinePoint(double xsi);

    // Getter for outer chordline point
    TIGL_EXPORT gp_Pnt GetOuterChordlinePoint(double xsi);

    // Getter for the length of the leading edge
    TIGL_EXPORT double GetLeadingEdgeLength();

    // Getter for the length of the trailing edge
    TIGL_EXPORT double GetTrailingEdgeLength();

    // Getter for eta value for passed point
    TIGL_EXPORT double GetMidplaneEta(const gp_Pnt& p);

    // Getter for eta direction of midplane (no 
    //             X-component)
    TIGL_EXPORT gp_Vec GetMidplaneEtaDir(double eta);

    // Getter for midplane normal vector
    TIGL_EXPORT gp_Vec GetMidplaneNormal(double eta);

protected:
    CCPACSWingComponentSegmentLineFunctionality(const CCPACSWingComponentSegment& compSegment);

    void Invalidate();

private:
    void CheckLines();

    // Method for building wires for eta-, leading edge-, trailing edge-lines
    void BuildLines();

private:
    const CCPACSWingComponentSegment& compSegment;

    TopoDS_Wire  etaLine;                  // 2d version (in YZ plane) of leadingEdgeLine
    TopoDS_Wire  extendedEtaLine;          // 2d version (in YZ plane) of extendedLeadingEdgeLine
    TopoDS_Wire  leadingEdgeLine;          // leading edge as wire
    TopoDS_Wire  extendedLeadingEdgeLine;  // leading edge extended along y-axis, see CPACS spar geometry definition
    TopoDS_Wire  trailingEdgeLine;         // trailing edge as wire
    TopoDS_Wire  extendedTrailingEdgeLine; // trailing edge extended along y-axis, see CPACS spar geometry definition
    bool         linesAreValid;
};
}
