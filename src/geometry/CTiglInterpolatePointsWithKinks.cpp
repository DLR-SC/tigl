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

#include "CTiglInterpolatePointsWithKinks.h"

#include "CTiglBSplineAlgorithms.h"
#include "CTiglPointsToBSplineInterpolation.h"
#include "tiglmathfunctions.h"

#include <iterator>

namespace
{
    template <typename ArrayLike>
    inline bool in(unsigned int key, const ArrayLike& array)
    {
        return std::find(std::begin(array), std::end(array), key) != array.end();
    }

    template <>
    inline bool in(unsigned int key, const tigl::ParamMap& map)
    {
        return map.find(key) != map.end();
    }

    template <typename ArrayLike>
    ArrayLike slice(const ArrayLike& array, size_t imin, size_t imax)
    {
        size_t len = imax - imin + 1;
        ArrayLike result(len);
        for (size_t i = 0; i < len; ++i) {
            result[i] = array[i + imin];
        }

        return result;
    }

    template <>
    Handle(TColgp_HArray1OfPnt) slice(const Handle(TColgp_HArray1OfPnt) & pnts, size_t imin, size_t imax)
    {
        int len = static_cast<int>(imax - imin + 1);
        Handle(TColgp_HArray1OfPnt) result = new TColgp_HArray1OfPnt(1, len);
        for (int i = 0; i < len; ++i) {
            result->SetValue(i+1, pnts->Value(i + static_cast<int>(imin)));
        }

        return result;
    }

    template <typename ArrayLike>
    ArrayLike stack(const ArrayLike& arr1, const ArrayLike& arr2)
    {
        auto result = arr1;
        for (const auto& v : arr2) {
            result.push_back(v);
        }

        return result;
    }

    template <>
    Handle(TColgp_HArray1OfPnt) stack(const Handle(TColgp_HArray1OfPnt) & pnts1, const Handle(TColgp_HArray1OfPnt) & pnts2)
    {
        const int len1 = pnts1->Length();
        const int len2 = pnts2->Length();


        Handle(TColgp_HArray1OfPnt) result = new TColgp_HArray1OfPnt(1, len1 + len2);
        for (int i = 0; i < len1; ++i) {
            result->SetValue(i+1, pnts1->Value(i+1));
        }

        for (int i = 0; i < len2; ++i) {
            result->SetValue(len1 + i + 1, pnts2->Value(i+1));
        }

        return result;
    }

    std::vector<double> computeParams(const Handle(TColgp_HArray1OfPnt)& pnts, tigl::ParamMap& params, double alpha)
    {
        auto initial_params = tigl::CTiglBSplineAlgorithms::computeParamsBSplineCurve(pnts, alpha);

        unsigned int n_pnts = static_cast<unsigned int>(pnts->Length());

        if (!in(0, params)) {
            params[0] = 0.;
        }
        if (!in(n_pnts - 1, params)) {
            params[n_pnts - 1] = 1.;
        }

        std::vector<double> kink_params_new;
        std::vector<double> kink_params_old;

        for (const auto& p : params) {
            kink_params_new.push_back(p.second);
            kink_params_old.push_back(initial_params[p.first]);
        }

        // Perform linear interpolation to get the new parameters
        std::vector<double> new_params;
        std::transform(std::begin(initial_params), std::end(initial_params),
                       std::back_inserter(new_params), [&](double p) {
           return  tigl::Interpolate(kink_params_old, kink_params_new, p);
        });

        return new_params;
    }

} // namespace

namespace tigl
{

CTiglInterpolatePointsWithKinks::CTiglInterpolatePointsWithKinks(const Handle(TColgp_HArray1OfPnt) & points,
                                                                const std::vector<unsigned int>& kinkIndices,
                                                                const ParamMap& parameters,
                                                                double alpha,
                                                                unsigned int maxDegree)
    : m_pnts(points)
    , m_kinks(kinkIndices)
    , m_params(parameters)
    , m_alpha(alpha)
    , m_maxDegree(maxDegree)
    , m_result(*this, &CTiglInterpolatePointsWithKinks::ComputeResult)
{
}

Handle(Geom_BSplineCurve) CTiglInterpolatePointsWithKinks::Curve() const
{
    return m_result->curve;
}

std::vector<double> CTiglInterpolatePointsWithKinks::Parameters() const
{
    return m_result->parameters;
}

void CTiglInterpolatePointsWithKinks::ComputeResult(Result& result) const
{
    auto params = m_params;

    // assuming iterating over parameters is sorted in keys
    auto new_params = computeParams(m_pnts, params, m_alpha);

    unsigned int n_pnts = static_cast<unsigned int>(m_pnts->Length());

    // make a sorted copy of the kinks array
    auto kinks = m_kinks;
    std::sort(std::begin(kinks), std::end(kinks));

    bool make_continous = false;
    if (m_pnts->Value(m_pnts->Lower()).IsEqual(m_pnts->Value(m_pnts->Upper()), 1e-6)) {
        if (!in(0, kinks) && ! in(n_pnts - 1, kinks)) {
            make_continous = true;
        }
    }

    // make sure, that first and last points are in kink list
    if (!make_continous) {
        if (!in(0, kinks)) {
            kinks.insert(kinks.begin(), 0);
        }
        if (!in(n_pnts - 1, kinks)) {
            kinks.push_back(n_pnts - 1);
        }
    }

    std::vector<Handle(Geom_BSplineCurve)> curve_segments;

    for (size_t i=1; i < kinks.size(); ++i) {
        auto start_idx = kinks[i-1];
        auto end_idx   = kinks[i];

        auto current_pnts = slice(m_pnts, start_idx + 1, end_idx + 1);
        auto the_pars = slice(new_params, start_idx, end_idx);

        auto segment_curve = CTiglPointsToBSplineInterpolation(current_pnts, the_pars, m_maxDegree, false).Curve();
        curve_segments.push_back(segment_curve);
    }

    // create a continous spline from the last to the first kink
    if (make_continous && kinks.size() > 0) {
        double offset = new_params.front() - new_params.back();

        auto last_kink_idx = kinks.back();
        auto first_kink_idx = kinks.front();

        auto pnts = stack(slice(m_pnts, last_kink_idx + 1, static_cast<size_t>(m_pnts->Length())),
                          slice(m_pnts, 2, first_kink_idx + 1));

        auto left_parms = slice(new_params, last_kink_idx,  new_params.size()-1);
        std::transform(std::begin(left_parms), std::end(left_parms), std::begin(left_parms), [offset](double p) {
            return p + offset;
        });
        auto right_parms = slice(new_params, 1, first_kink_idx);
        auto the_pars = stack(std::move(left_parms), std::move(right_parms));


        auto lower_curve = CTiglPointsToBSplineInterpolation(pnts, the_pars, m_maxDegree, false).Curve();

        double param_to_split = new_params.front();
        auto last = CTiglBSplineAlgorithms::trimCurve(lower_curve, lower_curve->FirstParameter(), param_to_split);
        auto first = CTiglBSplineAlgorithms::trimCurve(lower_curve, param_to_split, lower_curve->LastParameter());
        // shift parametrization of the last curve to the end of the parameter range
        CTiglBSplineAlgorithms::reparametrizeBSpline(*last, the_pars.front() - offset, param_to_split - offset);

        curve_segments.insert(curve_segments.begin(), first);
        curve_segments.push_back(last);
    }
    else if (make_continous && kinks.empty()) {
        auto curve = CTiglPointsToBSplineInterpolation(m_pnts, new_params, m_maxDegree, true).Curve();
        curve_segments.push_back(curve);
    }

    result.curve = CTiglBSplineAlgorithms::concatCurves(curve_segments, false);
    result.parameters = new_params;
}


} // namespace tigl
