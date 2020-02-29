/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
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


#include "CTiglTopoAlgorithms.h"

#include "CTiglBSplineAlgorithms.h"
#include "CTiglError.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS_Shell.hxx>
#include <BRep_Builder.hxx>
#include <TopoDS.hxx>
#include <Geom_Surface.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include <algorithm>

namespace
{
    void CutSurfaceAtUVParametersImpl(Handle(Geom_Surface) surface, std::vector<double> uparams, std::vector<double> vparams, TopoDS_Shell& shell)
    {
        if (surface.IsNull()) {
            return;
        }

        std::sort(std::begin(uparams), std::end(uparams));
        std::sort(std::begin(vparams), std::end(vparams));

        Standard_Real u1, u2, v1, v2;
        surface->Bounds(u1, u2, v1, v2);

        const double tol=1e-6;

        if (uparams.empty()) {
            uparams = {u1, u2};
        }
        else {
            std::remove_if(std::begin(uparams), std::end(uparams), [u1, u2](double par) {
               return par < u1 || par > u2;
            });

            if (fabs(u1 - uparams.front()) < tol) {
                uparams.front() = u1;
            }
            if (fabs(u2 - uparams.back()) < tol) {
                uparams.back() = u2;
            }
        }

        if (vparams.empty()) {
            vparams = {v1, v2};
        }
        else {
            std::remove_if(std::begin(vparams), std::end(vparams), [v1, v2](double par) {
               return par < v1 || par > v2;
            });

            if (fabs(v1 - vparams.front()) < tol) {
                vparams.front() = v1;
            }
            if (fabs(v2 - vparams.back()) < tol) {
                vparams.back() = v2;
            }
        }

        BRep_Builder builder;

        for (size_t vidx = 1; vidx < vparams.size(); ++vidx) {
            for (size_t uidx = 1; uidx < uparams.size(); ++uidx) {
                auto trimmedSurface = tigl::CTiglBSplineAlgorithms::trimSurface(surface, uparams[uidx-1], uparams[uidx], vparams[vidx-1], vparams[vidx]);
                BRepBuilderAPI_MakeFace faceMaker(trimmedSurface, 1e-10);
                builder.Add(shell, faceMaker.Face());
            }
        }

    }
}

namespace tigl
{

TopoDS_Shape CTiglTopoAlgorithms::CutShellAtUVParameters(TopoDS_Shape const& shape, std::vector<double> uparams, std::vector<double> vparams)
{

    bool cutInUDirection = (uparams.size() > 0);
    bool cutInVDirection = (vparams.size() > 0);

    if ( !cutInUDirection && !cutInVDirection ) {
        //nothing to do
        return shape;
    }

    // sort parameter vectors if they are not sorted
    if (cutInUDirection && !std::is_sorted(uparams.begin(), uparams.end()) ) {
        std::sort(uparams.begin(), uparams.end());
    }
    if (cutInVDirection && !std::is_sorted(vparams.begin(), vparams.end()) ) {
        std::sort(vparams.begin(), vparams.end());
    }

    TopoDS_Shell cutShape;
    BRep_Builder builder;
    builder.MakeShell(cutShape);

    for (TopExp_Explorer faces(shape, TopAbs_FACE); faces.More(); faces.Next()) {
        // trim each face/surface of the compound at the uv paramters in the paramter vectors
        auto surface = BRep_Tool::Surface(TopoDS::Face(faces.Current()));
        CutSurfaceAtUVParametersImpl(surface, uparams, vparams, cutShape);
    }
    return std::move(cutShape);
}

TopoDS_Shell CTiglTopoAlgorithms::CutSurfaceAtUVParameters(Handle(Geom_Surface) surface, std::vector<double> uparams, std::vector<double> vparams)
{
    if (!surface) {
        throw CTiglError("Null pointer surface in CTiglTopoAlgorithms::CutSurfaceAtUVParameters", TIGL_NULL_POINTER);
    }

    BRep_Builder builder;
    TopoDS_Shell shell;
    builder.MakeShell(shell);
    CutSurfaceAtUVParametersImpl(surface, uparams, vparams, shell);
    return shell;
}

TopoDS_Shell CTiglTopoAlgorithms::CutSurfaceAtKinks(Handle(Geom_BSplineSurface) surface)
{
    if (!surface) {
        throw CTiglError("Null pointer surface in CTiglTopoAlgorithms::CutSurfaceAtKinks", TIGL_NULL_POINTER);
    }

    auto kinks = CTiglBSplineAlgorithms::getKinkParameters(surface);

    // Include surface boundaries
    double u1, u2, v1, v2;
    surface->Bounds(u1, u2, v1, v2);
    kinks.u.insert(kinks.u.begin(), u1);
    kinks.u.push_back(u2);
    kinks.v.insert(kinks.v.begin(), v1);
    kinks.v.push_back(v2);

    return CutSurfaceAtUVParameters(surface, kinks.u, kinks.v);
}

} // namespace tigl
