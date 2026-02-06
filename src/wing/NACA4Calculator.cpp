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
#include "NACA4Calculator.h"
#include "CFunctionToBspline.h"

namespace tigl{
        
        NACA4Calculator::NACA4Calculator(double max_camber, double max_camber_position, double max_profile_thickness, double trailing_edge_thickness)
         : max_camber(max_camber/100)
         , max_camber_position(max_camber_position/10)
         , max_profile_thickness(max_profile_thickness/100)
         , trailing_edge_thickness(trailing_edge_thickness)
        {
            if(max_camber !=0 && max_camber !=2 && max_camber !=4 && max_camber !=6 ){
                throw ::std::logic_error("error in NACA4Calculator: max_camber must be 0; 2; 4 or 6.");
            }
            if(max_camber == 0 && max_camber_position != 0){
                throw ::std::logic_error("error in NACA4Calculator: if max_camber is 0 max_camber_position has to be 0 as well");
            }
            if(max_camber_position !=0 && max_camber_position !=2 && max_camber_position !=3 && max_camber_position !=4 && max_camber_position !=5 && max_camber_position !=6 && max_camber_position !=7 ){
                throw ::std::logic_error("error in NACA4Calculator: max_camber_position must be 0; 2; 3; 4; 5; 6 or 7.");
            }
            if(max_profile_thickness != 6 && max_profile_thickness != 9 && max_profile_thickness != 12 && max_profile_thickness != 15 && max_profile_thickness != 18 && max_profile_thickness != 21 && max_profile_thickness != 25){
                throw ::std::logic_error("error in NACA4Calculator: max_profile_thickness must be 6; 9; 12; 15; 18; 21 or 25.");
            }
        }


        NACA4Calculator::NACA4Calculator(::std::string const& naca_code)
            : NACA4Calculator(static_cast<double>(naca_code[0] - '0'),
                              static_cast<double>(naca_code[1] - '0'),
                              static_cast<double>(std::stoi(naca_code.substr(2,2))),
                              static_cast<double>(std::stoi(naca_code.substr(4,2))))
        {}


        double NACA4Calculator::camberline(double x) const{
        
            double m = this->max_camber;
            double p = this->max_camber_position;
            if(p == 0){
                return 0;
            }
            if(0 <= x && x <= p){
                return (2*p*x - x*x)*m/(p*p);; 
            }
            else if(x > p){ 
                return (1 - 2*p + 2*p*x - x*x)*m/((1-p)*(1-p));;
            }
            else{
                throw ::std::logic_error("error in NACA4Calculator::camberline: x must be between 0 and 1.");
            }
            if(::std::isnan(x)){
                throw ::std::logic_error("error in NACA4Calculator::camberline: x must be a number between 0 and 1.");
            }
        }

        gp_Vec2d NACA4Calculator::upper_curve(double x) const{
            double yt = profile_thickness(x); 
            double yc = camberline(x); //checked
            auto point = gp_Vec2d{x, yc};
            if(yt < 0){
                throw ::std::logic_error("error in NACA4Calculator::upper_curve: the profile_thickness must be positive or 0.");
            }
            return point + yt*normal(x);
        }
        /**
        * @brief calculates the y-coordinate
        * 
        * @param x 
        * @return gp_Vec2d 
        */
        gp_Vec2d NACA4Calculator::lower_curve(double x) const{ 
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            if(yt < 0){
                throw ::std::logic_error("error in NACA4Calculator::upper_curve: the profile_thickness must be positive or 0.");
            }
            return point - yt*normal(x);
        }

        double NACA4Calculator::profile_thickness(double x) const{ //wird in upper/lower curve getestet, da gehts leichter, sollt ihcs trzdm hier hins chreibn?
            double t = this->max_profile_thickness; 
            double e = trailing_edge_thickness_function(trailing_edge_thickness);
            return 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - e*pow(x,4));
        }

        double NACA4Calculator::camberline_derivative(double x) const{ //c'(x)
            double m = this->max_camber;
            double p = this->max_camber_position;

            if(0 <= x && x <= p){
                return (2*p - 2*x)*m/(p*p);
            }
            else if(x > p){
                return ( 2*p - 2*x)*m/((1-p)*(1-p));;
            }
            else{
                throw ::std::logic_error("error in NACA4Calculator::camberline_derivative: x must be between 0 and 1.");
            }
        }

        gp_Vec2d NACA4Calculator::normal(double x) const{
            gp_Vec2d normal = {-camberline_derivative(x), 1};
            double nrm = normal.Magnitude(); 
            if (nrm < 1e-12) {
                return gp_Vec2d{0,1};
            }
            return normal/nrm;
        }

        double NACA4Calculator::trailing_edge_thickness_function(double y) const{ //y ist die dicke bei x = 1 aber wenn ihc fürs gl profil bei x = 0.5 die dicke ausrechnen will brauche ich ja dnn das e
            double t = max_profile_thickness;
            if(y < 0){
                throw ::std::logic_error("error in NACA4Calculator::trailing_edge_thickness_function: trailing_edge_thickness must be bigger then 0.");
            }
            return -(y/(5*t)) + (0.2969 - 0.1260 - 0.3516 + 0.2843);
        }

        Handle(Geom_BSplineCurve) NACA4Calculator::upper_bspline() const{
            NACA4UpperCurve upperCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;
            int maxDepth = 10;


            tigl::CFunctionToBspline converter(upperCurve, umin, umax, degree, tolerance, maxDepth);     //zweck: das richtige format übergeben an cfunctiontobspline
            return converter.Curve();
        }

        NACA4UpperCurve::NACA4UpperCurve( NACA4Calculator const& calculator)
            : MathFunc3d(), //wurde bei der superellipse so gemacht, brauch ich das hier also auch?
            calculator(calculator)
        {}

        double NACA4UpperCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.X();
        }
        double NACA4UpperCurve::valueY(double t)  {
            return 0.0;
        }
        double NACA4UpperCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.Y();
        }

        //naca4uppercurve und die soll die upper_curve vom naca4calculator in  mathfunc3d übersetzen (in des value - format), also ich übergebe hier das ergebnis von upper_curve()
// x vom punkt = value.x
// y vom punkt = value.z
// value.y = 0


//das gleiche auch noch für lower_curve!

//vgl. superellipse in commonfunctions.cpp


/*
namespace
{
    class NACA4UpperCurve : public tigl::MathFunc3d
    {
    public:
        NACA4UpperCurve(NACA4Calculator& calculator)
            : tigl::MathFunc3d(),
             {}

        double valueX(double t) override
        {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.X();
        }

        double valueY(double t) override
        {
            return 0.0;
        }

        double valueZ(double t) override
        {
            gp_Vec2d vec = calculator.upper_curve(t);
            return vec.Y();
        }

    private:
          
    };

} //anonymos namespace
 */


} //namespace tigl


