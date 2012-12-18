#ifndef ITIGL_OBJECTIVE_FUNCTION
#define ITIGL_OBJECTIVE_FUNCTION

#define TIGL_MATRIX2D(A,n,i,j) ((A)[(i) + (j)*(n)])

namespace tigl {

class ITiglObjectiveFunction
{
public:
    ITiglObjectiveFunction(void);
    virtual ~ITiglObjectiveFunction(void);

    // set step size h for finite differences
    void setNumericalStepSize(double h);

    virtual int getParameterCount() const = 0;

    virtual bool hasAnalyticGradient() const = 0;
    virtual bool hasAnalyticHessian()  const = 0;

    /// returns the objective function for the parameter vector x
    virtual double getFunctionValue     (const double * x) const = 0;
    
    /// returns the gradient dx for the parameter vector x
    virtual void   getGradient (const double * x, double * dx) const;

    /// provides a routine for the numerical gradient calculation
    virtual void   getNumericalGradient(const double * x, double * dx) const;

    /// returns the hessian matrix for the parameter vector x
    virtual void   getHessian  (const double * x, double * H) const;

    /// provides a routine for the numerical hessian calculation
    virtual void   getNumericalHessian(const double * x, double * H) const;

    /// Sometimes it is more efficient to calculate gradient and hessian at once
    /// for these cases this function is provided.
    /// In the default implementation, it calls getGradient and getHessian succesively
    virtual void   getGradientHessian(const double * x, double * dx, double * H) const;

private:
    double h;


};


} // end namespace tigl

#endif // ITIGL_OBJECTIVE_FUNCTION