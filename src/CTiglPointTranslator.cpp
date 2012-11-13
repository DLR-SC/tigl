#include "CTiglPointTranslator.h"
#include "tigl.h"

#define SQR(x) ((x)*(x))

#define DEBUG

namespace tigl{

namespace {
    inline double max(double a, double b) { return a > b ? a : b; }
}

CTiglPointTranslator::CTiglPointTranslator(const CTiglPoint& x1, const CTiglPoint& x2, const CTiglPoint& x3, const CTiglPoint& x4){
    a = x2-x1;
    b = x3-x1;
    c = x1-x2-x3+x4;
    d = x1;

    grad[0] = 0.; grad[1] = 0.;
    hess[0][0] = hess[0][1] = hess[1][0] = hess[1][1] = 0.;
}

void CTiglPointTranslator::calcP(double alpha, double beta, CTiglPoint& p){
    p.x = alpha*a.x + beta*b.x + alpha*beta*c.x + d.x - x.x;
    p.y = alpha*a.y + beta*b.y + alpha*beta*c.y + d.y - x.y;
    p.z = alpha*a.z + beta*b.z + alpha*beta*c.z + d.z - x.z;
}

double CTiglPointTranslator::calc_obj(double eta, double xsi){
    CTiglPoint p;
    calcP(eta,xsi, p);

    return p.norm2Sqr();
}

int CTiglPointTranslator::calc_grad_hess(double alpha, double beta){
    CTiglPoint p;
    calcP(alpha,beta, p);

    CTiglPoint acb = a + c*beta;
    CTiglPoint bca = b + c*alpha;

    grad[0] = 2*CTiglPoint::inner_prod(p, acb);
    grad[1] = 2*CTiglPoint::inner_prod(p, bca);

    hess[0][0] = 2*acb.norm2Sqr();
    hess[1][1] = 2*bca.norm2Sqr();
    hess[0][1] = 2*CTiglPoint::inner_prod(bca, acb) + 2*CTiglPoint::inner_prod(p, c);
    hess[1][0] = hess[0][1];

    return 0;
}

int CTiglPointTranslator::optimize(double& eta, double& xsi){
    eta = 0.;
    xsi = 0.;

    double of = calc_obj(eta,xsi);
    double of_old = of + 1;

    calc_grad_hess(eta, xsi);


    // iterate
    int iter      = 0;
    int numOfIter = 100;
    double prec = 1e-5;

    while ( iter < numOfIter && 
          (of_old - of)/max(abs(of),1) > prec && 
          sqrt(grad[0]*grad[0]+grad[1]*grad[1]) >prec
          ) 
    {
        // calc direction
        double det = hess[0][0]*hess[1][1] - hess[0][1]*hess[1][0];
        if ( abs(det) < 1e-12 ){
            std::cerr << "Error: Determinant too small in CTiglPointTranslator::optimize!" << std::endl;
            return 1;
        }

        // calculate inverse hessian
        double inv_hess[2][2];
        double invdet = 1./det;
        inv_hess[0][0] = invdet * ( hess[1][1] );
        inv_hess[1][1] = invdet * ( hess[0][0] );
        inv_hess[0][1] = -invdet * ( hess[0][1] );
        inv_hess[1][0] = -invdet * ( hess[1][0] );

        // calculate search direction
        double dir[2];
        dir[0] = - inv_hess[0][0]*grad[0] - inv_hess[0][1]*grad[1];
        dir[1] = - inv_hess[1][0]*grad[0] - inv_hess[1][1]*grad[1];


        double alpha = 1;
        eta += alpha*dir[0];
        xsi += alpha*dir[1];

#ifdef DEBUG
        std::cout << "Iteration=" << iter << " f=" << of << " norm(grad)=" << grad[0]*grad[0]+grad[1]*grad[1] << std::endl;
#endif

        // calculate new values
        of_old = of;
        of = calc_obj(eta,xsi);
        calc_grad_hess(eta,xsi);

        iter++;
    }

}

TiglReturnCode CTiglPointTranslator::translate(const CTiglPoint& xx, double* eta, double * xsi){
    if(!eta || ! xsi){
        std::cerr << "Error in CTiglPointTranslator::translate(): eta and xsi may not be NULL Pointers!" << std::endl;
        return TIGL_NULL_POINTER;
    }

    grad[0] = 0.; grad[1] = 0.;
    hess[0][0] = hess[0][1] = hess[1][0] = hess[1][1] = 0.;

    this->x = xx;
    optimize(*eta, *xsi);
} 

} // end namespace tigl
