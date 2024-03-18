/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
/**
* @file
* @brief  Definition of the interface which describes a geometric component.
*/

#pragma once

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <map>
#include <vector>
#include <string>

#include "tigl.h"
#include "tigl_internal.h"
#include "tigl_config.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CCPACSTransformation.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"
#include "ECPACSTranslationType.h"
#include "CTiglError.h"

/**
 * @brief The MaybeOptionalPtr class is a wrapper for a boost::variant over a
 * T* or a a boost::optional<T>*. It provides a getter returning a
 * boost::optional<const T&> and a setter. The setter throws an error if the
 * underlying value is invalid (either a nullptr or an empty optional).
 *
 * The main purpose of this class is to avoid a large number of overloaded ctors
 * for CTiglRelativelyPositionedComponent: Some derived classes of
 * CTiglRelativelyPositionedComponent can have either a mandatory parentUID or an
 * optional parentUID as well as either a mandatory transformation or an optional
 * transformation.
 */
template <typename T>
class MaybeOptionalPtr : public boost::variant<T*, boost::optional<T>*>
{
public:
    explicit MaybeOptionalPtr() = default;
    MaybeOptionalPtr(T* t) : boost::variant<T*, boost::optional<T>*>(t) {}
    MaybeOptionalPtr(boost::optional<T>* t) : boost::variant<T*, boost::optional<T>*>(t) {}

    boost::optional<const T&> Get() const
    {
        struct Visitor : boost::static_visitor<boost::optional<const T&> > {
            Visitor() {}

            boost::optional<const T&> operator()(const T* value) {
                if (!value) {
                    return boost::optional<const T&>();
                }
                else {
                    return boost::optional<const T&>(*value);
                }
            }

            boost::optional<const T&> operator()(const boost::optional<T>* value) {
                if (!value || !*value)
                    return boost::optional<const T&>();
                return **value;
            }
        };

        Visitor v;
        return this->apply_visitor(v);
    }


    void Set(const T& value)
    {
        struct Visitor : boost::static_visitor<> {
            Visitor(const T& v)
                : m_value(v) {}
            void operator()(T* p) {
                if (p)
                    *p = m_value;
                else
                    throw tigl::CTiglError("Cannot set an invalid MaybeOptionalPtr.");
            }
            void operator()(boost::optional<T>* p) {
                if (p)
                    **p = m_value;
                else
                    throw tigl::CTiglError("Cannot set an invalid MaybeOptionalPtr");
            }
            const T& m_value;
        };

        Visitor v(value);
        this->apply_visitor(v);
    }
};

namespace tigl
{
class CTiglUIDManager;

// Base class for all CPACS objects which can be placed relatively to another one
class CTiglRelativelyPositionedComponent : public CTiglAbstractGeometricComponent
{
public:
    typedef std::vector<CTiglRelativelyPositionedComponent*> ChildContainerType;

    TIGL_EXPORT explicit CTiglRelativelyPositionedComponent(MaybeOptionalPtr<std::string> parentUid, MaybeOptionalPtr<CCPACSTransformation> trans);
    TIGL_EXPORT explicit CTiglRelativelyPositionedComponent(MaybeOptionalPtr<std::string> parentUid, MaybeOptionalPtr<CCPACSTransformation> trans, boost::optional<TiglSymmetryAxis>* symmetryAxis);
    TIGL_EXPORT explicit CTiglRelativelyPositionedComponent(tigl::CTiglRelativelyPositionedComponent *parent, MaybeOptionalPtr<CCPACSTransformation> trans);
    TIGL_EXPORT explicit CTiglRelativelyPositionedComponent(tigl::CTiglRelativelyPositionedComponent *parent, MaybeOptionalPtr<CCPACSTransformation> trans, boost::optional<TiglSymmetryAxis>* symmetryAxis);

    TIGL_EXPORT void Reset() const;

    TIGL_EXPORT TiglSymmetryAxis GetSymmetryAxis() const override; // resolves to parent components if no symmetry is available
    TIGL_EXPORT virtual void SetSymmetryAxis(const TiglSymmetryAxis& axis);

    TIGL_EXPORT virtual CTiglTransformation GetTransformationMatrix() const;

    TIGL_EXPORT virtual void SetTransformation(const CCPACSTransformation& transform);

    TIGL_EXPORT virtual CTiglPoint GetRotation() const;
    TIGL_EXPORT virtual CTiglPoint GetScaling() const;
    TIGL_EXPORT virtual CTiglPoint GetTranslation() const;
    TIGL_EXPORT virtual ECPACSTranslationType GetTranslationType() const;

    TIGL_EXPORT boost::optional<const std::string&> GetParentUID() const;
    TIGL_EXPORT void SetParentUID(const std::string& parentUID);

    TIGL_EXPORT ChildContainerType GetChildren(bool recursive);

protected:
    // Define a std::map to store the indices of already fused segments
    typedef std::map<int, int> FusedElementsContainerType;

private:
    friend class CTiglUIDManager;

    TIGL_EXPORT boost::optional<const CCPACSTransformation&> GetTransform() const;
    TIGL_EXPORT void SetParent(CTiglRelativelyPositionedComponent& parent);
    TIGL_EXPORT void AddChild(CTiglRelativelyPositionedComponent& child);
    TIGL_EXPORT void ClearChildren();

private:
    CTiglRelativelyPositionedComponent* _parent;
    ChildContainerType _children;
    MaybeOptionalPtr<std::string> _parentUID; ///< UID of the parent of this component, if supported by derived type

    MaybeOptionalPtr<CCPACSTransformation> _transformation;            // references down to the transformation of the derived class (may be empty in case derived class does not have transformation)
    boost::optional<TiglSymmetryAxis>* _symmetryAxis;   // references down to the symmetryAxis of the derived class (may be empty in case derived class does not have symmetry)
};

} // end namespace tigl
