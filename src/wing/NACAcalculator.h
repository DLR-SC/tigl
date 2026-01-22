#ifndef NACACALCULATOR_H
#define NACACALCULATOR_H

#include <iostream>
#include <cmath>

class NACA{

    public:
        
        NACA(double curvatue_reserve, double maximum_curvature, double maximum_profile_thickness);

        double maximum_profile_thickness(double x);
        double mean_camber_fkt(double x);
        double theta(double x);

        double upper_y_fct(double x);

        double lower_y_fct(double x);

    private:
        double  maximum_curvature_p, curvature_reserve_m, maximum_profile_thickness_xx;

};
#endif //NACACALCULATOR_H