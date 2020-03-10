#ifndef CTIGLTOPOALGORITHMS_H
#define CTIGLTOPOALGORITHMS_H
/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
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

#include "tigl_internal.h"
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Wire.hxx>
#include <Geom_BSplineSurface.hxx>
#include <vector>

namespace tigl
{

class CTiglTopoAlgorithms
{
public:

    /**
     * @brief Cuts all faces of shell at the specified parameters.
     * @param shape The shape to be cut
     * @param uparams  U-Parameters to cut
     * @param vparams  V-Parameters to cut
     * @return The shape at cutted params
     */
    TIGL_EXPORT static TopoDS_Shape CutShellAtUVParameters(const TopoDS_Shape &shape, std::vector<double> uparams, std::vector<double> vparams);

    /**
     * @brief Cuts the surface at the specified parameters into a shell
     * @param surface The surface to be cut
     * @param uparams  U-Parameters to cut
     * @param vparams  V-Parameters to cut
     * @return The surface cut into a shell
     */
    TIGL_EXPORT static TopoDS_Shell CutSurfaceAtUVParameters(Handle(Geom_Surface) surface, std::vector<double> uparams, std::vector<double> vparams);

    TIGL_EXPORT static void CutSurfaceAtUVParameters(Handle(Geom_Surface) surface, std::vector<double> uparams, std::vector<double> vparams, TopoDS_Shell& shell);


    /**
     * @brief Cuts the surface at c1 discontinuities into a shell
     * @param shape The shape to be cut
     * @return The cutted shape
     */
    TIGL_EXPORT static TopoDS_Shape CutShellAtKinks(const TopoDS_Shape &shape);

    /**
     * @brief Cuts the surface at c1 discontinuities into a shell
     * @param surface The surface to be cut
     * @return The surface cut into a shell
     */
    TIGL_EXPORT static TopoDS_Shell CutSurfaceAtKinks(Handle(Geom_BSplineSurface) surface);

    /**
     * @brief Cuts the surface at c1 discontinuities. The resulting faces are added to the shell
     * @param surface The surface to be cut
     * @param shell The shell to add the faces
    */
    TIGL_EXPORT static void CutSurfaceAtKinks(Handle(Geom_BSplineSurface) surface, TopoDS_Shell& shell);

    TIGL_EXPORT static bool IsDegenerated(const TopoDS_Wire& wire);
};

} // namespace tigl

#endif // CTIGLTOPOALGORITHMS_H
