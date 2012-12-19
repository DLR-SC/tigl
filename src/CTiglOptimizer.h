#include "tigl.h"


namespace tigl{

class CTiglOptimizer {
public:
    /**
    * @brief Optimizes the objective function f. 
    *
    * @param f (in) Objective function 
    * @param x (in/out) The function is initialized with the initial guess x and returns the solution x.
    */
    static TiglReturnCode optNewton2d(const class ITiglObjectiveFunction& f, double * x, double gradTol = 1e-5, double ofTol = 1e-5);

private:
    static double armijoBacktrack2d  (const class ITiglObjectiveFunction& f, const double * x, const double * grad, double * dir, double alpha, double& ofval);

};


}

