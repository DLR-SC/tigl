/* 
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-14 Hannah Gedler <hannah.gedler@dlr.de>
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
/**
* @file
* @brief Calculator for NACA wing profile coordinates
*/

#include "gp_Vec2d.hxx"
#include "tiglmathfunctions.h"
#include "CTiglNACACalculator.h"
#include "CFunctionToBspline.h"
#include "CTiglBSplineApproxInterp.h"
#include "CTiglError.h"
#include <math.h>


namespace
{
    // Leading-edge reparametrization: x(t) = (1+eps)*t*t/(t+eps).
    // Removes the thickness distribution's sqrt(x) derivative singularity at the leading
    // edge (x(0)=0, x'(0)=0, same as a plain t*t substitution close to t=0), but - unlike a
    // plain t*t, which reshapes the parametrization over the *entire* chord - relaxes back
    // towards the identity x(t)~=t once t significantly exceeds eps, so the curve fit away
    // from the leading edge (and its knot placement) stays close to the original, direct-x
    // parametrization. A plain t*t was found to shift curve representation enough at
    // e.g. x=0.25 to break a sibling-component boolean fuse in specific geometries; eps=0.02
    // keeps the identity-like region starting well before that.
    double leParam(double t)
    {
        const double eps = 0.02;
        return (1. + eps) * t * t / (t + eps);
    }
}

namespace tigl{

        NACA4DigitCode::NACA4DigitCode(std::string const& code) { // struct constructor oder so?
        if (code.size() != 4) {
            throw CTiglError("NACA4DigitCode: requires exactly 4 digits");
        }

        max_camber           = (code[0] - '0') / 100.0;
        max_camber_position  = (code[1] - '0') / 10.0;
        max_profile_thickness = std::stoi(code.substr(2, 2)) / 100.0;
        }

        NACA5DigitCode::NACA5DigitCode(std::string const& code) { // struct constructor oder so?
            if (code.size() != 5) {
                throw CTiglError("NACA5DigitCode: requires exactly 5 digits");
            }

            max_camber           = (code[0] - '0') / 100.0;
            max_camber_position  = (code[1] - '0') / 20.0;
            reflex               = code[2] - '0';
            max_profile_thickness = std::stoi(code.substr(3, 2)) / 100.0;
        }

        CTiglNACACalculator::CTiglNACACalculator(const NACA4DigitCode& code, double trailing_edge_thickness)
            : series_(Series::NACA4)
            , max_camber(code.max_camber)
            , max_camber_position(code.max_camber_position)
            , max_profile_thickness(code.max_profile_thickness)
            , trailing_edge_thickness_half(trailing_edge_thickness / 2.0)
        {
            if(this->max_camber > 1 || this->max_camber < 0){
                throw CTiglError("error in CTiglNACACalculator The argument max_camber must be between 0 and 9.");
            }
            if(this->max_camber_position > 1 || this->max_camber_position < 0){
                throw CTiglError("error in CTiglNACACalculator The argument max_camber_position must be between 0 and 9.");
            }
            if(this->max_profile_thickness > 1 || this->max_profile_thickness < 0){
                throw CTiglError("error in CTiglNACACalculator max_profile_thicknessmust be between 0 and 99.");
            }
        }

        CTiglNACACalculator::CTiglNACACalculator(const NACA5DigitCode& code, double trailing_edge_thickness)
            : series_(Series::NACA5)
            , max_camber(code.max_camber)
            , max_camber_position(code.max_camber_position)
            , reflex(code.reflex)
            , max_profile_thickness(code.max_profile_thickness)
            , trailing_edge_thickness_half(trailing_edge_thickness / 2.0)
        {
            if(this->max_camber > 1 || this->max_camber < 0){
                throw CTiglError("error in CTiglNACACalculator The argument max_camber must be between 0 and 9.");
            }
            if(this->max_camber_position > 1 || this->max_camber_position < 0){
                throw CTiglError("error in CTiglNACACalculator The argument max_camber_position must be between 0 and 9.");
            }
            if(this->max_profile_thickness > 1 || this->max_profile_thickness < 0){
                throw CTiglError("error in CTiglNACACalculator max_profile_thicknessmust be between 0 and 99.");
            }
            if(this->reflex != 0 && this->reflex != 1){
                throw CTiglError("error in CTiglNACACalculator The argument reflex must be 0 or 1.");
            }
        }
        /*
        CTiglNACACalculator::CTiglNACACalculator(double max_camber, double max_camber_position, double max_profile_thickness, double trailing_edge_thickness)
         : series_(Series::NACA4)
         , max_camber(max_camber/100)
         , max_camber_position(max_camber_position/10)
         , max_profile_thickness(max_profile_thickness/100)
         , trailing_edge_thickness_half(trailing_edge_thickness/2) 
        {
            if(this->max_camber > 1 || this->max_camber < 0){
                throw CTiglError("error in CTiglNACACalculator The argument max_camber must be between 0 and 9.");
            }
            if(this->max_camber_position > 1 || this->max_camber_position < 0){
                throw CTiglError("error in CTiglNACACalculator The argument max_camber_position must be between 0 and 9.");
            }
            if(this->max_profile_thickness > 1 || this->max_profile_thickness < 0){
                throw CTiglError("error in CTiglNACACalculator max_profile_thicknessmust be between 0 and 99.");
            }
        }

        CTiglNACACalculator::CTiglNACACalculator(double max_camber_cl, double max_camber_position, double reflex, double max_profile_thickness, double trailing_edge_thickness)
         : series_(Series::NACA5)
         , max_camber(max_camber_cl/100)
         , max_camber_position(max_camber_position/20)
         , reflex(reflex) //überprüfen, ob hier wirklich nh 0 oder 1 eingegben wurde
         , max_profile_thickness(max_profile_thickness/100)
         , trailing_edge_thickness_half(trailing_edge_thickness/2) 
        {
            
        }

        CTiglNACACalculator::CTiglNACACalculator(::std::string const& naca_code , const double te_thickness)
            : CTiglNACACalculator(0.0,
                              0.0,
                              0.0,
                              te_thickness)
        {
            if (naca_code.size() == 4){
                try{
                    double m = static_cast<double>(naca_code[0] - '0');
                    double p = static_cast<double>(naca_code[1] - '0');
                    double t = static_cast<double>(std::stoi(naca_code.substr(2,2)));

                    *this = CTiglNACACalculator(m, p, t, te_thickness);
                }
                catch(...){
                    throw CTiglError("error in CTiglNACACalculator: the naca_code format is not correct, it must to contain four digits and nothing else");
                }}
            else if (naca_code.size() == 5){
                try{
                    double s = static_cast<double>(naca_code[0] - '0');
                    double p = static_cast<double>(naca_code[1] - '0');
                    double q = static_cast<double>(naca_code[2] - '0');
                    double tt = static_cast<double>(std::stoi(naca_code.substr(3,2)));

                    *this = CTiglNACACalculator(s, p, q, tt, te_thickness);
                }
                catch(...){
                throw CTiglError("error in CTiglNACACalculator: the naca_code format is not correct, it must to contain four digits and nothing else");
            }
            }
            else{
                throw std::invalid_argument("error in CTiglNACACalculator: the naca_code format is not correct, it must to contain four or five digits and nothing else");
            }

        }*/

        double CTiglNACACalculator::get_trailing_edge_thickness() const
        {
            return trailing_edge_thickness_half;
        }

        double CTiglNACACalculator::k1_const(double max_camber, double max_camber_position, double reflex) const{
            int max_camber_cl_whole = static_cast<int>(max_camber*100);
            int max_camber_position_whole = static_cast<int>(max_camber_position*20);
            int reflex_int = static_cast<int>(reflex);
            double k1;
            //alles von chapter 8 table 8-6
            std::string meanline_designation_str = std::to_string(max_camber_cl_whole) + std::to_string(max_camber_position_whole) + std::to_string(reflex_int);
            int meanline_designation = std::stoi(meanline_designation_str);
            switch(meanline_designation){
                case 210:
                     k1 =361.400;
                     break;
                case 220:
                     k1 =51.640;
                     break;
                case 230:
                     k1 = 15.957;
                     break;
                case 240:
                    k1 = 6.643;
                    break;
                case 250:
                    k1 = 3.230;
                    break;
                case 211:
                    throw ::std::logic_error("error in CTiglNACACalculator::k1: this profile does not provide a constant for k1."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                case 221:
                    k1 = 51.99;
                    break;
                case 231:
                    k1 = 15.793;
                    break;
                case 241:
                    k1 = 6.520;
                    break;
                case 251:
                    k1 = 3.191;
                    break;
                default:
                    throw ::std::logic_error("error in CTiglNACACalculator::k1: this profile does not provide a constant for k1."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                }
            return k1; 
        }

        double CTiglNACACalculator::m_const(double max_camber, double max_camber_position, double reflex) const{
            int max_camber_cl_whole = static_cast<int>(max_camber*100);
            int max_camber_position_whole = static_cast<int>(max_camber_position*20);
            int reflex_int = static_cast<int>(reflex);
            double m;
            //alles von chapter 8 table 8-6 und von https://ntrs.nasa.gov/api/citations/19970008124/downloads/19970008124.pdf
            std::string meanline_designation_str = std::to_string(max_camber_cl_whole) + std::to_string(max_camber_position_whole) + std::to_string(reflex_int);
            int meanline_designation = std::stoi(meanline_designation_str);
            switch(meanline_designation){
                case 210:
                     m = 0.0580;
                     break;
                case 220:
                     m = 0.1260;
                     break;
                case 230:
                     m = 0.2025;
                     break;
                case 240:
                    m = 0.2900;
                    break;
                case 250:
                    m = 0.3910;
                    break;
                case 211:
                    throw ::std::logic_error("error in CTiglNACACalculator::m: this profile does not provide a constant for m."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                case 221:
                    m = 0.1300;
                    break;
                case 231:
                    m = 0.2170;
                    break;
                case 241:
                    m = 0.3180;
                    break;
                case 251:
                    m = 0.4410;
                    break;
                default:
                    throw ::std::logic_error("error in CTiglNACACalculator::m: this profile does not provide a constant for m."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                }
            return m; 
        }

        double CTiglNACACalculator::k2k1_const(double max_camber, double max_camber_position, double reflex) const{
            int max_camber_cl_whole = static_cast<int>(max_camber*100);
            int max_camber_position_whole = static_cast<int>(max_camber_position*20);
            int reflex_int = static_cast<int>(reflex);
            double k2k1;
            //alles von chapter 8 table 8-6 und von https://ntrs.nasa.gov/api/citations/19970008124/downloads/19970008124.pdf
            std::string meanline_designation_str = std::to_string(max_camber_cl_whole) + std::to_string(max_camber_position_whole) + std::to_string(reflex_int);
            int meanline_designation = std::stoi(meanline_designation_str);
            switch(meanline_designation){
                case 210:
                     throw ::std::logic_error("error in CTiglNACACalculator::k2k1: this profile does not provide a constant for k2/k1.");
                     break;
                case 220:
                     throw ::std::logic_error("error in CTiglNACACalculator::k2k1: this profile does not provide a constant for k2/k1.");
                     break;
                case 230:
                     throw ::std::logic_error("error in CTiglNACACalculator::k2k1: this profile does not provide a constant for k2/k1m.");
                     break;
                case 240:
                    throw ::std::logic_error("error in CTiglNACACalculator::k2k1: this profile does not provide a constant for k2/k1.");
                    break;
                case 250:
                    throw ::std::logic_error("error in CTiglNACACalculator::k2k1: this profile does not provide a constant for k2/k1.");
                    break;
                case 211:
                    throw ::std::logic_error("error in CTiglNACACalculator::k2k1: this profile does not provide a constant for k2/k1."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                case 221:
                    k2k1 = 0.000764;
                    break;
                case 231:
                    k2k1 = 0.00677;
                    break;
                case 241:
                    k2k1 = 0.0303;
                    break;
                case 251:
                    k2k1 = 0.1355;
                    break;
                default:
                    throw ::std::logic_error("error in CTiglNACACalculator::k2k1: this profile does not provide a constant for m."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                }
            return k2k1; 
        }




        

        double CTiglNACACalculator::camberline(double x) const{
        
            if(series_ == Series::NACA4){
                double m = this->max_camber;
                double p = this->max_camber_position;
                if(p == 0){
                    return 0;
                }
                if(x < 0 || x > 1){
                    throw CTiglError("error in CTiglNACACalculator::camberline::NACA4: x must be between 0 and 1.");
                }
                else if(x <= p){
                    return (2*p*x - x*x)*m/(p*p);
                }
                else if(x > p){ 
                    return (1 - 2*p + 2*p*x - x*x)*m/((1-p)*(1-p));
                }
            }
            else if(series_ == Series::NACA5){
                double s = this->max_camber;

                double p = this->max_camber_position;

                double q = this->reflex;

                double k1 = k1_const(s, p, q);

                double m = m_const(s, p, q);

                if(q == 0){

                    if(p == 0){
                        return 0;
                    }
                    if(0 <= x && x <= m){
                        double result = (k1/6)*(x*x*x-3*m*x*x+m*m*(3-m)*x); 
                        return result;
                    }
                    else if(x > m){ 
                        double result3 = ((k1*m*m*m)/6)*(1-x);
                        return result3;
                    }
                }
                else if(q == 1){
                    double frack2k1 = k2k1_const(s,p,q);

                    if(p == 0){
                        return 0.0;
                    }
                    if(0 <= x && x <= m){
                        double result1 = (k1/6)*(pow(x-m, 3) - frack2k1*pow(1-m, 3)*x - pow(m,3)*x + pow(m,3)); 
                        //getloft funktion mit timeit laufzeit rausfinden -> auf cache achten, das der nicht gespeichert wird
                        
                        return result1;
                    }
                    else if(x > m){ 
                    
                        double result2 = (k1/6)*(frack2k1*((x-m)*(x-m)*(x-m))-frack2k1*((1-m)*(1-m)*(1-m))*x-(m*m*m)*x+(m*m*m));
                        return result2;
                    }
                    }
                else{
                    throw ::std::logic_error("error in CTiglNACACalculator::camberline::NACA5: x must be between 0 and 1.");
                }
                return 0;
            }
            return 0;
        }

        
        gp_Vec2d CTiglNACACalculator::upper_curve(double x) const{
            double yt = profile_thickness(x); 
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            gp_Vec2d point_calculated = point + yt*normal(x);
            return point + yt*normal(x);
        }
        
        gp_Vec2d CTiglNACACalculator::lower_curve(double x) const{ 
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point - yt*normal(x);
        }

        double CTiglNACACalculator::profile_thickness(double x) const{ 
            double t = this->max_profile_thickness; 
            double e = -(trailing_edge_thickness_half/(5*t)) + (0.2969 - 0.1260 - 0.3516 + 0.2843);
            return 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - e*pow(x,4));
        }

        double CTiglNACACalculator::camberline_derivative(double x) const{ //c'(x)

            if(series_ == Series::NACA4){
                double m = this->max_camber;
                double p = this->max_camber_position;

                if(p == 0){
                    return 0;
                }
                if(0 <= x && x <= m){ //beim git merge hatte ich als altenative das hier statt m p ist
                    return (2*p - 2*x)*m/(p*p);
                }
                else if(x > m){ //beim git merge hatte ich als altenative das hier statt m p ist
                    return ( 2*p - 2*x)*m/((1-p)*(1-p));;
                }
                else{
                    throw CTiglError("error in CTiglNACACalculator::camberline_derivative: x must be between 0 and 1.");
                }
            }
            else if(series_ == Series::NACA5){
                double s = this->max_camber;
                double p = this->max_camber_position;
                double q = this->reflex;
                double k1 = k1_const(s, p, q);
                double m = m_const(s, p, q);
                
                if(p == 0){
                    return 0;
                }
                if(q == 0){
                    if(0 <= x && x <= m){
                        double result6 = (k1/6)*(3*x*x-6*m*x+m*m*(3-m));
                        //std::cerr << "result6 okay " << result6 << std::endl;
                        return result6;
                    }
                    else if(x > m){ //hier statt p au 0.3180 oder war das was anderes? //hier war p
                        return -((k1*m*m*m)/6);
                    }
                }
                else if(q==1){
                    double frack2k1 = (3*((m-p)*(m-p))-m*m*m)/((1-m)*(1-m)*(1-m)); //hier nicht auch die methode benutzen
                    if(0 <= x && x <= m){
                    
                        double result5 = ((k1/6)*(3*((x-m)*(x-m))-frack2k1*((1-m)*(1-m)*(1-m))-m*m*m));
                        //std::cerr << "result5 " << result5 << std::endl;
                        return result5;
                    }
                    else if(x > m){//0.3180
                    
                        return (k1/6)*(3*frack2k1*((x-m)*(x-m))-frack2k1*((1-m)*(1-m)*(1-m))-m*m*m);
                    }
                }
                else{
                    throw CTiglError("error in CTiglNACACalculator::camberline_derivative: x must be between 0 and 1.");
                }
                return 0;
            }
            return 0;
        }

        gp_Vec2d CTiglNACACalculator::normal(double x) const{
            gp_Vec2d normal = {-camberline_derivative(x), 1};
            double nrm = normal.Magnitude(); 
            if (nrm < 1e-12) {
                return gp_Vec2d{0,1};
            }
            return normal/nrm;
        }

        Handle(Geom_BSplineCurve) CTiglNACACalculator::upper_bspline() const{
            /*
            int npnts = 100;
            auto pnts = TColgp_Array1OfPnt(1, npnts);
            for (int i=1; i<=npnts; ++i) {
                double u = ((double)i-1)/(npnts-1);
                //double x = 0.5 * (1.0 - cos(M_PI * u));
                double x = pow(0.5 * (1.0 - cos(M_PI * u)), 0.5); //more clustering (bc of the pow)
                auto p = upper_curve(x);
                pnts.SetValue(i, gp_Pnt(p.X(), 0., p.Y()));
                std::cout << p.X() << ", " << p.Y() << "\n";
            }

            int nControlPoints = 34;
            int deg = 3;
            bool continuous_if_closed = false;
            auto builder = CTiglBSplineApproxInterp(pnts, nControlPoints, deg, continuous_if_closed);
            builder.InterpolatePoint(0);
            builder.InterpolatePoint(npnts-1);
            auto result = builder.FitCurveOptimal();
            return result.curve;
            */

            CTiglNACA4UpperCurve upperCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;//war 5!!
            int maxDepth = 10;//war 10!!

            tigl::CFunctionToBspline converter(upperCurve, umin, umax, degree, tolerance, maxDepth); 
            return converter.Curve();
        }

        Handle(Geom_BSplineCurve) CTiglNACACalculator::lower_bspline() const{
            /*
            int npnts = 100;
            auto pnts = TColgp_Array1OfPnt(1, npnts);
            for (int i=1; i<=npnts; ++i) {
                double u = ((double)i-1)/(npnts-1); // cosine distribution? closer at leading edge
                double x = pow(0.5 * (1.0 - cos(M_PI * u)), 0.5); //more clustering (bc of the pow)
                //double x = 0.5 * (1.0 - cos(M_PI * u));
                auto p = lower_curve(x);
                pnts.SetValue(i, gp_Pnt(p.X(), 0., p.Y()));
            }

            int nControlPoints = 34;
            int deg = 3;
            bool continuous_if_closed = false;
            auto builder = CTiglBSplineApproxInterp(pnts, nControlPoints, deg, continuous_if_closed);
            builder.InterpolatePoint(0);
            builder.InterpolatePoint(npnts-1);
            auto result = builder.FitCurveOptimal();
            return result.curve;
            */

            CTiglNACA4LowerCurve lowerCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;
            int maxDepth = 10;

            tigl::CFunctionToBspline converter(lowerCurve, umin, umax, degree, tolerance, maxDepth);
            return converter.Curve();
        }

        CTiglNACA4UpperCurve::CTiglNACA4UpperCurve( CTiglNACACalculator const& calculator)
            : MathFunc3d(), 
            calculator(calculator)
        {}

        double CTiglNACA4UpperCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.upper_curve(leParam(t));
            return vec.X();
        }
        double CTiglNACA4UpperCurve::valueY(double t)  {
            return 0.0;
        }
        double CTiglNACA4UpperCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.upper_curve(leParam(t));
            return vec.Y();
        }

        CTiglNACA4LowerCurve::CTiglNACA4LowerCurve( CTiglNACACalculator const& calculator)
            : MathFunc3d(), 
            calculator(calculator)
        {}

        double CTiglNACA4LowerCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.lower_curve(leParam(t));
            return vec.X();
        }

        double CTiglNACA4LowerCurve::valueY(double t)  {
            return 0.0;
        }

        double CTiglNACA4LowerCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.lower_curve(leParam(t));
            return vec.Y();
        }

} //namespace tigl


