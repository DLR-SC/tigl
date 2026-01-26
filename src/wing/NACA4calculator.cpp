#include <iostream>
#include <iomanip>
#include <cmath>
#include <gp_Vec2d.hxx>
#include "NACAcalculator.h"


class NACA4Calculator{

    public:
        
        NACA4Calculator(double max_camber, double max_camber_position, double max_profile_thickness)
         : max_camber(max_camber/100)
         , max_camber_position(max_camber_position/10)
         , max_profile_thickness(max_profile_thickness/100)
        {}

        double camberline(double x){
        
            double m = max_camber;
            double p = max_camber_position;
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

        gp_Vec2d upper_curve(double x){
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point + yt*normal(x);
        }

        gp_Vec2d lower_curve(double x){
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point - yt*normal(x);
        }

    private:

        double profile_thickness(double x){
            double t = max_profile_thickness; 
            return 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - 0.1015*pow(x,4));
        }

        double camberline_derivative(double x){ //c'(x)
            double m = max_camber;
            double p = max_camber_position;

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

        gp_Vec2d normal(double x){
            gp_Vec2d normal = {-camberline_derivative(x), 1};
            double nrm = normal.norm();
            if (nrm < 1e-12) {
                return gp_Vec2d{0,1};
            }
            return normal/nrm;
        }

    private:
        double  max_camber, max_camber_position, max_profile_thickness;
};

