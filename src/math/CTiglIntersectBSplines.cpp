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
#include "tiglcommonfunctions.h"

#include <math_MultipleVarFunctionWithGradient.hxx>
#include <math_FRPR.hxx>

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
            // Bounding boxes do not intersect. No intersection possible
            return {};
        }
        
        double c1_curvature = curvature(curve1);
        double c2_curvature = curvature(curve2);
        double max_curvature = 1.0005;
        
        // If both curves are linear enough, we can stop refining
        if (c1_curvature <= max_curvature && c2_curvature <= max_curvature) {
            return {BoundingBoxPair(h1, h2)};
        }
        
        double curve1MidParm = 0.5*(curve1->FirstParameter() + curve1->LastParameter());
        double curve2MidParm = 0.5*(curve2->FirstParameter() + curve2->LastParameter());
        
        if (c1_curvature > max_curvature && c2_curvature > max_curvature) {
            // Refine both curves by splitting them in the parametric center
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
            // Refine only curve 2
            Handle_Geom_BSplineCurve c21 = tigl::CTiglBSplineAlgorithms::trimCurve(curve2, curve2->FirstParameter(), curve2MidParm);
            Handle_Geom_BSplineCurve c22 = tigl::CTiglBSplineAlgorithms::trimCurve(curve2, curve2MidParm, curve2->LastParameter());
            
            auto result1 = getRangesOfIntersection(curve1, c21, tolerance);
            auto result2 = getRangesOfIntersection(curve1, c22, tolerance);
            
            result1.splice(std::begin(result1), result2);
            return result1;
        }
        else if (c2_curvature <= max_curvature && max_curvature < c1_curvature) {
            // Refine only curve 1
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

        // Reparametrization function R -> [0,1]
        static double activate(double z)
        {
            return 0.5 * (sin(z) + 1.);
        }

        // Derivative of reparametrization function
        static double d_activate(double z)
        {
            return 0.5 * cos(z);
        }

        double getUParam(double x0) const
        {
            double umin = m_c1->FirstParameter();
            double umax = m_c1->LastParameter();

            return activate(x0)*(umax - umin) + umin;
        }

        double getVParam(double x1) const
        {
            double vmin = m_c2->FirstParameter();
            double vmax = m_c2->LastParameter();

            return activate(x1)*(vmax - vmin) + vmin;
        }

        double d_getUParam(double x0) const
        {
            double umin = m_c1->FirstParameter();
            double umax = m_c1->LastParameter();

            return d_activate(x0)*(umax - umin);
        }

        double d_getVParam(double x1) const
        {
            double vmin = m_c2->FirstParameter();
            double vmax = m_c2->LastParameter();

            return d_activate(x1)*(vmax - vmin);
        }

        virtual  Standard_Boolean Values (const math_Vector& X, Standard_Real& F, math_Vector& G) override
        {

            // We use a reparametrization trick to ensure that u is in [umin, umax] and v in [vmin, vmax]
            double u = getUParam(X.Value(1));
            double v = getVParam(X.Value(2));

            gp_Pnt p1, p2;
            gp_Vec d1, d2;
            m_c1->D1(u, p1, d1);
            m_c2->D1(v, p2, d2);

            gp_Vec diff = p1.XYZ() - p2.XYZ();
            F = diff.SquareMagnitude();
            G(1) = 2. * diff.Dot(d1)* (m_c1->LastParameter()-m_c1->FirstParameter()) * d_getUParam(X.Value(1));
            G(2) = -2. * diff.Dot(d2) * (m_c2->LastParameter()-m_c2->FirstParameter()) * d_getUParam(X.Value(2));

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
    auto vessels = getRangesOfIntersection(curve1, curve2, tolerance);
    
    std::list<BoundingBox> curve1_ints, curve2_ints;
    for (const auto& vessel : vessels) {
        curve1_ints.push_back(vessel.b1);
        curve2_ints.push_back(vessel.b2);
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


    std::vector<tigl::CurveIntersectionResult> results;

    for (const BoundingBoxPair& boxes : intersectionCandidates) {

        auto c1 = CTiglBSplineAlgorithms::trimCurve(curve1, boxes.b1.range.min,boxes.b1.range.max);
        auto c2 = CTiglBSplineAlgorithms::trimCurve(curve2, boxes.b2.range.min,boxes.b2.range.max);

        CurveCurveDistanceObjective obj(c1, c2);

        // The objective is designed such that x=[0, 0] is in the middle of the parameter space of both curves
        math_Vector guess(1, 2);
        guess(1) = 0.;
        guess(2) = 0.;


        math_FRPR optimizer(obj, 1e-10, 200);
        optimizer.Perform(obj, guess);

        if (!optimizer.IsDone()) {
            LOG(ERROR) << "Unable to compute exact intersection in `IntersectBSplines` due to failure in minimization. Please file a report";
            continue;
        }

        // convert parameter space of optimized into u/v curve parameters
        double u = obj.getUParam(optimizer.Location().Value(1));
        double v = obj.getVParam(optimizer.Location().Value(2));

        double distance = c1->Value(u).Distance(c2->Value(v));
        if (distance < std::max(1e-10, tolerance)) {
            CurveIntersectionResult result;
            result.parmOnCurve1 = u;
            result.parmOnCurve2 = v;
            result.point = (CTiglPoint(curve1->Value(u).XYZ()) + CTiglPoint(curve2->Value(v).XYZ()))*0.5;
            results.push_back(result);
        }
    }
    
    return results;
}



} // namespace tigl
