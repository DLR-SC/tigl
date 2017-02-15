/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-11-17 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CWireToCurve.h"
#include "CTiglLogging.h"

#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>

#include <GeomConvert.hxx>
#include <GCPnts_AbscissaPoint.hxx>

#include <BRep_Tool.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>

#include <ShapeFix_Wire.hxx>

#include <TColStd_Array1OfReal.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>

#include <vector>

#include <cassert>

namespace tigl
{

CWireToCurve::CWireToCurve(const TopoDS_Wire& w, bool parByLength, double t)
{
    _wire = w;
    _tolerance = t;
    _parByLength = parByLength;
}

/**
 * @brief ShiftCurveRange Changes the parameter range of the curve. The shape of the curve is not changed
 * @param curve
 * @param umin FirstParameter of the resulting curve
 * @param umax LastParameter of the resulting curve
 * @return 
 */
Handle(Geom_BSplineCurve) CWireToCurve::ShiftCurveRange(Handle(Geom_BSplineCurve) curve, double umin, double umax)
{
    double u1 = curve->FirstParameter();
    double u2 = curve->LastParameter();

    TColStd_Array1OfReal knots(1, curve->NbKnots());
    TColStd_Array1OfReal weights(1, curve->NbPoles());
    TColgp_Array1OfPnt cp(1, curve->NbPoles());
    TColStd_Array1OfInteger mults(1, curve->NbKnots());

    // get knotes and control points
    curve->Knots(knots);
    curve->Poles(cp);
    curve->Multiplicities(mults);
    curve->Weights(weights);

    for (int iknot = 1; iknot <= curve->NbKnots(); ++iknot) {
        double u = curve->Knot(iknot);
        double unew = (u - u1) / (u2 - u1) * (umax - umin) + umin;
        knots.SetValue(iknot, unew);
    }
    
    Handle(Geom_BSplineCurve) result = new Geom_BSplineCurve(cp, weights, knots, mults, curve->Degree(), curve->IsPeriodic());
    
    return result;
}

Handle(Geom_BSplineCurve) CWireToCurve::curve()
{
    if (_wire.IsNull()) {
        LOG(ERROR) << "Wire is null in CWireToCurve::curve";
        return NULL;
    }
    
    // fix order of wires
    ShapeFix_Wire wireFixer;
    wireFixer.Load(_wire);
    wireFixer.FixReorder();
    wireFixer.Perform();
    
    TopoDS_Wire theWire = wireFixer.Wire();

    std::vector<Handle(Geom_BSplineCurve)> curves;
    std::vector<double> lengths;
    double totalLen  = 0;
    int    maxDegree = 0;

    // get the bsplines of each edge, 
    // compute the lengths of each edge, 
    // determine maximum degree of the curves
    for (TopExp_Explorer exp(theWire, TopAbs_EDGE); exp.More(); exp.Next()) {
        TopoDS_Edge e = TopoDS::Edge(exp.Current());

        double u1, u2;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(e, u1, u2);
        curve = new Geom_TrimmedCurve(curve, u1, u2);
        if (e.Orientation() == TopAbs_REVERSED) {
            curve->Reverse();
        }

        // convert to bspline
        Handle(Geom_BSplineCurve) bspl = GeomConvert::CurveToBSplineCurve(curve);
        curves.push_back(bspl);

        if (_parByLength) {
            // find out length of current curve
            Standard_Real umin = bspl->FirstParameter();
            Standard_Real umax = bspl->LastParameter();
            GeomAdaptor_Curve adaptorCurve(bspl, umin, umax);
            double len = GCPnts_AbscissaPoint::Length(adaptorCurve, umin, umax);
            lengths.push_back(len);
            totalLen += len;
        }

        // find out maximum degree
        if (bspl->Degree() > maxDegree) {
            maxDegree = bspl->Degree();
        }
    }
    if (curves.size() == 0) {
        LOG(ERROR) << "Wire is empty in CWireToCurve::curve";
        return NULL;
    }
    else if (curves.size() == 1) {
        return curves[0];
    }

    // check connectivities
    for (unsigned int icurve = 1; icurve < curves.size(); ++icurve) {
        Handle(Geom_BSplineCurve) c1 = curves[icurve-1];
        Handle(Geom_BSplineCurve) c2 = curves[icurve];

        gp_Pnt p1 = c1->Pole(c1->NbPoles());
        gp_Pnt p2 = c2->Pole(1);

        if (p1.Distance(p2) > _tolerance) {
            // error
            LOG(ERROR) << "Curves not connected within tolerance in CWireToCurve::curve";
            return NULL;
        }
    }

    // elevate degree of all curves to maxDegree
    for (unsigned int icurve = 0; icurve < curves.size(); ++icurve) {
        Handle(Geom_BSplineCurve) curve = curves[icurve];
        curve->IncreaseDegree(maxDegree);
    }

#ifdef DEBUG
    // check that each curve is at maxDegree
    for (unsigned int icurve = 0; icurve < curves.size(); ++icurve) {
        Handle(Geom_BSplineCurve) curve = curves[icurve];
        assert(curve->Degree() == maxDegree);
    }
#endif

    // shift knots of curves
    double startPar = 0;
    for (unsigned int icurve = 0; icurve < curves.size(); ++icurve) {
        Handle(Geom_BSplineCurve) curve = curves[icurve];
        double stopPar = startPar;
        if (_parByLength) {
            stopPar += lengths[icurve]/totalLen;
        }
        else {
            stopPar += curve->LastParameter() - curve->FirstParameter();
        }
        curve = ShiftCurveRange(curve, startPar, stopPar);
        curves[icurve] = curve;

        startPar = stopPar;
    }

    // count number of knots and control points for the final b-spline
    int nbknots = 1;
    int nbcp    = 1;
    for (unsigned int icurve = 0; icurve < curves.size(); ++icurve) {
        Handle(Geom_BSplineCurve) curve = curves[icurve];
        nbknots += curve->NbKnots() - 1;
        nbcp    += curve->NbPoles() - 1;
    }

    // allocate arrays
    TColgp_Array1OfPnt      cpoints(1, nbcp);
    TColStd_Array1OfReal    weights(1, nbcp);
    TColStd_Array1OfReal    knots(1, nbknots);
    TColStd_Array1OfInteger mults(1, nbknots);

    // concatenate everything
    int iknotT = 1, imultT = 1, icpT = 1, iweightT = 1;
    for (unsigned int icurve = 0; icurve < curves.size(); ++icurve) {
        Handle(Geom_BSplineCurve) curve = curves[icurve];

        // special handling of the first knot, control point
        knots.SetValue(iknotT++, curve->Knot(1));
        if (icurve == 0) {
            // we just copy the data of the very first point/knot
            mults.SetValue(imultT++, curve->Multiplicity(1));
            cpoints.SetValue(icpT++, curve->Pole(1));
            weights.SetValue(iweightT++, curve->Weight(1));
        }
        else {
            // set multiplicity to maxDegree to allow c0 concatenation
            mults.SetValue(imultT++, maxDegree);

            // compute midpoint between endpoint of previous 
            // curve and startpoint of current curve
            Handle(Geom_BSplineCurve) lastCurve = curves[icurve-1];
            gp_Pnt endPoint   = lastCurve->Pole(lastCurve->NbPoles());
            gp_Pnt startPoint = curve->Pole(1);
            gp_Pnt midPoint = (endPoint.XYZ() + startPoint.XYZ())/2.;
            cpoints.SetValue(icpT++, midPoint);
            
            // we use the average weight of previous curve and current curve
            // This is probably wrong and could change the shape of the curve
            // Instead, one could scale all weights of the curve to match the weight of 
            // the previous curve
            weights.SetValue(iweightT++, (lastCurve->Weight(lastCurve->NbPoles()) + curve->Weight(1))/2.);
        }

        // just copy control points, weights, knots and multiplicites
        for (int iknot = 2; iknot < curve->NbKnots(); ++iknot) {
            knots.SetValue(iknotT++, curve->Knot(iknot));
            mults.SetValue(imultT++, curve->Multiplicity(iknot));
        }
        for (int icp = 2; icp < curve->NbPoles(); ++icp) {
            cpoints.SetValue(icpT++, curve->Pole(icp));
            weights.SetValue(iweightT++, curve->Weight(icp));
        }
        
    }

    // special handling of the last point and knot
    Handle(Geom_BSplineCurve) lastCurve = curves[curves.size()-1];
    knots.SetValue(iknotT, lastCurve->Knot(lastCurve->NbKnots()));
    mults.SetValue(imultT,  lastCurve->Multiplicity(lastCurve->NbKnots()));
    cpoints.SetValue(icpT, lastCurve->Pole(lastCurve->NbPoles()));
    weights.SetValue(iweightT, lastCurve->Weight(lastCurve->NbPoles()));

#ifdef DEBUG
    // check that we have the correct number of knots, control points etc...
    int nkn = 0;
    for (int ik = knots.Lower(); ik <= knots.Upper(); ++ik) {
        nkn += mults.Value(ik);
    }

    // check validity of bspline
    assert (cpoints.Length() + maxDegree + 1 == nkn);
#endif

    // build the resulting B-Spline
    Handle(Geom_BSplineCurve) result = new Geom_BSplineCurve(cpoints, weights, knots, mults, maxDegree, false);
    return result;
}

CWireToCurve::operator Handle(Geom_BSplineCurve)()
{
    return curve();
}

} // namespace tigl

