/* 
* Copyright (C) 2007-2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-13 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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

#include "SplineLibIO.h"
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>

#include <vector>
#include <limits>
#include <iomanip>

void exportCurveToSplineLib(Handle_Geom_BSplineCurve curve, const std::string& filename) 
{
    TColStd_Array1OfReal knots(0, curve->NbKnots()-1) ;
    curve->Knots(knots);
    TColStd_Array1OfInteger multi(0, curve->NbKnots()-1) ;
    curve->Multiplicities(multi);

    TColgp_Array1OfPnt poles(0, curve->NbPoles()-1);
    curve->Poles(poles);
    TColStd_Array1OfReal weights(0, curve->NbPoles()-1);
    curve->Weights(weights);

    ofstream out(filename.c_str());
    out << setprecision(std::numeric_limits<double>::digits10 + 1);
    out << std::scientific;
    out << "# Splinelib geometry file" << std::endl;
    out << "# Type = B-Spline Curve" << std::endl;
    out << "# Dimensionality of control points : 3" << std::endl;
    out << "# Degree : " << curve->Degree() << std::endl;
    out << "# Is rational : " << curve->IsRational() << std::endl;
    out << "# Knots" << std::endl;
    for (int i = 0; i < knots.Length(); i++) {
        for (int j = 0; j < multi(i); j++) {
            out << knots(i); 
            if (j < multi(i)-1) {
                out << " ";
            }
        }
        if (i<knots.Length()-1) {
            out << " ";
        }
    }
    out << std::endl;

    out << "# Control points: Index, X, Y, (Z)" << std::endl;
    for (int i = 0; i < curve->NbPoles(); i++) {
        out << i << " " <<  poles(i).X() << " "<< poles(i).Y() << " "<< poles(i).Z() << std::endl;
    }

    if (curve->IsRational()) {
        out << "# Weights: Index, weight" << std::endl;
        for (int i = 0; i < curve->NbPoles(); i++) {
            out << weights(i) << std::endl;
        }
    }
    out.close();
}

void exportSurfaceToSplineLib(Handle_Geom_BSplineSurface surf, const std::string& filename) 
{
    TColStd_Array1OfReal knots_u(0, surf->NbUKnots()-1) ;
    surf->UKnots(knots_u);
    TColStd_Array1OfInteger multi_u(0, surf->NbUKnots()-1) ;
    surf->UMultiplicities(multi_u);

    TColStd_Array1OfReal knots_v(0, surf->NbVKnots()-1);
    surf->VKnots(knots_v);
    TColStd_Array1OfInteger multi_v(0, surf->NbVKnots()-1) ;
    surf->VMultiplicities(multi_v);

    TColgp_Array2OfPnt poles(0, surf->NbUPoles()-1, 0, surf->NbVPoles()-1);
    surf->Poles(poles);
    TColStd_Array2OfReal weights(0, surf->NbUPoles()-1, 0, surf->NbVPoles()-1);
    surf->Weights(weights);

    ofstream out(filename.c_str());
    out << setprecision(std::numeric_limits<double>::digits10 + 1);
    out << std::scientific;
    out << "# Splinelib geometry file" << std::endl;
    out << "# Type = B-Spline Surface" << std::endl;
    out << "# Dimensionality of control points : 3" << std::endl;
    out << "# Degree in U direction : " << surf->UDegree() << std::endl;
    out << "# Degree in V direction : " << surf->VDegree() << std::endl;
    out << "# Is rational : " << (surf->IsURational() || surf->IsVRational()) << std::endl;
    out << "# Knots in U direction" << std::endl;
    for (int i = 0; i < knots_u.Length(); i++) {
        for (int j = 0; j < multi_u(i); j++) {
            out << knots_u(i); 
            if (j < multi_u(i)-1) {
                out << " ";
            }
        }
        if (i<knots_u.Length()-1) {
            out << " ";
        }
    }
    out << std::endl;

    out << "# Knots in V direction" << std::endl;
    for (int i = 0; i < knots_v.Length(); i++) {
        for (int j = 0; j < multi_v(i); j++) {
            out << knots_v(i); 
            if (j < multi_v(i)-1) {
                out << " ";
            }
        }
        if (i<knots_v.Length()-1) {
            out << " ";
        }
    }
    out << std::endl;

    out << "# Control points: U-Index, V-Index, X, Y, (Z)" << std::endl;
    for (int i = 0; i < surf->NbUPoles(); i++) {
        for (int j = 0; j < surf->NbVPoles(); j++) {
            out << i << " " << j << " " << poles(i, j).X() << " "<< poles(i, j).Y() << " "<< poles(i, j).Z() << std::endl;
        }
    }

    if (surf->IsURational() || surf->IsVRational()) {
        out << "# Weights: U-Index, V-Index, weight" << std::endl;
        for (int i = 0; i < surf->NbUPoles(); i++) {
            for (int j = 0; j < surf->NbVPoles(); j++) {
                out << i << " " << j << " " << weights(i, j) << std::endl;
            }
        }
    }
    out.close();
}
