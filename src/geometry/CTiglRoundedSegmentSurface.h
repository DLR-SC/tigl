#ifndef CTIGLROUNDEDSEGMENTSURFACE_H
#define CTIGLROUNDEDSEGMENTSURFACE_H

#include "Geom_BSplineCurve.hxx"
#include "Geom_BSplineSurface.hxx"
#include "tigl_internal.h"
#include <vector>

namespace tigl {

class CTiglRoundedSegmentSurface
{
public:
    /**
     * @brief CTiglRoundedSegmentSurface
     * Surface skinning algorithm to create a loft for a given set of
     * profile curves with rounded sections.
     * @param m_profileCurves
     * @param inner_rounding_distance Vector of numbers between  0.< 1. per section
     * with a size of m_profileCurves.size()-2
     * (outer section has no inner rounding distance)
     * @param outer_rounding_distance Vector of numbers between  0.< 1. per section
     * with a size of m_profileCurves.size()-2
     * (inner section has no outer rounding distance)
     */
    TIGL_EXPORT CTiglRoundedSegmentSurface(const std::vector<Handle(Geom_Curve)> &m_profileCurves,
                                           const std::vector<double> &inner_rounding_distance,
                                           const std::vector<double> &outer_rounding_distance);

    TIGL_EXPORT  Handle(Geom_BSplineSurface) Surface();

private:

    TIGL_EXPORT void Perform();


    /**
     * @brief The RoundedSegment class
     * A Rounded Segment is a helper container for each segment of the surface to create.
     * It gets two profile curves and two rounding distances, and creates a vector for
     * the poles of each dummy-profile curve from this information.
     */
    struct RoundedSegment
    {
        TColgp_HArray1OfPnt firstProfile;
        TColgp_HArray1OfPnt lastProfile;
        std::vector<TColgp_HArray1OfPnt> dummyProfiles;
        double inner_rounding_distance;
        double outer_rounding_distance;

        RoundedSegment(Handle(Geom_BSplineCurve) const& start,
                       Handle(Geom_BSplineCurve) const& end,
                       double inner_rd,
                       double outer_rd):
            firstProfile(1,start->NbPoles()),
            lastProfile(1,start->NbPoles()),
            dummyProfiles({}),
            inner_rounding_distance(inner_rd),
            outer_rounding_distance(outer_rd)
        {
            for(int i = 1; i < start->NbPoles()+1; i++){
                firstProfile.SetValue(i,start->Pole(i));
                lastProfile.SetValue(i,end->Pole(i));
            }
            if(inner_rounding_distance>1e-6){
                insert_inner_rows(3);
            }
            if(outer_rounding_distance>1e-6){
                insert_outer_rows(3);
            }
        }

        void insert_inner_rows(int nb_dummies) {
            for (int i = 0; i < nb_dummies; i++) {
                auto row =  new TColgp_HArray1OfPnt(1, firstProfile.Size());
                // for every profile point in start profile
                for(int j = 1; j< firstProfile.Size()+1; j++){
                    //calculate Vector between both profile poles and normalize it
                    gp_Vec vector_in_v_direction(firstProfile.Value(j), lastProfile.Value(j)); //v01
                    gp_Vec normalized_vector_in_v_direction(vector_in_v_direction);
                    // interpolate relative position between the point in the start and end profiles for the ith dummy profile
                    // append interpolated point to row
                    double inner_distance = inner_rounding_distance/nb_dummies*(i+1);
                    gp_Vec inner_pole_vec(firstProfile.Value(j).Coord().X(),firstProfile.Value(j).Coord().Y(),firstProfile.Value(j).Coord().Z());
                    //save new poles in a vector for each inner dummy profile
                    gp_Vec inner_vec(normalized_vector_in_v_direction);
                    inner_vec.Scale(inner_distance); //scaled normalized vector in v-direction
                    inner_vec.Add(inner_pole_vec); //vector to new k-th pole
                    gp_Pnt new_pole_inner(inner_vec.XYZ());
                    row->SetValue(j, new_pole_inner);
                }
                dummyProfiles.push_back(*row);
            }
        }

        void insert_outer_rows(int nb_dummies) {
            for (int i = 0; i < nb_dummies; i++) {
                auto row =  new TColgp_HArray1OfPnt(1, firstProfile.Size());
                for (int j=1; j < firstProfile.Size()+1; j++){
                    //calculate Vector between both profile poles and normalize it
                    gp_Vec vector_in_v_direction(firstProfile.Value(j), lastProfile.Value(j)); //v01
                    gp_Vec normalized_vector_in_v_direction(vector_in_v_direction);
                    //calculate distance between k-th outer dummy-pole in v-direction and profile
                    double outer_distance = (outer_rounding_distance/nb_dummies) * (nb_dummies-i);
                    gp_Vec outer_vec(lastProfile.Value(j).Coord().X(),
                                     lastProfile.Value(j).Coord().Y(),
                                     lastProfile.Value(j).Coord().Z());
                    gp_Vec outer_normalized_vec(normalized_vector_in_v_direction);
                    outer_normalized_vec.Scale(outer_distance); //scaled normalized vector in v-direction
                    outer_vec.Subtract(outer_normalized_vec);
                    gp_Pnt new_pole_outer(outer_vec.XYZ());
                    //save new poles in a vector for each outer dummy profile
                    row->SetValue(j, new_pole_outer);
                }
                dummyProfiles.push_back(*row);
            }
        }

        TColgp_HArray1OfPnt get_first_profile(){
            return firstProfile;
        }
        TColgp_HArray1OfPnt get_last_profile(){
            return lastProfile;
        }
        std::vector<TColgp_HArray1OfPnt> getDummyProfiles(){
            return dummyProfiles;
        }


    };

    TIGL_EXPORT void Invalidate() { _hasPerformed = false; }

private:
    // Rows represent profile curves (u-direction) and dummy curves, Columns define poles of curves in v-direction
    TColgp_Array2OfPnt m_pole_matrix;

    TColStd_Array1OfReal m_u_knots;
    TColStd_Array1OfReal m_v_knots;

    TColStd_HArray1OfInteger m_u_multiplicities;
    TColStd_HArray1OfInteger m_v_multiplicities;

    //Storage of inner and outer rounding distance per segment
    std::vector<double> m_inner_rounding_distance;
    std::vector<double> m_outer_rounding_distance;

    // Store as B-spline curves internally (after conversion)
    std::vector<Handle(Geom_BSplineCurve)> m_profileCurves; //Vector contains 1 edge per profile Wire, loft ist build edgewise
    std::vector<RoundedSegment> m_segments;
    Handle(Geom_BSplineSurface) m_surface;

    bool _hasPerformed = false;
    int _maxDegree = 3;
    int _u_degree = 3;
    int _v_degree = 3;
    size_t _nb_dummies =3; //Rows per rounding distance


};

}
#endif // CTIGLROUNDEDSEGMENTSURFACE_H
