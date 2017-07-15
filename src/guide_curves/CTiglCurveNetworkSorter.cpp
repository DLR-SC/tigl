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

#include "CTiglCurveNetworkSorter.h"

#include "CTiglError.h"

#include <utility>
#include <cassert>

namespace
{

    // returns the column index if the maximum of i-th row
    size_t maxRowIndex(const math_Matrix& m, size_t irow)
    {
        double max = DBL_MIN;
        size_t jmax = 0;

        for (Standard_Integer jcol = 0; jcol < m.ColNumber(); ++jcol) {
            if (m(static_cast<Standard_Integer>(irow), jcol) > max) {
                max = m(static_cast<Standard_Integer>(irow), jcol);
                jmax = static_cast<size_t>(jcol);
            }
        }

        return jmax;
    }

    // returns the row index if the maximum of i-th col
    size_t maxColIndex(const math_Matrix& m, size_t jcol)
    {
        double max = DBL_MIN;
        size_t imax = 0;

        for (Standard_Integer irow = 0; irow < m.RowNumber(); ++irow) {
            if (m(irow, static_cast<Standard_Integer>(jcol)) > max) {
                max = m(irow, static_cast<Standard_Integer>(jcol));
                imax = static_cast<size_t>(irow);
            }
        }

        return imax;
    }

    // returns the column index if the minimum of i-th row
    size_t minRowIndex(const math_Matrix& m, size_t irow)
    {
        double min = DBL_MAX;
        size_t jmin = 0;

        for (Standard_Integer jcol = 0; jcol < m.ColNumber(); ++jcol) {
            if (m(static_cast<Standard_Integer>(irow), jcol) < min) {
                min = m(static_cast<Standard_Integer>(irow), jcol);
                jmin = static_cast<size_t>(jcol);
            }
        }

        return jmin;
    }

    // returns the column index if the minimum of i-th row
    size_t minColIndex(const math_Matrix& m, size_t jcol)
    {
        double min = DBL_MAX;
        size_t imin = 0;

        for (Standard_Integer irow = 0; irow < m.RowNumber(); ++irow) {
            if (m(irow, static_cast<Standard_Integer>(jcol)) < min) {
                min = m(irow, static_cast<Standard_Integer>(jcol));
                imin = static_cast<size_t>(irow);
            }
        }

        return imin;
    }

}

namespace tigl
{

CTiglCurveNetworkSorter::CTiglCurveNetworkSorter(const std::vector<Handle (Geom_Curve)> &profiles,
                                                 const std::vector<Handle (Geom_Curve)> &guides,
                                                 const math_Matrix &parmsIntersProfiles,
                                                 const math_Matrix &parmsIntersGuides)
    : m_profiles(profiles)
    , m_guides(guides)
    , m_parmsIntersProfiles(parmsIntersProfiles)
    , m_parmsIntersGuides(parmsIntersGuides)
    , m_hasPerformed(false)

{
    // check consistency of input data
    size_t n_profiles = profiles.size();
    size_t n_guides = guides.size();

    if (n_profiles != m_parmsIntersProfiles.RowNumber()) {
        throw CTiglError("Invalid row size of parmsIntersProfiles matrix.");
    }

    if (n_profiles != m_parmsIntersGuides.RowNumber()) {
        throw CTiglError("Invalid row size of parmsIntersGuides matrix.");
    }

    if (n_guides != m_parmsIntersProfiles.ColNumber()) {
        throw CTiglError("Invalid col size of parmsIntersProfiles matrix.");
    }

    if (n_guides != m_parmsIntersGuides.ColNumber()) {
        throw CTiglError("Invalid col size of parmsIntersGuides matrix.");
    }

    assert(m_parmsIntersGuides.UpperRow() == n_profiles - 1);
    assert(m_parmsIntersProfiles.UpperRow() == n_profiles - 1);
    assert(m_parmsIntersGuides.UpperCol() == n_guides - 1);
    assert(m_parmsIntersProfiles.UpperCol() == n_guides - 1);

    // create helper vectors with indices
    for (int i = 0; i < n_profiles; ++i) {
        std::stringstream str;
        str << i;
        m_profIdx.push_back(str.str());
    }

    for (int i = 0; i < n_guides; ++i) {
        std::stringstream str;
        str << i;
        m_guidIdx.push_back(str.str());
    }
}

void CTiglCurveNetworkSorter::swapProfiles(size_t idx1, size_t idx2)
{
    if (idx1 == idx2) {
        return;
    }

    std::iter_swap(m_profiles.begin() + idx1, m_profiles.begin() + idx2);
    std::iter_swap(m_profIdx.begin() + idx1, m_profIdx.begin() + idx2);
    m_parmsIntersGuides.SwapRow(static_cast<Standard_Integer>(idx1), static_cast<Standard_Integer>(idx2));
    m_parmsIntersProfiles.SwapRow(static_cast<Standard_Integer>(idx1), static_cast<Standard_Integer>(idx2));
}

void CTiglCurveNetworkSorter::swapGuides(size_t idx1, size_t idx2)
{
    if (idx1 == idx2) {
        return;
    }

    std::iter_swap(m_guides.begin() + idx1, m_guides.begin() + idx2);
    std::iter_swap(m_guidIdx.begin() + idx1, m_guidIdx.begin() + idx2);
    m_parmsIntersGuides.SwapCol(static_cast<Standard_Integer>(idx1), static_cast<Standard_Integer>(idx2));
    m_parmsIntersProfiles.SwapCol(static_cast<Standard_Integer>(idx1), static_cast<Standard_Integer>(idx2));
}

void CTiglCurveNetworkSorter::GetStartCurveIndices(size_t &prof_idx, size_t &guid_idx, bool &guideMustBeReversed) const
{
    // find curves, that begin at the same point (have the smallest parameter at their intersection)
    for (size_t irow = 0; irow < NProfiles(); ++irow) {
        size_t jmin = minRowIndex(m_parmsIntersProfiles, irow);
        size_t imin = minColIndex(m_parmsIntersGuides, jmin);

        if (imin == irow) {
            // we found the start curves
            prof_idx = imin;
            guid_idx = jmin;
            guideMustBeReversed = false;
            return;
        }
    }

    // there are situtation (a loop) when the previous situation does not exist
    // find curves were the start of a profile hits the end of a guide
    for (size_t irow = 0; irow < NProfiles(); ++irow) {
        size_t jmin = minRowIndex(m_parmsIntersProfiles, irow);
        size_t imax = maxColIndex(m_parmsIntersGuides, jmin);

        if (imax == irow) {
            // we found the start curves
            prof_idx = imax;
            guid_idx = jmin;
            guideMustBeReversed = true;
            return;
        }
    }

    // we have not found the starting curve. The network seems invalid
    throw CTiglError("Cannot find starting curves of curve network.");
}

void CTiglCurveNetworkSorter::Perform()
{
    if (m_hasPerformed) {
        return;
    }

    size_t prof_start = 0, guide_start = 0;
    Standard_Integer nGuid = static_cast<Standard_Integer>(NGuides());
    Standard_Integer nProf = static_cast<Standard_Integer>(NProfiles());

    bool guideMustBeReversed = false;
    GetStartCurveIndices(prof_start, guide_start, guideMustBeReversed);

    // put start curves first in array
    swapProfiles(0, prof_start);
    swapGuides(0, guide_start);

    if (guideMustBeReversed) {
        reverseGuide(0);
    }

    // perform a bubble sort for the guides,
    // such that the guides intersection of the first profile are ascending
    for (int n = nGuid; n > 1; n = n - 1) {
        for (int j = 1; j < n - 1; ++j) {
            if (m_parmsIntersProfiles(0, j) > m_parmsIntersProfiles(0, j + 1)) {
                swapGuides(j, j + 1);
            }
        }
    }

    // perform a bubble sort of the profiles,
    // such that the profiles are in ascending order of the first guide
    for (int n = nProf; n > 1; n = n - 1) {
        for (int i = 1; i < n - 1; ++i) {
            if (m_parmsIntersGuides(i, 0) > m_parmsIntersGuides(i + 1, 0)) {
                swapProfiles(i, i + 1);
            }
        }
    }

    // reverse profiles, if necessary
    for (Standard_Integer iProf = 1; iProf < nProf; ++iProf) {
        if (m_parmsIntersProfiles(iProf, 0) > m_parmsIntersProfiles(iProf, nGuid - 1)) {
            reverseProfile(iProf);
        }
    }

    // reverse guide, if necessary
    for (Standard_Integer iGuid = 1; iGuid < nGuid; ++iGuid) {
        if (m_parmsIntersGuides(0, iGuid) > m_parmsIntersGuides(nProf - 1, iGuid)) {
            reverseGuide(iGuid);
        }
    }

    m_hasPerformed = true;
}

size_t CTiglCurveNetworkSorter::NProfiles() const
{
    return m_profiles.size();
}

size_t CTiglCurveNetworkSorter::NGuides() const
{
    return m_guides.size();
}

const std::vector<std::string> &CTiglCurveNetworkSorter::ProfileIndices() const
{
    return m_profIdx;
}

const std::vector<std::string> &CTiglCurveNetworkSorter::GuideIndices() const
{
    return m_guidIdx;
}

void CTiglCurveNetworkSorter::reverseProfile(size_t profileIdx)
{
    Standard_Integer pIdx = static_cast<Standard_Integer>(profileIdx);

    Handle(Geom_Curve) profile = m_profiles[profileIdx];
    Standard_Real lastParm = !profile.IsNull() ?
                profile->LastParameter() :
                m_parmsIntersProfiles(pIdx, static_cast<Standard_Integer>(maxRowIndex(m_parmsIntersProfiles, pIdx)));

    Standard_Real firstParm = !profile.IsNull() ?
                profile->FirstParameter() :
                m_parmsIntersProfiles(pIdx, static_cast<Standard_Integer>(minRowIndex(m_parmsIntersProfiles, pIdx)));


    // compute new parameters
    for (int icol = 0; icol < NGuides(); ++icol) {
        m_parmsIntersProfiles(pIdx, icol) = -m_parmsIntersProfiles(pIdx, icol) + firstParm + lastParm;
    }

    if (!profile.IsNull()) {
        profile->Reverse();
    }

    m_profIdx[profileIdx] = "-" + m_profIdx[profileIdx];
}

void CTiglCurveNetworkSorter::reverseGuide(size_t guideIdx)
{

    Standard_Integer gIdx = static_cast<Standard_Integer>(guideIdx);

    Handle(Geom_Curve) guide = m_guides[guideIdx];
    Standard_Real lastParm = !guide.IsNull() ?
                guide->LastParameter() :
                m_parmsIntersGuides(static_cast<Standard_Integer>(maxColIndex(m_parmsIntersGuides, gIdx)), gIdx);

    Standard_Real firstParm = !guide.IsNull() ?
                guide->FirstParameter() :
                m_parmsIntersGuides(static_cast<Standard_Integer>(minColIndex(m_parmsIntersGuides, gIdx)), gIdx);

    // compute new parameter
    for (int irow = 0; irow < NProfiles(); ++irow) {
        m_parmsIntersGuides(irow, gIdx) = -m_parmsIntersGuides(irow, gIdx) + firstParm + lastParm;
    }

    if (!guide.IsNull()) {
        guide->Reverse();
    }

    m_guidIdx[guideIdx] = "-" + m_guidIdx[guideIdx];
}


} // namespace tigl
