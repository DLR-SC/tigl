/*
* Copyright (C) 2007-2022 German Aerospace Center (DLR/SC)
*
* Created: 2022-03-24 Jan Kleinert <Jan.Kleinert@dlr.de>
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
* @brief  Implementation of CPACS duct handling routines.
*/

//#define USE_TIGL_FUSER

#include "CCPACSDuctAssembly.h"
#include "CCPACSDuct.h"
#include "Debugging.h"
#include "CCutShape.h"
#include "CNamedShape.h"
#include "CTiglError.h"

#ifdef USE_TIGL_FUSER
#include "CFuseShapes.h"
#else
#include "TopTools_ListOfShape.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "CBooleanOperTools.h"
#endif



namespace tigl {

CCPACSDuctAssembly::CCPACSDuctAssembly(CCPACSDucts* parent, CTiglUIDManager* uidMgr)
  : generated::CPACSDuctAssembly(parent, uidMgr)
  , CTiglRelativelyPositionedComponent(&m_parentUID, &m_transformation)
{
    for (auto const& uid: m_ducts.GetUIDs()) {
        auto& duct = GetParent()->GetDuct(uid);
        duct.RegisterInvalidationCallback([&](){ this->Invalidate(); });
    }
}

std::string CCPACSDuctAssembly::GetDefaultedUID() const
{
    return generated::CPACSDuctAssembly::GetUID();
}

TiglGeometricComponentType CCPACSDuctAssembly::GetComponentType() const
{
    return TIGL_COMPONENT_DUCT;
}

TiglGeometricComponentIntent CCPACSDuctAssembly::GetComponentIntent() const
{
    // needs to be physical, so that transformation relative to parent works
    return TIGL_INTENT_PHYSICAL;
}

void CCPACSDuctAssembly::InvalidateImpl(const boost::optional<std::string>&) const
{
    CTiglAbstractGeometricComponent::Reset();
}

PNamedShape CCPACSDuctAssembly::BuildLoft() const
{
    if (m_ducts.GetUIDs().size() == 0) {
        return PNamedShape();
    }

    if (m_ducts.GetUIDs().size() == 1 && !GetParent()->GetDuct(m_ducts.GetUIDs()[0]).GetMirroredLoft()) {
        // no need to fuse, there is just one solid
        return GetParent()->GetDuct(m_ducts.GetUIDs()[0]).GetLoft();
    }
#ifdef USE_TIGL_FUSER
    // first fuse all ducts to one solid tool
    PNamedShape parentDuct = nullptr;
    ListPNamedShape childDucts;
    for (auto const& duct: m_ducts) {
        if (!parentDuct) {
            parentDuct = duct->GetLoft();
        }
        else {
            childDucts.push_back(duct->GetLoft());
        }

        // add mirrored loft, if any
        PNamedShape mirroredLoft = duct->GetMirroredLoft();
        if (mirroredLoft) {
            childDucts.push_back(mirroredLoft);
        }

    }
    tool = CFuseShapes(parentDuct, childDucts);
#else
    TopTools_ListOfShape arguments, tools;
    for (auto const& duct_uid: m_ducts.GetUIDs()) {

        auto& duct = GetParent()->GetDuct(duct_uid);

        PNamedShape loft = duct.GetLoft();
        if (arguments.Extent() == 0) {
            arguments.Append(loft->Shape());
        }
        else {
            tools.Append(loft->Shape());
        }

        // add mirrored loft, if any
        PNamedShape mirroredLoft = duct.GetMirroredLoft();
        if (mirroredLoft) {
            tools.Append(mirroredLoft->Shape());
        }
    }

    PNamedShape tool;
    BRepAlgoAPI_Fuse fuse;
    fuse.SetArguments(arguments);
    fuse.SetTools(tools);
    fuse.Build();
    if (fuse.IsDone()) {
        tool = std::make_shared<CNamedShape>(fuse.Shape(), "BOP_FUSE");

        // map names to tool
        for (auto const& duct_uid: m_ducts.GetUIDs()) {
            auto& duct = GetParent()->GetDuct(duct_uid);
            CBooleanOperTools::MapFaceNamesAfterBOP(fuse, duct.GetLoft(), tool);
        }
    }
    else {
        throw CTiglError("Cannot fuse ducts to a single cutting tool for Boolean operations with geometric components\n.");
    }
    return tool;
#endif
}

} //namespace tigl
