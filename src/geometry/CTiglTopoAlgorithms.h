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

#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
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
    static TopoDS_Shape CutShellAtUVParameters(const TopoDS_Shape &shape, std::vector<double> uparams, std::vector<double> vparams);

    /**
     * @brief Cuts the surface at the specified parameters into a shell
     * @param surface The surface to be cut
     * @param uparams  U-Parameters to cut
     * @param vparams  V-Parameters to cut
     * @return The surface cut into a shell
     */
    static TopoDS_Shell CutSurfaceAtUVParameters(Handle(Geom_Surface) surface, std::vector<double> uparams, std::vector<double> vparams);

    /**
     * @brief Cuts the surface at c1 discontinuities into a shell
     * @param surface The surface to be cut
     * @return The surface cut into a shell
     */
    static TopoDS_Shell CutSurfaceAtKinks(Handle(Geom_BSplineSurface) surface);

};

} // namespace tigl

#endif // CTIGLTOPOALGORITHMS_H
