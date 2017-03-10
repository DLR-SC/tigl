/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Definition of the interface which describes a geometric component.
*/

#ifndef CTIGLABSTRACTPHYISICALCOMPONENT_H
#define CTIGLABSTRACTPHYISICALCOMPONENT_H

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <map>
#include <vector>
#include <string>

#include "tigl.h"
#include "tigl_internal.h"
#include "tigl_config.h"
#include "CTiglAbstractGeometricComponent.h"
#include "CTiglTransformation.h"
#include "CTiglPoint.h"
#include "ECPACSTranslationType.h"

namespace tigl
{
class CTiglUIDManager;

// Base class for all CPACS objects which can be placed relatively to another one
class CTiglRelativeComponent : public CTiglAbstractGeometricComponent
{
public:
    typedef std::vector<CTiglRelativeComponent*> ChildContainerType;

    TIGL_EXPORT CTiglRelativeComponent(boost::optional<std::string>* parentUid, CCPACSTransformation* trans);
    TIGL_EXPORT CTiglRelativeComponent(boost::optional<std::string>* parentUid, CCPACSTransformation* trans, TiglSymmetryAxis* symmetryAxis);
    TIGL_EXPORT CTiglRelativeComponent(boost::optional<std::string>* parentUid, CCPACSTransformation* trans, boost::optional<TiglSymmetryAxis>* symmetryAxis);

    TIGL_EXPORT void Reset();

    TIGL_EXPORT virtual TiglSymmetryAxis GetSymmetryAxis() const OVERRIDE; // resolves to parent components if no symmetry is available
    TIGL_EXPORT virtual void SetSymmetryAxis(const TiglSymmetryAxis& axis);

    TIGL_EXPORT virtual CTiglTransformation GetTransformation() const;

    TIGL_EXPORT virtual CTiglPoint GetRotation() const;
    TIGL_EXPORT virtual CTiglPoint GetScaling() const;
    TIGL_EXPORT virtual CTiglPoint GetTranslation() const;
    TIGL_EXPORT virtual ECPACSTranslationType GetTranslationType() const;

    TIGL_EXPORT virtual void Translate(CTiglPoint trans);

    TIGL_EXPORT boost::optional<const std::string&> GetParentUID() const;
    TIGL_EXPORT void SetParentUID(const std::string& parentUID);

    TIGL_EXPORT ChildContainerType GetChildren(bool recursive);

protected:
    // Define a std::map to store the indices of already fused segments
    typedef std::map<int, int> FusedElementsContainerType;

private:
    friend class CTiglUIDManager;
    TIGL_EXPORT void SetParent(CTiglRelativeComponent& parent);
    TIGL_EXPORT void AddChild(CTiglRelativeComponent& child);
    TIGL_EXPORT void ClearChildren();

private:
    CTiglRelativeComponent* _parent;
    ChildContainerType _children;
    boost::optional<std::string>* _parentUID; ///< UID of the parent of this component, if supported by derived type

    CCPACSTransformation* _transformation;                                                 // references down to the transformation of the derived class (may be empty in case derived class does not have transformation)
    boost::variant<TiglSymmetryAxis*, boost::optional<TiglSymmetryAxis>*> _symmetryAxis;   // references down to the symmetryAxis of the derived class (may be empty in case derived class does not have symmetry)
};

} // end namespace tigl

#endif // CTIGLABSTRACTPHYISICALCOMPONENT_H
