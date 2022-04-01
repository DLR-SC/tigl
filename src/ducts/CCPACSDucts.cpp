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

#include "CCPACSDucts.h"
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
#endif



namespace tigl {

CCPACSDucts::CCPACSDucts(CCPACSAircraftModel* parent, CTiglUIDManager* uidMgr)
  : generated::CPACSDucts(parent, uidMgr)
  , fusedDucts(*this, &CCPACSDucts::FuseDucts)
{}

void CCPACSDucts::Invalidate()
{
    fusedDucts.clear();
}

void CCPACSDucts::FuseDucts(PNamedShape& tool) const
{
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
    for (auto const& duct: m_ducts) {

        PNamedShape loft = duct->GetLoft();
        if (arguments.Extent() == 0) {
            arguments.Append(loft->Shape());
        }
        else {
            tools.Append(loft->Shape());
        }

        // add mirrored loft, if any
        PNamedShape mirroredLoft = duct->GetMirroredLoft();
        if (mirroredLoft) {
            tools.Append(mirroredLoft->Shape());
        }
    }

    BRepAlgoAPI_Fuse fuse;
    fuse.SetArguments(arguments);
    fuse.SetTools(tools);
    fuse.Build();
    if (fuse.IsDone()) {
        tool = std::make_shared<CNamedShape>(fuse.Shape(), "some_name");
    }
    else {
        throw CTiglError("Cannot fuse ducts to a single cutting tool for Boolean operations with geometric components\n.");
    }
#endif
}

PNamedShape CCPACSDucts::LoftWithDuctCutouts(PNamedShape const& cleanLoft) const
{
    if (m_ducts.size() == 0) {
        return cleanLoft;
    }
    auto loft = CCutShape(cleanLoft, *fusedDucts).NamedShape();

    // update the facetraits for every face of the result
    for (int iFace = 0; iFace < static_cast<int>(loft->GetFaceCount()); ++iFace) {
        CFaceTraits ft = loft->GetFaceTraits(iFace);
        ft.SetOrigin(cleanLoft);
        loft->SetFaceTraits(iFace, ft);
    }

#ifdef DEBUG
    dumpShape((*fusedDucts)->Shape(), "debugShapes", "ductTool");
    dumpShape(loft->Shape(), "debugShapes", "loftWithoutDucts");
#endif

    return loft;
}

} //namespace tigl
