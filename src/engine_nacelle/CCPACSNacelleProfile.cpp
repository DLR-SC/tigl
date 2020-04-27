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

#include "CCPACSNacelleProfile.h"
#include "CTiglInterpolateBsplineWire.h"
#include "BRepBuilderAPI_MakeWire.hxx"

namespace tigl {

CCPACSNacelleProfile::CCPACSNacelleProfile(CCPACSCurveProfiles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSProfileGeometry2D(parent, uidMgr)
    , pointListAlgo(*this,&CCPACSNacelleProfile::buildPointListAlgo)
{}

CCPACSNacelleProfile::CCPACSNacelleProfile(CCPACSNacelleProfiles* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSProfileGeometry2D(parent, uidMgr)
    , pointListAlgo(*this,&CCPACSNacelleProfile::buildPointListAlgo)
{}

void CCPACSNacelleProfile::SetPointListAlgoType(enum pointListAlgoType type)
{
    algoType = type;
}

void CCPACSNacelleProfile::buildPointListAlgo(std::unique_ptr<CTiglWingProfilePointList>& cache) const
{
    cache.reset(new CTiglWingProfilePointList(*this, *m_pointList_choice1));
}

ITiglWingProfileAlgo* CCPACSNacelleProfile::GetProfileAlgo()
{
    if (m_pointList_choice1) {
        // in case the wing profile algorithm is a point list, create the additional algorithm instance
        return &**pointListAlgo;
    } else if (m_cst2D_choice2) {
        return &*m_cst2D_choice2;
    } else {
        throw CTiglError("no profile algorithm");
    }
}

const ITiglWingProfileAlgo* CCPACSNacelleProfile::GetProfileAlgo() const
{
    return const_cast<CCPACSNacelleProfile&>(*this).GetProfileAlgo();
}

bool CCPACSNacelleProfile::HasBluntTE() const
{
    if ( algoType == Airfoil ) {
        const ITiglWingProfileAlgo* algo = GetProfileAlgo();
        if (!algo) {
            throw CTiglError("No wing profile algorithm regsitered in CCPACSNacelleProfile::HasBluntTE()!");
        }
        return algo->HasBluntTE();
    }
    else {
        throw CTiglError("CCPACSNacelleProfile::HasBluntTE() only supported in conjunction with an Airfoil point list algorithm!");
        return false;
    }
}

TopoDS_Wire CCPACSNacelleProfile::GetWire(TiglShapeModifier mod) const
{
    TopoDS_Wire wire;
    if ( algoType == Airfoil ) {
        const ITiglWingProfileAlgo* profileAlgo = GetProfileAlgo();

        // rebuild closed wire
        BRepBuilderAPI_MakeWire closedWireBuilder;
        closedWireBuilder.Add(profileAlgo->GetLowerWire(mod));
        closedWireBuilder.Add(profileAlgo->GetUpperWire(mod));
        if (!profileAlgo->GetTrailingEdge(mod).IsNull()) {
            closedWireBuilder.Add(profileAlgo->GetTrailingEdge(mod));
        }

        wire = closedWireBuilder.Wire();
    }
    else if ( algoType == Simple ) {
        if ( !GetPointList_choice1() ) {
            throw CTiglError("CCPACSNacelleProfile::GetWire() uses point list algorithm type \"Simple\", but the CPACSProfileGeometry2D type is not defined using a list of points (Maybe CST type?)");
        }
        const std::vector<CTiglPoint>& tiglpoints = GetPointList_choice1()->AsVector();
        ITiglWireAlgorithm::CPointContainer points;

        for ( size_t i = 0; i<tiglpoints.size(); ++i) {
            points.push_back(gp_Pnt(tiglpoints[i].x, tiglpoints[i].y, tiglpoints[i].z) );
        }

        CTiglInterpolateBsplineWire wireBuilder;
        wire = wireBuilder.BuildWire(points);
    }
    else {
        throw CTiglError("Unsupported point list algorithm type in CCPACSNacelleProfile::GetWire()!");
    }
    return wire;
}

TopoDS_Edge CCPACSNacelleProfile::GetUpperWire(TiglShapeModifier mod) const
{
    if ( algoType == Airfoil ) {
        return GetProfileAlgo()->GetUpperWire(mod);
    }
    else {
        throw CTiglError("CCPACSNacelleProfile::GetUpperWire() only supported in conjunction with an Airfoil point list algorithm!");
    }
}

TopoDS_Edge CCPACSNacelleProfile::GetLowerWire(TiglShapeModifier mod) const
{
    if ( algoType == Airfoil ) {
        return GetProfileAlgo()->GetLowerWire(mod);
    }
    else {
        throw CTiglError("CCPACSNacelleProfile::GetLowerWire() only supported in conjunction with an Airfoil point list algorithm!");
    }
}

TopoDS_Edge CCPACSNacelleProfile::GetTrailingEdge(TiglShapeModifier mod) const
{
    if ( algoType == Airfoil ) {
        return GetProfileAlgo()->GetTrailingEdge(mod);
    }
    else {
        throw CTiglError("CCPACSNacelleProfile::GetTrailingEdge() only supported in conjunction with an Airfoil point list algorithm!");
    }
}

gp_Pnt CCPACSNacelleProfile::GetLEPoint() const
{
    return GetProfileAlgo()->GetLEPoint();
}

// Returns the trailing edge point of the wing profile wire. The trailing edge point
// is already transformed by the wing profile transformation.
gp_Pnt CCPACSNacelleProfile::GetTEPoint() const
{
    return GetProfileAlgo()->GetTEPoint();
}

} //namepsace tigl
