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
         : series_(Series::NACA4)
         , max_camber(max_camber/100)
         , max_camber_position(max_camber_position/10)
         , max_profile_thickness(max_profile_thickness/100)
         , trailing_edge_thickness_half(trailing_edge_thickness/2) 
        {
            if(this->max_camber > 1 || this->max_camber < 0){
                throw ::std::logic_error("error in NACA4Calculator max_camber after normalization must be between 0 and 1.");
            }
            if(this->max_camber_position > 1 || this->max_camber_position < 0){
                throw ::std::logic_error("error in NACA4Calculator max_camber_position after normalization must be between 0 and 1.");
            }
            if(this->max_profile_thickness > 1 || this->max_profile_thickness < 0){
                throw ::std::logic_error("error in NACA4Calculator max_profile_thickness after normalization must be between 0 and 1.");
            }
        }

        NACA4Calculator::NACA4Calculator(double max_camber_cl, double max_camber_position, double reflex, double max_profile_thickness, double trailing_edge_thickness)
         : series_(Series::NACA5)
         , max_camber(max_camber_cl/100)
         , max_camber_position(max_camber_position/20)
         , reflex(reflex) //überprüfen, ob hier wirklich nh 0 oder1 eingegben wurde
         , max_profile_thickness(max_profile_thickness/100)
         , trailing_edge_thickness_half(trailing_edge_thickness/2) 
        {
            
        }

        NACA4Calculator::NACA4Calculator(::std::string const& naca_code , const double te_thickness)
            : NACA4Calculator(0.0,
                              0.0,
                              0.0,
                              te_thickness)
        {
            if (naca_code.size() == 4){
                try{
                    double m = static_cast<double>(naca_code[0] - '0');
                    double p = static_cast<double>(naca_code[1] - '0');
                    double t = static_cast<double>(std::stoi(naca_code.substr(2,2)));

                    *this = NACA4Calculator(m, p, t, te_thickness);
                }
                catch(...){
                    throw std::invalid_argument("error in NACA4Calculator: the naca_code format is not correct, it must to contain four digits and nothing else");
                }}
            else if (naca_code.size() == 5){
                try{
                    double s = static_cast<double>(naca_code[0] - '0');
                    double p = static_cast<double>(naca_code[1] - '0');
                    double q = static_cast<double>(naca_code[2] - '0');
                    double tt = static_cast<double>(std::stoi(naca_code.substr(3,2)));

                    *this = NACA4Calculator(s, p, q, tt, te_thickness);
                }
                catch(...){
                throw std::invalid_argument("error in NACA4Calculator: the naca_code format is not correct, it must to contain four digits and nothing else");
            }
            }
            else{
                throw std::invalid_argument("error in NACA4Calculator: the naca_code format is not correct, it must to contain four or five digits and nothing else");
            }

        }

        

        double NACA4Calculator::get_trailing_edge_thickness() const
        {
            return trailing_edge_thickness_half;
        }

        double NACA4Calculator::k1_const(double max_camber, double max_camber_position, double reflex) const{
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
                    throw ::std::logic_error("error in NACA4Calculator::k1: this profile does not provide a constant for k1."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
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
                    throw ::std::logic_error("error in NACA4Calculator::k1: this profile does not provide a constant for k1."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                }
            return k1; 
        }

        double NACA4Calculator::m_const(double max_camber, double max_camber_position, double reflex) const{
            int max_camber_cl_whole = static_cast<int>(max_camber*100);
            int max_camber_position_whole = static_cast<int>(max_camber_position*20);
            int reflex_int = static_cast<int>(reflex);
            double m;
            //alles von chapter 8 table 8-6
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
                    throw ::std::logic_error("error in NACA4Calculator::m: this profile does not provide a constant for m."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
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
                    throw ::std::logic_error("error in NACA4Calculator::m: this profile does not provide a constant for m."); //gibts hier nh ander elösung, mit dem ich rechnen kann?
                    break;
                }
            return m; 
        }


        double NACA4Calculator::camberline(double x) const{
        
            if(series_ == Series::NACA4){
                double m = this->max_camber;
                double p = this->max_camber_position;
                if(p == 0){
                    return 0;
                }
                if(x < 0 || x > 1){
                    throw ::std::logic_error("error in NACA4Calculator::camberline::NACA4: x must be between 0 and 1.");
                }
                else if(0 <= x && x <= p){
                    return (2*p*x - x*x)*m/(p*p);
                }
                else if(x > p){ 
                    return (1 - 2*p + 2*p*x - x*x)*m/((1-p)*(1-p));
                }
            }
            else if(series_ == Series::NACA5){
                double s = this->max_camber;
                std::cerr << "s " << s<< std::endl;
                double p = this->max_camber_position;
                std::cerr << "p " << p<< std::endl;
                double q = this->reflex;
                std::cerr << "q " << q<< std::endl;
                double k1 = k1_const(s, p, q);
                std::cerr << "k1 " << k1<< std::endl;
                double m = m_const(s, p, q);
                std::cerr << "m " << m<< std::endl;
                double frack2k1 = (3*((m-p)*(m-p))-(m*m*m))/((1-m)*(1-m)*(1-m));
                std::cerr << "frack2k1 " << frack2k1<< std::endl;
                if(q == 0){
                     //std::cerr << "x " << x<< std::endl;

                    if(p == 0){
                        return 0;
                    }
                    if(0 <= x && x <= p){
                        double result = (k1/6)*(x*x*x-3*m*x*x+m*m*(3-m)*x); 
                        //std::cerr << "result okay " << result << std::endl;
                        //std::cerr << "m " << m<< std::endl;
                        //double yt = profile_thickness(x); 
                        //std::cerr << "yt " << yt<< std::endl;
                        //double yc = camberline(x);
                        //std::cerr << "camberline " << yc<< std::endl;
                        return result;
                    }
                    else if(x > p){ 
                        double result3 = ((k1*m*m*m)/6)*(1-x);
                        //std::cerr << "result3 " << result3 << std::endl;
                         //std::cerr << "p " << p<< std::endl;
                        //double yt = profile_thickness(x); 
                        //double yc = camberline(x);
                        //std::cerr << "camberline " << yc<< std::endl;
                        
                        return result3;
                    }
                }
                else if(q == 1){
                        //std::cerr << "x " << x<< std::endl;

                    if(p == 0){
                        return 0.0;
                    }
                    if(0 <= x && x <= p){
                    
                        //double result1 = ((k1/6)*(((x-m)*(x-m)*(x-m))-(frack2k1*((1-m)*(1-m)*(1-m))*x)- ((m*m*m)*x+(m*m*m)))); 
                        double result1 = ((((k1/6)*(pow(x-m, 3)))-(frack2k1*(pow(1-m,3))*x)- (pow(m,3)*x+pow(m,3))))/2; 
                        //std::cerr << "x<p " << x<< std::endl;
                        //std::cerr << "m " << m<< std::endl;
                        //std::cerr << "x " << x<< std::endl;
                        //std::cerr << "k1 " << k1<< std::endl;
                        //std::cerr << "frack2k1 " << frack2k1<< std::endl;
                        //std::cerr << "result1 " << result1<< std::endl;
                        //double yt = profile_thickness(x); 
                        //std::cerr << "yt " << yt<< std::endl;
                        return result1;
                    }
                    else if(x > p){ 
                        double result2 = (k1/6)*(frack2k1*((x-m)*(x-m)*(x-m))-frack2k1*((1-m)*(1-m)*(1-m))*x-(m*m*m)*x+(m*m*m));
                        //std::cerr << "x>p " << x<< std::endl;
                        //std::cerr << "result2 " << frack2k1 << std::endl;
                        //std::cerr << "p " << p<< std::endl;
                        //double yt = profile_thickness(x); 
                        //std::cerr << "yt " << yt<< std::endl;
                        return result2;
                    }
                    }
                else{
                    throw ::std::logic_error("error in NACA4Calculator::camberline::NACA5: x must be between 0 and 1.");
                }
                return 0;
            }
            return 0;
        }

        
        gp_Vec2d NACA4Calculator::upper_curve(double x) const{
            std::cerr << "x \t  " << x;
            double yt = profile_thickness(x); 
            std::cerr << "\t profile thickness \t  " << yt;
            double yc = camberline(x);
            std::cerr << "\t camberline \t " << yc << std::endl;
            auto point = gp_Vec2d{x, yc};
            //std::cerr << "\t point.x \t " << point.X();
            //std::cerr << "\t point.y \t " << point.Y() << std::endl;
            gp_Vec2d point_calculated = point + yt*normal(x);
            //std::cerr << "\t point_calculatedX \t " << point_calculated.X();
            //std::cerr << "\t point_calculatedY \t " << point_calculated.Y() << "\n\n";
            return point + yt*normal(x);
        }
        
        gp_Vec2d NACA4Calculator::lower_curve(double x) const{ 
            double yt = profile_thickness(x);
            double yc = camberline(x);
            auto point = gp_Vec2d{x, yc};
            return point - yt*normal(x);
        }

        double NACA4Calculator::profile_thickness(double x) const{ 
            double t = this->max_profile_thickness; 
            double e = -(trailing_edge_thickness_half/(5*t)) + (0.2969 - 0.1260 - 0.3516 + 0.2843);
            return 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - e*pow(x,4));
        }

        double NACA4Calculator::camberline_derivative(double x) const{ //c'(x)

            if(series_ == Series::NACA4){
                double m = this->max_camber;
                double p = this->max_camber_position;

                if(p == 0){
                    return 0;
                }
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
            else if(series_ == Series::NACA5){
                double s = this->max_camber;
                double p = this->max_camber_position;
                double q = this->reflex;
                double k1 = k1_const(s, p, q);
                double m = m_const(s, p, q);
                double frack2k1 = (3*((m-p)*(m-p))-m*m*m)/((1-m)*(1-m)*(1-m));
                if(p == 0){
                    return 0;
                }
                if(q == 0){
                    if(0 <= x && x <= p){
                        double result6 = (k1/6)*(3*x*x-6*m*x+m*m*(3-m));
                        //std::cerr << "result6 okay " << result6 << std::endl;
                        return result6;
                    }
                    else if(x > p){
                        return -((k1*m*m*m)/6);
                    }
                }
                else if(q==1){
                    if(0 <= x && x <= p){
                        double result5 = ((k1/6)*(3*((x-m)*(x-m))-frack2k1*((1-m)*(1-m)*(1-m))-m*m*m));
                        //std::cerr << "result5 " << result5 << std::endl;
                        return result5;
                    }
                    else if(x > p){
                        return (k1/6)*(3*frack2k1*((x-m)*(x-m))-frack2k1*((1-m)*(1-m)*(1-m))-m*m*m);
                    }
                }
                else{
                    throw ::std::logic_error("error in NACA4Calculator::camberline_derivative: x must be between 0 and 1.");
                }
                return 0;
            }
            return 0;
        }

        gp_Vec2d NACA4Calculator::normal(double x) const{
            gp_Vec2d normal = {-camberline_derivative(x), 1};
            double nrm = normal.Magnitude(); 
            if (nrm < 1e-12) {
                return gp_Vec2d{0,1};
            }
            return normal/nrm;
        }

        Handle(Geom_BSplineCurve) NACA4Calculator::upper_bspline() const{

            NACA4UpperCurve upperCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;//war 5!!
            int maxDepth = 10;

            tigl::CFunctionToBspline converter(upperCurve, umin, umax, degree, tolerance, maxDepth); 
            return converter.Curve();
        }

        Handle(Geom_BSplineCurve) NACA4Calculator::lower_bspline() const{
            NACA4LowerCurve lowerCurve(*this);

            const double umin = 0.;
            const double umax = 1.;
            int degree = 3;
            double tolerance=1e-5;
            int maxDepth = 10;

            tigl::CFunctionToBspline converter(lowerCurve, umin, umax, degree, tolerance, maxDepth);
            return converter.Curve();
        }

        NACA4UpperCurve::NACA4UpperCurve( NACA4Calculator const& calculator)
            : MathFunc3d(), 
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

        NACA4LowerCurve::NACA4LowerCurve( NACA4Calculator const& calculator)
            : MathFunc3d(), 
            calculator(calculator)
        {}

        double NACA4LowerCurve::valueX(double t)  {
            gp_Vec2d vec = calculator.lower_curve(t);
            return vec.X();
        }

        double NACA4LowerCurve::valueY(double t)  {
            return 0.0;
        }

        double NACA4LowerCurve::valueZ(double t)  {
            gp_Vec2d vec = calculator.lower_curve(t);
            return vec.Y();
        }

} //namespace tigl


