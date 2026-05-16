#include "CTiglRoundedSegmentSurface.h"
#include "CTiglError.h"


namespace tigl{
CTiglRoundedSegmentSurface::CTiglRoundedSegmentSurface(const std::vector<TopoDS_Wire>&, double inner_rounding_distance, double outer_rounding_distance) {
    Standard_Integer m = m_profileCurves.size()+(m_profileCurves.size()-2)*2*_nb_dummies;
    Standard_Integer n = m_profileCurves[0]->NbPoles();

    TColgp_HArray2OfPnt pole_matrix(1,m,1,n);
    //TODO What else to initialize?
}

//TODO
void CTiglRoundedSegmentSurface::ConvertCurves(){}
void CTiglRoundedSegmentSurface::buildLoft(){}

}

void NumberOfPolesIsSameInAllCurves(std::vector<Handle(Geom_BSplineCurve)> &profileCurves){
    //check if NbPoles is same in all profileCurves
    for(int i=0; i<profileCurves.size()-1; i++){
        if(!(profileCurves[i]->NbPoles()==profileCurves[i+1]->NbPoles())){
            throw tigl::CTiglError("ProfileCurves: Numbers of poles don't match");
        }
    }
}
