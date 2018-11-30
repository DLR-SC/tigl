/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CCPACSNacelleCowl.h"
#include "CCPACSNacelleSection.h"
#include "generated/CPACSNacelleGuideCurve.h"
#include "CTiglNacelleGuideCurveBuilder.h"
#include "CTiglMakeLoft.h"

#include "BRepTools.hxx"

namespace tigl
{

CCPACSNacelleCowl::CCPACSNacelleCowl(CTiglUIDManager* uidMgr)
    : generated::CPACSNacelleCowl(uidMgr)
    , CTiglAbstractGeometricComponent()
{}

std::string CCPACSNacelleCowl::GetDefaultedUID() const
{
    return generated::CPACSNacelleCowl::GetUID();
}

PNamedShape CCPACSNacelleCowl::BuildLoft() const
{
    CTiglMakeLoft lofter;

    // get profile curves
    for(size_t i = 1; i <= m_sections.GetSectionCount(); ++i ) {
        CCPACSNacelleSection& section = m_sections.GetSection(i);
        TopoDS_Wire profileWire = section.GetTransformedWire();

        // remove blending part for rotationally symmetric interior
        // TODO

#ifdef DEBUG
        std::stringstream ss;
        ss << "D:/tmp/nacelleProfile_"<<i<<".brep";
        BRepTools::Write(profileWire, ss.str().c_str());
#endif
        lofter.addProfiles(profileWire);
    }

    // get guide curves
    std::vector<TopoDS_Wire> guides;
    for(size_t i = 1; i <= m_guideCurves.GetGuideCurveCount(); ++i ) {
        const CCPACSNacelleGuideCurve& guide = m_guideCurves.GetGuideCurve(i);

        CTiglNacelleGuideCurveBuilder gcbuilder(guide);
        TopoDS_Wire guideWire = gcbuilder.GetWire();
        guides.push_back(guideWire);

#ifdef DEBUG
        std::stringstream ss;
        ss << "D:/tmp/nacelleGuide_"<<i<<".brep";
        BRepTools::Write(guideWire, ss.str().c_str());
#endif
    }

    // Check if we have enough guide curves, otherwise add at LE, TE (two for blunt TE) and at rotation curves'
    // statZetaBlending and endZetaBlendig values.
    //TODO

    for(size_t i=0; i<guides.size(); ++i) {
        lofter.addGuides(guides[i]);
    }



    // get rotation curve and generate rotationally symmetric interior

    // blend the surfaces

    // interpolate curve network

    return PNamedShape();
}

} //namespace tigl
