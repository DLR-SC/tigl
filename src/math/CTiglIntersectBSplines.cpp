/*
* Copyright (C) 2019 German Aerospace Center (DLR/SC)
*
* Created: 2019-09-30 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglIntersectBSplines.h"
#include "CTiglBSplineAlgorithms.h"
#include "CTiglLineSegment.h"
#include "tiglcommonfunctions.h"

#include <math_MultipleVarFunctionWithGradient.hxx>
#include <math_BFGS.hxx>

#include <limits>
#include <list>
#include <algorithm>

#include <cassert>

#include <Standard_Version.hxx>

namespace
{
    template <typename T>
    T maxval(const T& v1, const T& v2)
    {
        return v1 > v2 ? v1 : v2;
    }

    template <typename T>
    T sqr(const T& v)
    {
        return v*v;
    }

    tigl::CTiglPoint minCoords(const tigl::CTiglPoint& p1, const tigl::CTiglPoint& p2)
    {
        tigl::CTiglPoint result = p1;
        result.x = p1.x < p2.x? p1.x : p2.x;
        result.y = p1.y < p2.y? p1.y : p2.y;
        result.z = p1.z < p2.z? p1.z : p2.z;
        return result;
        
    }
    
    tigl::CTiglPoint maxCoords(const tigl::CTiglPoint& p1, const tigl::CTiglPoint& p2)
    {
        tigl::CTiglPoint result = p1;
        result.x = p1.x > p2.x? p1.x : p2.x;
        result.y = p1.y > p2.y? p1.y : p2.y;
        result.z = p1.z > p2.z? p1.z : p2.z;
        return result;
        
    }

    class Intervall
    {
    public:
        Intervall(double mmin, double mmax)
            : min(mmin), max(mmax)
        {}
        double min;
        double max;
        
        bool operator==(const Intervall& other) const
        {
            const double EPS = 1e-15;
            return fabs(min - other.min) < EPS && fabs(max - other.max) < EPS;
        }
    };
    
    class BoundingBox
    {
    public:
        BoundingBox(const Handle(Geom_BSplineCurve) curve)
            : range(curve->FirstParameter(), curve->LastParameter())
        {
            low.x = low.y = low.z = std::numeric_limits<double>::max();
            high.x = high.y = high.z = -std::numeric_limits<double>::max();
            // compute min / max from control points
            for (Standard_Integer i = 1; i <= curve->NbPoles(); ++i) {
                gp_XYZ p = curve->Pole(i).XYZ();
                low = minCoords(low, p);
                high = maxCoords(high, p);
            }
        }
        
        bool Intersects(const BoundingBox& other, double eps) const
        {
            tigl::CTiglPoint min = maxCoords(low, other.low);
            tigl::CTiglPoint max = minCoords(high, other.high);
            return (min.x < max.x + eps) && (min.y < max.y + eps) && (min.z < max.z + eps);
        }
        
        BoundingBox& Merge(const BoundingBox& other)
        {
            assert(range.max == other.range.min);
            range.max = other.range.max;
            high = maxCoords(high, other.high);
            low = minCoords(low, other.low);

            return *this;
        }
        
        bool operator==(const BoundingBox& other) const
        {
            return range == other.range;
        }
        
        tigl::CTiglPoint low, high;
        Intervall range;
    };

    // Computes the total curvature of the curve
    // A curvature of 1 is equivalent to a straight line
    double curvature(const Handle(Geom_BSplineCurve)& curve)
    {
        double len = curve->Pole(1).Distance(curve->Pole(curve->NbPoles()));
        double total = 0.;
        for (Standard_Integer i = 1; i < curve->NbPoles(); ++i) {
            gp_Pnt p1 = curve->Pole(i);
            gp_Pnt p2 = curve->Pole(i+1);
            double dist = p1.Distance(p2);
            total += dist;
        }
        
        return total / len;
    }
    
    struct BoundingBoxPair
    {
        BoundingBoxPair (const BoundingBox& i1, const BoundingBox& i2)
            : b1(i1), b2(i2)
        {}
        BoundingBox b1;
        BoundingBox b2;
    };

    
    /// Computes possible ranges of intersections by a bracketing approach
    std::list<BoundingBoxPair> getRangesOfIntersection(const Handle(Geom_BSplineCurve) curve1, const Handle(Geom_BSplineCurve) curve2, double tolerance)
    {
        BoundingBox h1(curve1);
        BoundingBox h2(curve2);
        
        if (!h1.Intersects(h2, tolerance)) {
            return {};
        }
        
        double c1_curvature = curvature(curve1);
        double c2_curvature = curvature(curve2);
        double max_curvature = 1.0005;
        
        if (c1_curvature <= max_curvature && c2_curvature <= max_curvature) {
            // both curves are now almost linear. check approximate distance between line segments
            tigl::CTiglLineSegment l1(curve1->Pole(1).XYZ(), curve1->Pole(curve1->NbPoles()).XYZ());
            tigl::CTiglLineSegment l2(curve2->Pole(1).XYZ(), curve2->Pole(curve2->NbPoles()).XYZ());
            
            if (l1.distance(l2) < tolerance){
                return {BoundingBoxPair(h1, h2)};
            }
            else {
                return {};
            }
        }
        
        double curve1MidParm = 0.5*(curve1->FirstParameter() + curve1->LastParameter());
        double curve2MidParm = 0.5*(curve2->FirstParameter() + curve2->LastParameter());
        
        if (c1_curvature > max_curvature && c2_curvature > max_curvature) {
            Handle_Geom_BSplineCurve c11 = tigl::CTiglBSplineAlgorithms::trimCurve(curve1, curve1->FirstParameter(), curve1MidParm);
            Handle_Geom_BSplineCurve c12 = tigl::CTiglBSplineAlgorithms::trimCurve(curve1, curve1MidParm, curve1->LastParameter());
            
            Handle_Geom_BSplineCurve c21 = tigl::CTiglBSplineAlgorithms::trimCurve(curve2, curve2->FirstParameter(), curve2MidParm);
            Handle_Geom_BSplineCurve c22 = tigl::CTiglBSplineAlgorithms::trimCurve(curve2, curve2MidParm, curve2->LastParameter());
            
            auto result1 = getRangesOfIntersection(c11, c21, tolerance);
            auto result2 = getRangesOfIntersection(c11, c22, tolerance);
            auto result3 = getRangesOfIntersection(c12, c21, tolerance);
            auto result4 = getRangesOfIntersection(c12, c22, tolerance);
            
            // append all results
            result1.splice(std::begin(result1), result2);
            result1.splice(std::begin(result1), result3);
            result1.splice(std::begin(result1), result4);
            
            return result1;
        }
        else if (c1_curvature <= max_curvature && max_curvature < c2_curvature) {
            Handle_Geom_BSplineCurve c21 = tigl::CTiglBSplineAlgorithms::trimCurve(curve2, curve2->FirstParameter(), curve2MidParm);
            Handle_Geom_BSplineCurve c22 = tigl::CTiglBSplineAlgorithms::trimCurve(curve2, curve2MidParm, curve2->LastParameter());
            
            auto result1 = getRangesOfIntersection(curve1, c21, tolerance);
            auto result2 = getRangesOfIntersection(curve1, c22, tolerance);
            
            result1.splice(std::begin(result1), result2);
            return result1;
        }
        else if (c2_curvature <= max_curvature && max_curvature < c1_curvature) {
            Handle_Geom_BSplineCurve c11 = tigl::CTiglBSplineAlgorithms::trimCurve(curve1, curve1->FirstParameter(), curve1MidParm);
            Handle_Geom_BSplineCurve c12 = tigl::CTiglBSplineAlgorithms::trimCurve(curve1, curve1MidParm, curve1->LastParameter());
            
            auto result1 = getRangesOfIntersection(c11, curve2, tolerance);
            auto result2 = getRangesOfIntersection(c12, curve2, tolerance);
            
            result1.splice(std::begin(result1), result2);
            return result1;
        }
        
        return {};
    }

    class CurveCurveDistanceObjective : public math_MultipleVarFunctionWithGradient
    {
    public:
        CurveCurveDistanceObjective(const Handle(Geom_Curve)& c1, const Handle(Geom_Curve)& c2)
            : m_c1(c1), m_c2(c2)
        {}

        virtual Standard_Integer NbVariables()  const override
        {
            return 2;
        }

        Standard_Boolean Value (const math_Vector& X, Standard_Real& F) override
        {
            math_Vector G(1, 2);
            return Values(X, F, G);
        }

        Standard_Boolean Gradient (const math_Vector& X, math_Vector& G) override
        {
            Standard_Real F = 0.;
            return Values(X, F, G);
        }

        virtual  Standard_Boolean Values (const math_Vector& X, Standard_Real& F, math_Vector& G) override
        {
            double u = X.Value(1);
            double v = X.Value(2);

            gp_Pnt p1, p2;
            gp_Vec d1, d2;
            m_c1->D1(u, p1, d1);
            m_c2->D1(v, p2, d2);

            gp_Vec diff = p1.XYZ() - p2.XYZ();
            F = diff.SquareMagnitude();
            G(1) = 2. * diff.Dot(d1);
            G(2) = -2. * diff.Dot(d2);

            // we add a large penalty, if the variables go outside the valid range
            double u_penalty = sqr(maxval(0., m_c1->FirstParameter() - u)) + sqr(maxval(0., u - m_c1->LastParameter()));
            double v_penalty = sqr(maxval(0., m_c2->FirstParameter() - v)) + sqr(maxval(0., v - m_c2->LastParameter()));

            double d_u_penalty = -2. * maxval(0., m_c1->FirstParameter() - u) + 2.*maxval(0., u - m_c1->LastParameter());
            double d_v_penalty = -2. * maxval(0., m_c2->FirstParameter() - v) + 2.*maxval(0., v - m_c2->LastParameter());

            double fac = 1e7;
            F += fac*(u_penalty + v_penalty);
            G(1) += fac*d_u_penalty;
            G(2) += fac*d_v_penalty;

            return true;
        }

    private:
        const Handle(Geom_Curve) m_c1, m_c2;
    };
   

} // namespace

namespace tigl
{


std::vector<tigl::CurveIntersectionResult> IntersectBSplines(const Handle(Geom_BSplineCurve) curve1, const Handle(Geom_BSplineCurve) curve2, double tolerance)
{
    auto hulls = getRangesOfIntersection(curve1, curve2, tolerance);
    
    std::list<BoundingBox> curve1_ints, curve2_ints;
    for (const auto& hull : hulls) {
        curve1_ints.push_back(hull.b1);
        curve2_ints.push_back(hull.b2);
    }
    
    auto compare = [](const BoundingBox& b1, const BoundingBox& b2) {
        return b1.range.min < b2.range.min;
    };
    
    // sort ascending parameter
    curve1_ints.sort(compare);
    curve2_ints.sort(compare);

    // Remove duplicates
    curve1_ints.unique();
    curve2_ints.unique();
    
    auto is_adjacent = [](const BoundingBox& b1, const BoundingBox& b2) {
        const double EPS = 1e-15;
        return fabs(b1.range.max - b2.range.min) < EPS;
    };
    
    auto merge_boxes = [](const BoundingBox& b1, const BoundingBox& b2) {
        BoundingBox result(b1);
        return result.Merge(b2);
    };
    
    // merge neighboring intervals
    ReplaceAdjacentWithMerged(curve1_ints, is_adjacent, merge_boxes);
    ReplaceAdjacentWithMerged(curve2_ints, is_adjacent, merge_boxes);
    
    // combine intersection intervals
    std::vector<BoundingBoxPair> intersectionCandidates;
    for (const BoundingBox& b1 : curve1_ints) {
        for (const BoundingBox& b2 : curve2_ints) {
            if (b1.Intersects(b2, tolerance)) {
                intersectionCandidates.push_back(BoundingBoxPair(b1, b2));
            }
        }
    }

    CurveCurveDistanceObjective obj(curve1, curve2);

    std::vector<tigl::CurveIntersectionResult> results;

    for (const BoundingBoxPair& boxes : intersectionCandidates) {
        math_Vector guess(1, 2);
        guess(1) = 0.5*(boxes.b1.range.min + boxes.b1.range.max);
        guess(2) = 0.5*(boxes.b2.range.min + boxes.b2.range.max);

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,1)
        math_BFGS optimizer(obj.NbVariables(), 1e-12);
        optimizer.Perform(obj, guess);
#else
        math_BFGS optimizer(obj, guess, 1e-12);
#endif

        double u = Clamp(optimizer.Location().Value(1), curve1->FirstParameter(), curve1->LastParameter());
        double v = Clamp(optimizer.Location().Value(2), curve2->FirstParameter(), curve2->LastParameter());
        CurveIntersectionResult result;
        result.parmOnCurve1 = u;
        result.parmOnCurve2 = v;
        result.point = (CTiglPoint(curve1->Value(u).XYZ()) + CTiglPoint(curve2->Value(v).XYZ()))*0.5;

        results.push_back(result);
    }
    
    return results;
}



} // namespace tigl
