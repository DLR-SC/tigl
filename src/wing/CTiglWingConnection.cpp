/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: CCPACSWingConnection.cpp 2698 2017-04-26 14:40:01Z bgruber $ 
*
* Version: $Revision: 2698 $
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
* @brief  Implementation of CPACS wing connection handling routines.
*/

#include <iostream>

#include "CTiglWingConnection.h"
#include "CTiglError.h"
#include "CCPACSWing.h"
#include "CCPACSEnginePylon.h"
#include "CCPACSWingSection.h"
#include "CCPACSWingSegment.h"
#include "CCPACSConfiguration.h"
#include "tiglcommonfunctions.h"

namespace tigl
{
CTiglWingConnection::CTiglWingConnection(CCPACSWingSegment* aSegment)
    : segment(aSegment)
    , m_resolved(*this, &CTiglWingConnection::resolve)
{
}

// Constructor
CTiglWingConnection::CTiglWingConnection(const std::string& elementUID, CCPACSWingSegment* aSegment)
    : elementUID(elementUID), segment(aSegment)
    , m_resolved(*this, &CTiglWingConnection::resolve)
{
}

// Returns the section UID of this connection
const std::string& CTiglWingConnection::GetSectionUID() const
{
    return *m_resolved->sectionUidPtr;
}

// Returns the section element UID of this connection
const std::string& CTiglWingConnection::GetSectionElementUID() const
{
    return elementUID;
}

// Returns the section index of this connection
int CTiglWingConnection::GetSectionIndex() const
{
    return m_resolved->sectionIndex;
}


// Returns the section element index of this connection
int CTiglWingConnection::GetSectionElementIndex() const
{
    return m_resolved->elementIndex;
}

// Returns the wing profile referenced by this connection
CCPACSWingProfile& CTiglWingConnection::GetProfile()
{
    return segment->GetUIDManager().ResolveObject<CCPACSWingProfile>(*m_resolved->profileUIDPtr);
}

const CCPACSWingProfile& CTiglWingConnection::GetProfile() const
{
    return segment->GetUIDManager().ResolveObject<CCPACSWingProfile>(*m_resolved->profileUIDPtr);
}

// Returns the positioning transformation (segment transformation) for the referenced section
CTiglTransformation CTiglWingConnection::GetPositioningTransformation() const
{
    // TODO: this is a bit ugly
    if (segment->GetParent()->IsParent<CCPACSWing>()) {
        return segment->GetParent()->GetParent<CCPACSWing>()->GetPositioningTransformation(GetSectionUID());
    }
    else if (segment->GetParent()->IsParent<CCPACSEnginePylon>()) {
        return segment->GetParent()->GetParent<CCPACSEnginePylon>()->GetPositioningTransformation(GetSectionUID());
    }
    else {
        throw CTiglError("Positioning cannot be queried for unknown type.");
    }

}

// Returns the section matrix referenced by this connection
CTiglTransformation CTiglWingConnection::GetSectionTransformation() const
{
    const CCPACSWingSectionElement& element = segment->GetUIDManager().ResolveObject<CCPACSWingSectionElement>(elementUID);
    return element.GetParent()->GetParent()->GetSectionTransformation();
}

// Returns the section element matrix referenced by this connection
CTiglTransformation CTiglWingConnection::GetSectionElementTransformation() const
{
    // TODO: what if the elementUID references an element outside of this wing

    const CCPACSWingSectionElement& element = segment->GetUIDManager().ResolveObject<CCPACSWingSectionElement>(elementUID);
    return element.GetSectionElementTransformation();
}

void CTiglWingConnection::SetElementUID(const std::string & uid)
{
    elementUID = uid;
    m_resolved.clear();
}

void CTiglWingConnection::resolve(ResolvedIndices& cache) const
{
    const CCPACSWingSectionElement& element = segment->GetUIDManager().ResolveObject<CCPACSWingSectionElement>(elementUID);
    size_t elementIndex = IndexFromUid(element.GetParent()->GetElements(), element.GetUID()) + 1;

    const CCPACSWingSection* section = element.GetParent()->GetParent();
    size_t sectionIndex = IndexFromUid(section->GetParent()->GetSections(), section->GetUID()) + 1;

    cache.sectionUidPtr = &section->GetUID();
    cache.sectionIndex = sectionIndex;
    cache.elementIndex = elementIndex;
    cache.profileUIDPtr = &element.GetAirfoilUID();
}

} // end namespace tigl
