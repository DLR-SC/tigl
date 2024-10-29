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
#include "CTiglError.h"

#include "CTiglBSplineAlgorithms.h"
#include <Geom_BSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>

#include <BRep_Tool.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>

#include <ShapeFix_Wire.hxx>

#include <vector>


namespace tigl
{

CWireToCurve::CWireToCurve(const TopoDS_Wire& w, bool parByLength, double t)
{
    _wire = w;
    _tolerance = t;
    _parByLength = parByLength;
}


Handle(Geom_BSplineCurve) CWireToCurve::curve()
{
    if (_wire.IsNull()) {
        LOG(ERROR) << "Wire is null in CWireToCurve::curve";
        return nullptr;
    }
    
    // fix order of wires
    ShapeFix_Wire wireFixer;
    wireFixer.Load(_wire);
    wireFixer.FixReorder();
    wireFixer.Perform();
    
    TopoDS_Wire theWire = wireFixer.Wire();

    std::vector<Handle(Geom_BSplineCurve)> curves;


    // get the bsplines of each edge, 
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
    }

    return CTiglBSplineAlgorithms::concatCurves(curves, _parByLength, _tolerance);

}

CWireToCurve::operator Handle(Geom_BSplineCurve)()
{
    return curve();
}

} // namespace tigl

