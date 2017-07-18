/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017 Martin Siggel <Martin.Siggel@dlr.de>
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

#ifndef CTIGLCURVENETWORKSORTER_H
#define CTIGLCURVENETWORKSORTER_H

#include <Geom_Curve.hxx>
#include <math_Matrix.hxx>

#include <vector>

namespace tigl
{

class CTiglCurveNetworkSorter
{
public:
    /**
     * @brief This algorithm sorts and corrects a curve network
     *
     * The input data must fulfill the following condition:
     *
     * profiles[i](parmsIntersProfiles(i, j)) == guides[j](parmsIntersGuides(i, j))
     *
     * This means, that i-th profile and j-th guide intersect each other at the profile parameter
     * parmsIntersProfiles(i, j) and the guide parameter parmsIntersGuides(i, j)
     *
     * @param profiles Profile curves
     * @param guides   Guide curves
     * @param parmsIntersProfiles Intersection parameters on the profiles with the guides
     * @param parmsIntersGuides   Intersection parameters on the guides with the profiles
     */
    CTiglCurveNetworkSorter(const std::vector<Handle(Geom_Curve)>& profiles,
                            const std::vector<Handle(Geom_Curve)>& guides,
                            const math_Matrix& ProfileIntersectionParms,
                            const math_Matrix& GuideIntersectionParms);


    /**
     * @brief Helper function to determine the first profile and guide
     * of the curve network
     *
     * The function returns the index of the first profile and the index
     * of the first guide.
     *
     * This is the only part in the algorithm, were profiles and guides are handled
     * assymetrically. It tries to find a corner point of the curve network, where
     * a profile starts and a guide starts as well. There might be cases (e.g. a circular
     * connection of the outer curves), when such a corner point does not exist.
     * In this case, the algorithm tries to find a corner, where a profiles starts and
     * a guide ends. In this case, the variable guideMustBeReversed is true.
     *
     * @param prof_idx Computed index of the first profile.
     * @param guid_idx Computed index of the first guide.
     * @param guideMustBeReversed If true, the first guide curve must be reversed to get a
     *        correct curve network.
     */
    void GetStartCurveIndices(size_t& prof_idx, size_t& guid_idx, bool& guideMustBeReversed) const;

    /**
     * @brief Performs the reordering of the curve network for the use in gordon surfaces
     */
    void Perform();

    /**
     * @brief Returns the number of profile curves of the network
     */
    size_t NProfiles() const;

    /**
     * @brief Returns the number of guide curves of the network
     */
    size_t NGuides() const;

    /**
     * @brief Returns the parameters on the profile curves, where the guides are intersectings.
     *
     * The returned matrix contains the intersection parameter on the i-th profile, where
     * the j-th guide intersects.
     */
    math_Matrix const & ProfileIntersectionParms() const
    {
        return m_parmsIntersProfiles;
    }

    /**
     * @brief Returns the parameters on the guide curves, where the profiles are intersectings.
     *
     * The returned matrix contains the intersection parameter on the j-th guide, where
     * the i-th profile intersects.
     */
    math_Matrix const & GuideIntersectionParms() const
    {
        return m_parmsIntersGuides;
    }

    // The next functions are just for testing the algorithm and
    // are not required elsewhere.
    std::vector<std::string> const & ProfileIndices() const;
    std::vector<std::string> const & GuideIndices() const;

private:
    void swapProfiles(size_t idx1, size_t idx2);
    void swapGuides(size_t idx1, size_t idx2);

    void reverseProfile(size_t profileIdx);
    void reverseGuide(size_t guideIdx);

    std::vector<Handle(Geom_Curve)> m_profiles;
    std::vector<Handle(Geom_Curve)> m_guides;
    math_Matrix m_parmsIntersProfiles;
    math_Matrix m_parmsIntersGuides;

    // helper vectors for testing
    std::vector<std::string> m_profIdx;
    std::vector<std::string> m_guidIdx;

    bool m_hasPerformed;
};

} // namespace tigl

#endif // CTIGLCURVENETWORKSORTER_H
