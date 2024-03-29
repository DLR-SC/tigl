// Copyright (c) 2020 RISC Software GmbH
//
// This file was generated by CPACSGen from CPACS XML Schema (c) German Aerospace Center (DLR/SC).
// Do not edit, all changes are lost when files are re-generated.
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <CCPACSCurveParamPointMap.h>
#include <CCPACSStringVector.h>
#include <string>
#include <tixi.h>
#include "CreateIfNotExists.h"
#include "tigl_internal.h"

namespace tigl
{
class CTiglUIDObject;

namespace generated
{
    class CPACSProfileGeometry;

    // This class is used in:
    // CPACSProfileGeometry

    /// @brief A curve that interpolates a list of points.
    /// 
    /// The curve interpolates the list of points, typically with a b-spline.
    /// In theory, the interpolation is somewhat ambiguous as it is not defined at which
    /// curve parameter a point will be interpolated.
    /// To solve is ambiguity, an optional parameter map can be defined
    /// that maps point indices with curve parameters.
    /// Kinks can also be modeled by populating the "kinks" array with the
    /// indices of points that should be on a kink. As an example, look at the following image:
    /// @see curve_point_list_xyz
    /// In this example, the kinks array will be "3;7".
    /// Optionally, the parameters of the kinks can be set in the parameter map.
    /// The whole profile looks as follows:
    /// 
    /// <pointList>
    /// <x>...</x>
    /// <y>...</y>
    /// <z>...</z>
    /// <kinks>3;7</kinks>
    /// <parameterMap>
    /// <pointIndex>3;5;7</pointIndex>
    /// <paramOnCurve>0.2;0.5;0.8</paramOnCurve>
    /// </parameterMap>
    /// </pointList>
    /// 
    class CPACSCurvePointListXYZ
    {
    public:
        TIGL_EXPORT CPACSCurvePointListXYZ(CPACSProfileGeometry* parent);

        TIGL_EXPORT virtual ~CPACSCurvePointListXYZ();

        TIGL_EXPORT CPACSProfileGeometry* GetParent();

        TIGL_EXPORT const CPACSProfileGeometry* GetParent() const;

        TIGL_EXPORT virtual CTiglUIDObject* GetNextUIDParent();
        TIGL_EXPORT virtual const CTiglUIDObject* GetNextUIDParent() const;

        TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
        TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;

        TIGL_EXPORT virtual const CCPACSStringVector& GetX() const;
        TIGL_EXPORT virtual CCPACSStringVector& GetX();

        TIGL_EXPORT virtual const CCPACSStringVector& GetY() const;
        TIGL_EXPORT virtual CCPACSStringVector& GetY();

        TIGL_EXPORT virtual const CCPACSStringVector& GetZ() const;
        TIGL_EXPORT virtual CCPACSStringVector& GetZ();

        TIGL_EXPORT virtual const boost::optional<CCPACSStringVector>& GetKinkIndices() const;
        TIGL_EXPORT virtual boost::optional<CCPACSStringVector>& GetKinkIndices();

        TIGL_EXPORT virtual const boost::optional<CCPACSCurveParamPointMap>& GetParameterMap() const;
        TIGL_EXPORT virtual boost::optional<CCPACSCurveParamPointMap>& GetParameterMap();

        TIGL_EXPORT virtual CCPACSStringVector& GetKinkIndices(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveKinkIndices();

        TIGL_EXPORT virtual CCPACSCurveParamPointMap& GetParameterMap(CreateIfNotExistsTag);
        TIGL_EXPORT virtual void RemoveParameterMap();

    protected:
        CPACSProfileGeometry* m_parent;

        /// Vector of x coordinates
        CCPACSStringVector                        m_x;

        /// Vector of y coordinates
        CCPACSStringVector                        m_y;

        /// Vector of z coordinates
        CCPACSStringVector                        m_z;

        /// Indices of points at which the curve has a kink. Each index is in the range [1, npoints].
        boost::optional<CCPACSStringVector>       m_kinkIndices;

        /// Map between point index and curve parameter.
        boost::optional<CCPACSCurveParamPointMap> m_parameterMap;

    private:
        CPACSCurvePointListXYZ(const CPACSCurvePointListXYZ&) = delete;
        CPACSCurvePointListXYZ& operator=(const CPACSCurvePointListXYZ&) = delete;

        CPACSCurvePointListXYZ(CPACSCurvePointListXYZ&&) = delete;
        CPACSCurvePointListXYZ& operator=(CPACSCurvePointListXYZ&&) = delete;
    };
} // namespace generated

// CPACSCurvePointListXYZ is customized, use type CCPACSCurvePointListXYZ directly

// Aliases in tigl namespace
using CCPACSProfileGeometry = generated::CPACSProfileGeometry;
} // namespace tigl
