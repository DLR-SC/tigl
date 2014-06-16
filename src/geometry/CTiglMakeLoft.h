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
    CTiglMakeLoft(double tolerance = 1e-6);
    CTiglMakeLoft(const TopoDS_Shape& profiles, const TopoDS_Shape& guides, double tolerance = 1e-6);
    
    /**
     * @brief Should be called to add sections/profiles to the algorithm.
     *
     * @param profiles A shape that is either a wire or a compound consisting of multiple wires.
     */
    void addProfiles(const TopoDS_Shape& profiles);
    
    /** 
     * This function may be called to add guide curves to
     * the lofting algo. The shape should be either a wire
     * or a Compound consisting of multiple wires
     */
    void addGuides(const TopoDS_Shape& guides);
    
    /**
     * @brief Use the function to adjust, if you want to get a closed
     * solid (inclidung the side caps at the inner and outer profile)
     * or just the surface without caps.
     * 
     * @param enabled If true, a solid is built.
     */
    void setMakeSolid(bool enabled);
    
    /**
     * @brief setMakeSmooth switches, wether the resulting loft will be ruled
     * or smoothed. This switch only applies, if no guide curves are applied.
     * 
     * @param enabled Set to true, if smoothing should be enabled.
     */
    void setMakeSmooth(bool enabled);
    
    TopoDS_Shape& Shape();
    
    operator TopoDS_Shape& ();
    
    /**
     * @brief Performs the algorithm. Doesn't need to be called
     * manually.
     */
    void Perform();
    
private:
    void makeLoftWithGuides();
    void makeLoftWithoutGuides();
    
    double _myTolerance;
    std::vector<TopoDS_Wire> guides, profiles;
    bool _hasPerformed, _makeSolid, _makeSmooth;
    
    TopoDS_Shape _result;
};

#endif // CMAKELOFT_H
