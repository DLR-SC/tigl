#include <iostream>
#include <cmath>
#include "NACAcalculator.h"

class NACAcalculator{

    public:
        
        NACAcalculator(double curvatue_reserve, double maximum_curvature, double maximum_profile_thickness){
            curvature_reserve_m = curvatue_reserve;
            maximum_curvature_p = maximum_curvature;
            maximum_profile_thickness_xx = maximum_profile_thickness;
        }

        double maximum_profile_thickness(double x){
            double t = maximum_profile_thickness_xx/100; 
            //std::cout << "t: " << t << "\n";

            double thickness = 5*t*(0.2969*sqrt(x) - 0.1260*x - 0.3516*(x*x)+0.2843*pow(x,3) - 0.1015*pow(x,4)); //das is neg und dann vz bei upper u lower vertauscht
            //std::cout << "yt: " << yt << "\n";
            return thickness;
        }

        double mean_camber_fkt(double x){
            curvature_reserve_m = curvature_reserve_m/100;
            maximum_curvature_p = maximum_curvature_p/10;

            if(0 <= x && x <= maximum_curvature_p){
                double mean_camber = (2*maximum_curvature_p*x - pow(x,2))*curvature_reserve_m/pow(maximum_curvature_p,2);
                return mean_camber; 
            }
            else if(x > maximum_curvature_p){ //in wikipedia steht das es hier au gr gleich ist
                double mean_camber = (1 - 2*maximum_curvature_p + 2*maximum_curvature_p*x - pow(x,2))*curvature_reserve_m/pow(1 - maximum_curvature_p,2);
                return mean_camber;
            }
            else{
                std::cout << "x muss >= 0 sein";
                return 0;
            }
        }

        double theta(double x){
            curvature_reserve_m = curvature_reserve_m/100;
            maximum_curvature_p = maximum_curvature_p/10;

            if(curvature_reserve_m == 0){
                return 0;
            }
            else if(0 <= x && x <= maximum_curvature_p){ 
                double theta = (2*curvature_reserve_m/pow(maximum_curvature_p,2))*(maximum_curvature_p - x);
                std::cout << "theta1: " << theta << "\n";
                return atan(theta); 
            }
            else if(x > maximum_curvature_p){ //in wikipedia steht das es hier au gr gleich ist
                double theta = (2*curvature_reserve_m/pow(1 - maximum_curvature_p,2))*(maximum_curvature_p - x);
                std::cout << "theta: " << theta << "\n";
                return atan(theta); 
            }
            else{
                std::cout << "x muss >= 0 sein";
                return 0;
            }
        }

        double upper_y_fct(double x){
            double yt = maximum_profile_thickness(x);
            double yc = mean_camber_fkt(x);
            double angle = theta(x);
            double upper_y = yc + yt * cos(angle);
            return upper_y; 
        }

        double lower_y_fct(double x){
            double yt = maximum_profile_thickness(x);
            double yc = mean_camber_fkt(x);
            double angle = theta(x);
            double lower_y = yc - yt * cos(angle);
            return lower_y;
        }


    private:
        double  maximum_curvature_p, curvature_reserve_m, maximum_profile_thickness_xx;

};

/*int main(){
    NACA fourdigitNACA(0,0,12);
    double x_koord = 1;
    double result_upper_y = fourdigitNACA.upper_y_fct(x_koord);
    double result_lower_y = fourdigitNACA.lower_y_fct(x_koord);

    std::cout << "upper y: " << result_upper_y << "\n";
    std::cout << "lower y: " << result_lower_y << "\n";

    return 0;
}*/