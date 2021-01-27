/*
* Copyright (C) 2021 German Aerospace Center (DLR/SC)
*
* Created: 2021-01-21 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#ifndef CTIGLRECTGRIDSURFACE_H
#define CTIGLRECTGRIDSURFACE_H

#include <type_traits>
#include <vector>

#include "tigl_internal.h"
#include "CTiglError.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <BRepTools.hxx>
#include <Geom_Surface.hxx>
#include <BRep_Tool.hxx>

namespace {
struct DefaultAnnotation
{};
}


namespace tigl
{

/**
 * @brief The CTiglRectGridSurface class represents
 * a collection of faces arranged in a rectangular
 * grid. Optionally, the faces can be annotated using
 * a custom type.
 */
template <typename Annotation = DefaultAnnotation>
class CTiglRectGridSurface
{
private:

    /**
     * @brief The AnnotatedFace class is used to
     * store
     *   a) the connectivity info of a single face
     *      in a rectangular grid of faces
     *   b) an optional Annotation
     */
    class AnnotatedFace
    {
        friend class CTiglRectGridSurface;
    public:
        template <typename... Args>
        AnnotatedFace(TopoDS_Face const& f, Args&&... args)
            : face{f}
            , annotation{std::forward<Args>(args)...}
        {
            BRepTools::UVBounds(face, umin, umax, vmin, vmax);
            Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
            surf->D0(umin, vmin, u0v0);
            surf->D0(umax, vmin, u1v0);
            surf->D0(umin, vmax, u0v1);
            surf->D0(umax, vmax, u1v1);
        }

        /**
         * @brief UNext returns the neighboring annotated face in positive u direction
         * @return the neighboring annotated face in positive u direction
         */
        AnnotatedFace*       UNext()       { return Pos_u_Neighbor; }
        AnnotatedFace const* UNext() const { return Pos_u_Neighbor; }

        /**
         * @brief UPrev returns the neighboring annotated face in negative u direction
         * @return the neighboring annotated face in positive u direction
         */
        AnnotatedFace*       UPrev()       { return Neg_u_Neighbor; }
        AnnotatedFace const* UPrev() const { return Neg_u_Neighbor; }

        /**
         * @brief VNext returns the neighboring annotated face in positive v direction
         * @return the neighboring annotated face in positive v direction
         */
        AnnotatedFace*       VNext()       { return Pos_v_Neighbor; }
        AnnotatedFace const* VNext() const { return Pos_v_Neighbor; }

        /**
         * @brief VPrev returns the neighboring annotated face in negative v direction
         * @return the neighboring annotated face in positive vu direction
         */
        AnnotatedFace*       VPrev()       { return Neg_v_Neighbor; }
        AnnotatedFace const* VPrev() const { return Neg_v_Neighbor; }


        /**
         * @brief GetFace returns the wrapped TopoDS_Face
         * @return the wrapped TopoDS_Face
         */
        TopoDS_Face       GetFace()       { return face; }
        TopoDS_Face const GetFace() const { return face; }

        /**
         * @brief SetFace replaces the wrapped TopoDS_Face with another face
         *
         * Note, that this possibly breaks the rectangular grid and the
         * u,v bounds are not updated. This is intentional and not checked internally.
         *
         * @param f the new TopoDS_Face
         *
         */
        void ReplaceFace(TopoDS_Face const& f) {
            face = f;
        }

        /**
         * @brief UMin returns the starting u parameter of this face
         * @return the starting u parameter of this face
         */
        double UMin() const { return umin; };

        /**
         * @brief UMax returns the ending u parameter of this face
         * @return the ending u parameter of this face
         */
        double UMax() const { return umax; };

        /**
         * @brief VMin returns the starting v parameter of this face
         * @return the starting v parameter of this face
         */
        double VMin() const { return vmin; };

        /**
         * @brief VMax returns the ending v parameter of this face
         * @return the ending v parameter of this face
         */
        double VMax() const { return vmax; };

        /**
         * @brief GetAnnotation returns the custom annotation associated with this face
         * @return the custom annotation associated with this face
         */
        Annotation&       GetAnnotation()       { return annotation; };
        Annotation const& GetAnnotation() const { return annotation; };

    private:
        TopoDS_Face face;
        gp_Pnt u0v0, u1v0, u0v1, u1v1; // corner points
        double umin, umax, vmin, vmax; // parametric bounds

        AnnotatedFace* Neg_u_Neighbor {nullptr};
        AnnotatedFace* Pos_u_Neighbor {nullptr};
        AnnotatedFace* Neg_v_Neighbor {nullptr};
        AnnotatedFace* Pos_v_Neighbor {nullptr};

        Annotation annotation;
    };

public:

    TIGL_EXPORT CTiglRectGridSurface(){}

    /**
     * @brief CTiglRectGridSurface constructs the surface
     * from a shape
     * @param shape a shape containing faces arranged in a rectangular
     * grid
     * @param args arguments to be forwarded to the Annotation class of each
     * face.
     */
    template <typename... Args>
    CTiglRectGridSurface(TopoDS_Shape const& shape, Args&&... args)
    {
        SetShape(shape, std::forward<Args>(args)...);
    }

    /**
     * @brief SetShape sets the shape represented by this instance
     * @param shape a shape containing faces arranged in a rectangular
     * @param args arguments to be forwarded to the Annotation class of each
     * face.
     */
    template <typename... Args>
    void SetShape(TopoDS_Shape const& shape, Args&&... args)
    {
        TopTools_IndexedMapOfShape faceMap;
        TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
        for (int i = 1; i <= faceMap.Extent(); i++) {
            TopoDS_Face current = TopoDS::Face(faceMap(i));
            face_infos.emplace_back(AnnotatedFace(current, std::forward<Args>(args)...));
        }
        connect_faces();
    }

    // CTiglRectGridSurface is move-only, because internally some pointers are stored.
    CTiglRectGridSurface(CTiglRectGridSurface const&) = delete;
    CTiglRectGridSurface& operator=(CTiglRectGridSurface const&) = delete;
    CTiglRectGridSurface(CTiglRectGridSurface&&) = default;
    CTiglRectGridSurface& operator=(CTiglRectGridSurface&&) = default;

    /**
     * @brief Root returns the root face of the rectangular grid, which is one of its corners
     * @return the root face of the rectangular grid, which is one of its corners
     */
    AnnotatedFace* Root() {
        assert(face_infos.size()>0);
        return &face_infos[root_idx];
    }

    AnnotatedFace const* Root() const {
        assert(face_infos.size()>0);
        return &face_infos[root_idx];
    }

    /**
     * @brief NRows number of rows
     * @return number of rows
     */
    size_t NRows() const { return nrows; };

    /**
     * @brief NCols number of columns
     * @return number of columns
     */
    size_t NCols() const { return ncols; };


private:

    void connect_faces()
    {
        for (auto& current : face_infos ) {
            for (auto& other : face_infos) {

                if (    (void*)&current == (void*)&other ) {
                    continue;
                }

                //  if other face has no negative u neighbor yet,
                // and the corner points match, register neighborship
                if (    other.Neg_u_Neighbor == nullptr
                     && current.u1v0.SquareDistance(other.u0v0) < m_sqr_dist_tol
                     && current.u1v1.SquareDistance(other.u0v1) < m_sqr_dist_tol ) {
                    current.Pos_u_Neighbor = &other;
                    other.Neg_u_Neighbor = &current;
                }
                if (    other.Neg_v_Neighbor == nullptr
                          && current.u0v1.SquareDistance(other.u0v0) < m_sqr_dist_tol
                          && current.u1v1.SquareDistance(other.u1v0) < m_sqr_dist_tol ) {
                    // do the same for the v direction
                    current.Pos_v_Neighbor = &other;
                    other.Neg_v_Neighbor = &current;
                }
            }
        }

        //Determine root
        for (root_idx=0; root_idx < face_infos.size(); ++root_idx ) {
            if( face_infos[root_idx].Neg_u_Neighbor == nullptr
                && face_infos[root_idx].Neg_v_Neighbor == nullptr ){
                break;
            }
        }

        /*
         * Determine number of rows and columns
         */
        ncols = 1;
        for(AnnotatedFace* current = Root(); current->UNext(); current = current->UNext(), ++ncols)
        {}
        nrows = 1;
        for(AnnotatedFace* current = Root(); current->VNext(); current = current->VNext(), ++nrows)
        {}

        //Check faces: Are they really a rectangular grid?
        for (auto& face : face_infos ) {
            if (!face.UPrev()){
                //every face without uprev should have ncols unext neighbors
                size_t ncols_tmp = 1;
                for(AnnotatedFace* current = &face; current->UNext(); current = current->UNext(), ++ncols_tmp)
                {}
                if (ncols_tmp != ncols ) {
                    throw CTiglError("The Input Shape is not a rectangular grid of faces");
                }
            }
            if (!face.VPrev()) {
                //every face without vprev should have nrows vnext neighbors
                size_t nrows_tmp = 1;
                for(AnnotatedFace* current = &face; current->VNext(); current = current->VNext(), ++nrows_tmp)
                {}
                if (nrows_tmp != nrows ) {
                    throw CTiglError("The Input Shape is not a rectangular grid of faces");
                }
            }
        }

    }


    std::vector<AnnotatedFace> face_infos;
    size_t root_idx {0};
    size_t nrows {0};
    size_t ncols {0};
    double m_sqr_dist_tol {1e-4};
};

} //namespace tgil

#endif
