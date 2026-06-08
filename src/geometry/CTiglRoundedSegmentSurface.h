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
    TIGL_EXPORT CTiglRoundedSegmentSurface(const  std::vector<Handle(Geom_BSplineCurve)> &m_profileCurves,
                                           double inner_rounding_distance,
                                           double outer_rounding_distance,
                                           int u_degree = 3,
                                           int v_degree = 3);

    TIGL_EXPORT  Handle(Geom_BSplineSurface) Surface();

private:
    TIGL_EXPORT void Perform();

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
            int count =1; //DEBUG DELETEME
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
                std::cerr << "Insert inner rows:" << count << std::endl; //DEBUG DELETEME
                count ++;
            }
        }

        void insert_outer_rows(int nb_dummies) {
            int count  =1; //DEBUG DELETEME
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
                std::cerr << "Insert outers rows:" << count << std::endl; //DEBUG DELETEME
                count ++;
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

    // Deduce required knot vector and multiplicies from pole matrix properties
    //TODO: Add mathematical boundaries for required number of knots and multiplicities
    TIGL_EXPORT void calculateKnotsAndMultiplicities();
    /*
     * @brief Creates a surface with rounded sections

        Steps:  Create Matrix that contains all poles of each profile curve and
                Dummy-Profile-Curves that contain poles that are required to build the curve in v-Direction
                TODO: Add mathematical description of the rounded segment curve building process
                - i-th row represents a profile in u-direction
                - j-th columnrepresents a curve in v-direction

                Steps:  Retrieve poles from inner and outer profile
                        Calculate distance between poles, apply inner/outer rounding distance relative to number of dummy profile
                        store new poles and original poles all in one matrix
    */

    // Call required methods to initialize required values and build surface
    TIGL_EXPORT void Invalidate() { _hasPerformed = false; }

private:
    // Rows represent profile curves (u-direction) and dummy curves, Columns define poles of curves in v-direction
    TColgp_HArray2OfPnt m_pole_matrix;

    TColStd_Array1OfReal m_u_knots;
    TColStd_Array1OfReal m_v_knots;

    TColStd_HArray1OfInteger m_u_multiplicities;
    TColStd_HArray1OfInteger m_v_multiplicities;

    //Storage of inner and outer rounding distance per segment
    double m_inner_rounding_distance;
    double m_outer_rounding_distance;

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
