/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-08 Martin Siggel <Martin.Siggel@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
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
/**
* @file
* @brief  Implementation of an abstract class for phyical components (fuselage, wing, wheels etc...).
*/

#include "CTiglRelativelyPositionedComponent.h"
#include "CTiglError.h"
#include "CTiglIntersectionCalculation.h"
#include "CCPACSConfiguration.h"

#include "TCollection_ExtendedString.hxx"
#include "TCollection_HAsciiString.hxx"
#include "TopExp_Explorer.hxx"


namespace tigl
{
CTiglRelativelyPositionedComponent::CTiglRelativelyPositionedComponent(boost::optional<std::string>* parentUid, CCPACSTransformation* trans)
    : _transformation(trans), _symmetryAxis(static_cast<TiglSymmetryAxis*>(NULL)), _parentUID(parentUid), _parent(NULL) {}

CTiglRelativelyPositionedComponent::CTiglRelativelyPositionedComponent(boost::optional<std::string>* parentUid, CCPACSTransformation* trans, TiglSymmetryAxis* symmetryAxis)
    : _transformation(trans), _symmetryAxis(symmetryAxis), _parentUID(parentUid), _parent(NULL) {}

CTiglRelativelyPositionedComponent::CTiglRelativelyPositionedComponent(boost::optional<std::string>* parentUid, CCPACSTransformation* trans, boost::optional<TiglSymmetryAxis>* symmetryAxis)
    : _transformation(trans), _symmetryAxis(symmetryAxis), _parentUID(parentUid), _parent(NULL) {}


void CTiglRelativelyPositionedComponent::Reset()
{
    CTiglAbstractGeometricComponent::Reset();
    if (_transformation)
        _transformation->reset();
}

namespace {
    struct GetSymmetryVisitor : boost::static_visitor<TiglSymmetryAxis> {
        GetSymmetryVisitor(CTiglRelativelyPositionedComponent* parent)
            : _parent(parent) {}

        TiglSymmetryAxis operator()(const TiglSymmetryAxis* s) {
            if (s)
                return *s;
            else if (_parent)
                return _parent->GetSymmetryAxis();
            else
                return ENUM_VALUE(TiglSymmetryAxis, TIGL_NO_SYMMETRY);
        }
        TiglSymmetryAxis operator()(const boost::optional<TiglSymmetryAxis>* s) {
            if (s && *s)
                return **s;
            else if (_parent)
                return _parent->GetSymmetryAxis();
            else
                return ENUM_VALUE(TiglSymmetryAxis, TIGL_NO_SYMMETRY);
        }

    private:
        CTiglRelativelyPositionedComponent* _parent;
    };
}

TiglSymmetryAxis CTiglRelativelyPositionedComponent::GetSymmetryAxis() const {
    GetSymmetryVisitor visitor(_parent);
    return _symmetryAxis.apply_visitor(visitor);
}

namespace {
    struct SetSymmetryVisitor : boost::static_visitor<> {
        SetSymmetryVisitor(const TiglSymmetryAxis& axis)
            : axis(axis) {}
        void operator()(TiglSymmetryAxis* s) {
            if (s)
                *s = axis;
            else
                throw CTiglError("Type does not have a symmetry");
        }
        void operator()(boost::optional<TiglSymmetryAxis>* s) {
            if (s)
                *s = axis;
            else
                throw CTiglError("Type does not have a symmetry");
        }
    private:
        const TiglSymmetryAxis& axis;
    };
}

void CTiglRelativelyPositionedComponent::SetSymmetryAxis(const TiglSymmetryAxis& axis) {
    SetSymmetryVisitor visitor(axis);
    _symmetryAxis.apply_visitor(visitor);
}

CTiglTransformation CTiglRelativelyPositionedComponent::GetTransformation() const
{
    if (_transformation)
        return _transformation->getTransformationMatrix();
    else
        return CTiglTransformation();
}

CTiglPoint CTiglRelativelyPositionedComponent::GetRotation() const
{
    if (_transformation)
        return _transformation->getRotation();
    else
        return CTiglPoint(0, 0, 0);
}

CTiglPoint CTiglRelativelyPositionedComponent::GetScaling() const
{
    if (_transformation)
        return _transformation->getScaling();
    else
        return CTiglPoint(1, 1, 1);
}

CTiglPoint CTiglRelativelyPositionedComponent::GetTranslation() const
{
    if (_transformation)
        return _transformation->getTranslationVector();
    else
        return CTiglPoint(0, 0, 0);
}

ECPACSTranslationType CTiglRelativelyPositionedComponent::GetTranslationType() const
{
    if (_transformation)
        return _transformation->getTranslationType();
    else
        return ENUM_VALUE(ECPACSTranslationType, ABS_GLOBAL); // TODO(bgruber): is this a valid default?
}

void CTiglRelativelyPositionedComponent::Translate(CTiglPoint trans)
{
    if (_transformation) {
        _transformation->setTranslation(GetTranslation() + trans, GetTranslationType());
        _transformation->updateMatrix();
    }
    else
        throw CTiglError("Type does not have a _transformation");
}


// Returns a pointer to the list of children of a component.
CTiglRelativelyPositionedComponent::ChildContainerType CTiglRelativelyPositionedComponent::GetChildren(bool recursive)
{
    if (!recursive) {
        return _children;
    }
    else {
        ChildContainerType allChildsWithChilds;
        for (ChildContainerType::iterator it = _children.begin(); it != _children.end(); ++it) {
            allChildsWithChilds.push_back(*it);
            const ChildContainerType& childsOfChild = (*it)->GetChildren(true);
            allChildsWithChilds.insert(allChildsWithChilds.end(), childsOfChild.begin(), childsOfChild.end());
        }
        return allChildsWithChilds;
    }
}

// Returns the parent unique id
boost::optional<const std::string&> CTiglRelativelyPositionedComponent::GetParentUID() const
{
    if (!_parentUID || !*_parentUID)
        return boost::optional<const std::string&>();
    return **_parentUID;
}

// Sets the parent uid.
void CTiglRelativelyPositionedComponent::SetParentUID(const std::string& parentUID)
{
    if (!_parentUID)
        throw CTiglError("Derived type does not have a parentUID field");
    **_parentUID = parentUID;
}

void CTiglRelativelyPositionedComponent::SetParent(CTiglRelativelyPositionedComponent& parent)
{
    _parent = &parent;
}

// Adds a child to this geometric component.
void CTiglRelativelyPositionedComponent::AddChild(CTiglRelativelyPositionedComponent& child)
{
    _children.push_back(&child);
}

void CTiglRelativelyPositionedComponent::ClearChildren()
{
    _children.clear();
}


} // namespace tigl

