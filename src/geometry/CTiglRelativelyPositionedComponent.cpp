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
CTiglRelativelyPositionedComponent::CTiglRelativelyPositionedComponent(MaybeOptionalPtr<std::string> parentUid, MaybeOptionalPtr<CCPACSTransformation> trans)
    : _parent(NULL), _parentUID(parentUid), _transformation(trans), _symmetryAxis(nullptr) {}

CTiglRelativelyPositionedComponent::CTiglRelativelyPositionedComponent(MaybeOptionalPtr<std::string> parentUid, MaybeOptionalPtr<CCPACSTransformation> trans, boost::optional<TiglSymmetryAxis>* symmetryAxis)
    : _parent(NULL), _parentUID(parentUid), _transformation(trans), _symmetryAxis(symmetryAxis){}

void CTiglRelativelyPositionedComponent::Reset() const
{
    CTiglAbstractGeometricComponent::Reset();
    if (GetTransform())
        const_cast<CCPACSTransformation&>(*GetTransform()).reset();
}

TiglSymmetryAxis CTiglRelativelyPositionedComponent::GetSymmetryAxis() const
{
    if (_symmetryAxis && _symmetryAxis->is_initialized() && _symmetryAxis->value() != TIGL_INHERIT_SYMMETRY)
        return **_symmetryAxis;
    else if (_parent)
        return _parent->GetSymmetryAxis();
    else
        return TIGL_NO_SYMMETRY;
}

void CTiglRelativelyPositionedComponent::SetSymmetryAxis(const TiglSymmetryAxis& axis)
{
    if (_symmetryAxis)
        *_symmetryAxis = axis;
    else
        throw CTiglError("Type does not have a symmetry");
}

CTiglTransformation CTiglRelativelyPositionedComponent::GetTransformationMatrix() const
{
    const CTiglTransformation thisTransformation = GetTransform() ? GetTransform()->getTransformationMatrix() : CTiglTransformation();
    if (_parent && GetTranslationType() == ABS_LOCAL) {
        const CTiglTransformation& parentTransformation = _parent->GetTransformationMatrix();

        // we only transform with the parent's translation
        CTiglTransformation parentTranslation;
        parentTranslation.AddTranslation(
            parentTransformation.GetValue(0, 3),
            parentTransformation.GetValue(1, 3),
            parentTransformation.GetValue(2, 3)
        );
        return parentTranslation * thisTransformation;
    }
    else
        return thisTransformation;
}

void CTiglRelativelyPositionedComponent::SetTransformation(const CCPACSTransformation &transform)
{
    if (!GetTransform()) {
        throw CTiglError("Cannot set Transformation for component \"" + GetDefaultedUID() + "\". The component has not transformation");
    }

    _transformation.Set(transform);

    // the component's geometry must be invalidated
    CTiglAbstractGeometricComponent::Reset();

}

CTiglPoint CTiglRelativelyPositionedComponent::GetRotation() const
{
    if (GetTransform())
        return GetTransform()->getRotation();
    else
        return CTiglPoint(0, 0, 0);
}

CTiglPoint CTiglRelativelyPositionedComponent::GetScaling() const
{
    if (GetTransform())
        return GetTransform()->getScaling();
    else
        return CTiglPoint(1, 1, 1);
}

CTiglPoint CTiglRelativelyPositionedComponent::GetTranslation() const
{
    if (GetTransform())
        return GetTransform()->getTranslationVector();
    else
        return CTiglPoint(0, 0, 0);
}

ECPACSTranslationType CTiglRelativelyPositionedComponent::GetTranslationType() const
{
    if (GetTransform())
        return GetTransform()->getTranslationType();
    else
        return ABS_GLOBAL; // TODO(bgruber): is this a valid default?
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
    return _parentUID.Get();
}

// Returns the transformation
boost::optional<const CCPACSTransformation&> CTiglRelativelyPositionedComponent::GetTransform() const
{
    return _transformation.Get();
}

// Sets the parent uid.
void CTiglRelativelyPositionedComponent::SetParentUID(const std::string& parentUID)
{
    _parentUID.Set(parentUID);
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

