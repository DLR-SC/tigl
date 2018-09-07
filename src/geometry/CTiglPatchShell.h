/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2018-08-22 Jan Kleinert <Jan.Kleinert@dlr.de>
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

// Code adapted from opencascade, BRepOffsetAPI_ThruSections

#include <vector>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>

namespace tigl {

/**
 * @brief The CTiglPatchShell class can be used to create watertight geometries
 * and solids from a TopoDS_Shell consisting of several faces. Side Caps for
 * holes can be added by supplying the boundary of the hole as a TopoDS_Wire.
 * By Default the algorithm tries to make a solid. Use SetMakeSolid(false) to make
 * a closed shell.
 */
class CTiglPatchShell
{
public:
    /**
     * @brief CTiglPatchShell constructor of the shell patcher. Input is a TopoDS_Shell,
     * possibly with holes that are to be patched using this class.
     *
     * @param shell The input shape of the algorithm
     * @param tolerance the tolerance used in the OpenCascade functions
     *
     */
    explicit CTiglPatchShell(TopoDS_Shape const& shell, double tolerance = 1e-6);

    /**
     * @brief Determine, wether the resulting shape shall be a solid or a closed shell
     *
     * @param enabled boolean wether to create a solid or not
     */
    void SetMakeSolid(bool enabled = true) { _makeSolid = enabled; }

    /**
     * @brief AddSideCap lets the user add faces to the shape to close holes. This is
     * done by providing the boundary of the hole as a wire
     *
     * @param boundaryWire the boundary of the hole that needs patching
     */
    void AddSideCap(TopoDS_Wire const&  boundaryWire);

    /**
     * @brief PatchedShape returns the patched shape calculated by the algorithm
     * @return the patched shape
     */
    TopoDS_Shape PatchedShape();

private:
    /**
     * @brief Perform contains the main algorithm
     */
    void Perform();

    /**
     * @brief _inputShell The input faces
     */
    TopoDS_Shape const& _inputShell;

    /**
     * @brief _sidecaps a list of faces created to fill holes using the
     * function AddSideCap
     */
    std::vector<TopoDS_Face> _sidecaps;

    /**
     * @brief _result the resulting shape of the calculation
     */
    TopoDS_Shape _result;

    // some parameters
    bool   _makeSolid    = true;
    double _tolerance    = 1e-6;
    bool   _hasPerformed = false;

};

}
