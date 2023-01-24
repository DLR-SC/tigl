/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-05-26 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CMAKELOFT_H
#define CMAKELOFT_H

#include "tigl_internal.h"

#include <vector>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
/**
 * @brief The CTiglMakeLoft class is used to create loft topologies using cross sections
 * and optional guide curves. By default, the resulting topology is a solid. In order to
 * prevent closing the loft side faces (and making a solid), you can call
 * CTiglMakeLoft::setMakeSolid(false)
 */
class CTiglMakeLoft
{
public:
    TIGL_EXPORT CTiglMakeLoft(double tolerance = 1e-6, double sameKnotTolerance = 1e-6);
    TIGL_EXPORT CTiglMakeLoft(const TopoDS_Shape& profiles, const TopoDS_Shape& guides, double tolerance = 1e-6, double sameKnotTolerance = 1e-6);
    
    /**
     * @brief Should be called to add sections/profiles to the algorithm.
     *
     * @param profiles A shape that is either a wire or a compound consisting of multiple wires.
     */
    TIGL_EXPORT void addProfiles(const TopoDS_Shape& profiles);
    
    /** 
     * This function may be called to add guide curves to
     * the lofting algo. The shape should be either a wire
     * or a Compound consisting of multiple wires
     */
    TIGL_EXPORT void addGuides(const TopoDS_Shape& guides);
    
    /**
     * @brief Use the function to adjust, if you want to get a closed
     * solid (inclidung the side caps at the inner and outer profile)
     * or just the surface without caps.
     *
     * @param enabled If true, a solid is built.
     */
    TIGL_EXPORT void setMakeSolid(bool enabled);

    /**
     * @brief Use the function to enable the Gordon surface algorithm to
     * interpolate curve networks.
     * If enabled is false, Coons patches will be used instead.
     *
     * @param enabled If true, the Gordon Surface algorithm is used.
     */
    TIGL_EXPORT void useGordonSurfaceAlgorithm(bool enabled);

    /**
     * @brief setMakeSmooth switches, whether the resulting loft will be ruled
     * or smoothed. This switch only applies, if no guide curves are applied.
     *
     * @param enabled Set to true, if smoothing should be enabled.
     */
    TIGL_EXPORT void setMakeSmooth(bool enabled);
    
    TIGL_EXPORT TopoDS_Shape& Shape();
    
    TIGL_EXPORT operator TopoDS_Shape& ();
    
    /**
     * @brief Performs the algorithm. Doesn't need to be called
     * manually.
     */
    TIGL_EXPORT void Perform();

private:
    void makeLoftWithGuides();
    void makeLoftWithoutGuides();

    void makeLoftCoons();
    void makeLoftGordon();

    /**
     * @brief transform the shape to a solid if the correspondig flag is set,
     * transform it to a shell otherwise (wrapper for CTiglPatchShell)
     */
    void CloseShape();
    
    /**
     * \brief Tolerance for the MakeSolid, MakeShells, BRepOffsetAPI_ThruSections
     *        and for checking coincidence of ends of curves.
     *        (Last only relevant in the case of guide curves)
     *        Default Value is 1E-6
     */
    double _myTolerance;
    /**
     * \brief Relative tolerance for comparing knots of opposite curves
     *        If k1 and k2 are values of corresponding knots of opposite curves C1 and C2 
     *        then k1 ~= k2 if Abs(k1-k2)/((k1+k2)/2) <= theTolParam
     *        Only relevant in the case of guide curves
     *        Default Value is 1E-6
     */
    double _mySameKnotTolerance;
    std::vector<TopoDS_Wire> guides, profiles;
    std::vector<Standard_Real> uparams, vparams;
    bool _hasPerformed, _makeSolid, _use_gordon_surface_algorithm;
    bool _makeSmooth = false;
    
    TopoDS_Shape _result;
};

#endif // CMAKELOFT_H
