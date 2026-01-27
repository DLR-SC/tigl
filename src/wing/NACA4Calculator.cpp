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

#include <gp_Vec2d.hxx>
#include "NACA4Calculator.h"
        
        NACA4Calculator::NACA4Calculator(double max_camber, double max_camber_position, double max_profile_thickness)
         : max_camber(max_camber/100)
         , max_camber_position(max_camber_position/10)
         , max_profile_thickness(max_profile_thickness/100)//hier überprüfungen hinzuf+gen
        {}

        double NACA4Calculator::camberline(double x){
        
            double m = this->max_camber;
            double p = this->max_camber_position;
            if(0 <= x && x <= p){
                return (2*p*x - x*x)*m/(p*p);; 
            }
            else if(x > p){ 
                return (1 - 2*p + 2*p*x - x*x)*m/((1-p)*(1-p));;
            }
            else{
                throw std::logic_error("error in NACA4Calculator::camberline: x must be between 0 and 1.");
            }
        }

        gp_Vec2d NACA4Calculator::upper_curve(double x){
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point + yt*normal(x);
        }

        gp_Vec2d NACA4Calculator::lower_curve(double x){
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point - yt*normal(x);
        }

    

        double NACA4Calculator::profile_thickness(double x){
            double t = this->max_profile_thickness; 
            return 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - 0.1015*pow(x,4)); //muss nicht überprüft werden weild ie ja private is
        }

        double NACA4Calculator::camberline_derivative(double x){ //c'(x)
            double m = this->max_camber;
            double p = this->max_camber_position;

            if(0 < x && x <= p){
                return (2*p - 2*x)*m/(p*p);
            }
            else if(x > p){
                return ( 2*p - 2*x)*m/((1-p)*(1-p));;
            }
            else{
                throw std::logic_error("error in NACA4Calculator::camberline_derivative: x must be between 0 and 1.");
            }
        }

        gp_Vec2d NACA4Calculator::normal(double x){
            gp_Vec2d normal = {-camberline_derivative(x), 1};
            double nrm = normal.Magnitude(); 
            if (nrm < 1e-12) {
                return gp_Vec2d{0,1};
            }
            return normal/nrm;
        }


