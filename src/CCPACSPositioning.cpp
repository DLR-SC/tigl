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
* @brief  Implementation of CPACS wing positioning handling routines.
*/

#include "CCPACSPositioning.h"
#include "CTiglError.h"
#include "CTiglUIDManager.h"
#include "CCPACSWingSection.h"
#include "CCPACSFuselageSection.h"
#include "gp_Pnt.hxx"
#include "tiglcommonfunctions.h"
#include "tiglmathfunctions.h"

namespace tigl
{

CCPACSPositioning::CCPACSPositioning(CCPACSPositionings* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSPositioning(parent, uidMgr), invalidated(true) {}

// Build outer transformation matrix for the positioning
void CCPACSPositioning::BuildMatrix()
{
    // Compose the transformation for the tip section reference point.
    // The positioning transformation is basically a translation in two steps:
    // 1. from the wing origin to the innerPoint (= outerPoint of previous positioning)
    // 2. from the innerPoint to the outerPoint with coordinates given
    //    in a "spherical" coordinate system (length, sweepAngle, dihedralAngle).
    // The original section is neither rotated by sweepAngle nor by dihedralAngle.

    // Calculate the cartesian translation components for step two from "spherical" input coordinates
    CTiglTransformation tempTransformation;
    tempTransformation.SetIdentity();
    tempTransformation.AddRotationZ(-m_sweepAngle);
    tempTransformation.AddRotationX(m_dihedralAngle);
    gp_Pnt tempPnt = tempTransformation.Transform(gp_Pnt(0.0, m_length, 0.0));

    // Setup transformation combining both steps
    _toTransformation.SetIdentity();
    _toTransformation.AddTranslation(
        _fromPoint.x + tempPnt.X(),
        _fromPoint.y + tempPnt.Y(),
        _fromPoint.z + tempPnt.Z()
    );

    // calculate outer section point by transforming orign
    tempPnt = _toTransformation.Transform(gp_Pnt(0.0, 0.0, 0.0));
    _toPoint.x = tempPnt.X();
    _toPoint.y = tempPnt.Y();
    _toPoint.z = tempPnt.Z();
}

// Update internal segment data
void CCPACSPositioning::Update()
{
    if (!invalidated) {
        return;
    }

    BuildMatrix();
    invalidated = false;
}

 //Invalidates internal state
void CCPACSPositioning::InvalidateImpl(const boost::optional<std::string>& source) const
{
    if (invalidated) {
        return;
    }
    invalidated = true;

    // invalidate 
    if (m_uidMgr && m_uidMgr->IsUIDRegistered(m_toSectionUID)) {
        if (m_uidMgr->IsType<CCPACSWingSection>(m_toSectionUID)) {
            m_uidMgr->ResolveObject<CCPACSWingSection>(m_toSectionUID).Invalidate(GetUID());
        }
        else if (m_uidMgr->IsType<CCPACSFuselageSection>(m_toSectionUID)) {
            m_uidMgr->ResolveObject<CCPACSFuselageSection>(m_toSectionUID).Invalidate(GetUID());
        }
    }
}

// Read CPACS segment elements
void CCPACSPositioning::ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& positioningXPath)
{
    generated::CPACSPositioning::ReadCPACS(tixiHandle, positioningXPath);
    Update();
}

void CCPACSPositioning::SetLength(const double& value)
{
    generated::CPACSPositioning::SetLength(value);
    Invalidate();
}

void CCPACSPositioning::SetSweepAngle(const double& value)
{
    generated::CPACSPositioning::SetSweepAngle(value);
    Invalidate();
}

void CCPACSPositioning::SetDihedralAngle(const double& value)
{
    generated::CPACSPositioning::SetDihedralAngle(value);
    Invalidate();
}

void CCPACSPositioning::SetFromSectionUID(const boost::optional<std::string>& value)
{
    generated::CPACSPositioning::SetFromSectionUID(value);
    Invalidate();
}

void CCPACSPositioning::SetToSectionUID(const std::string& value)
{
    generated::CPACSPositioning::SetToSectionUID(value);
    Invalidate();
}

void CCPACSPositioning::SetFromPoint(const CTiglPoint& aPoint)
{
    _fromPoint = aPoint;
    Invalidate();
}

const CTiglPoint& CCPACSPositioning::GetFromPoint()
{
    Update();
    return _fromPoint;
}

void CCPACSPositioning::SetFromPointKeepingToPoint(const CTiglPoint& newFrom)
{
    Update();
    CTiglPoint delta =  _toPoint - newFrom ;
    SetParametersFromVector(delta);
}

void CCPACSPositioning:: SetToPoint(const CTiglPoint& newToPoint, bool moveDependentPositionings)
{
    Update();
    CTiglPoint delta = newToPoint - _fromPoint;
    SetParametersFromVector(delta);

    // update dependencies because we just want to change the section managed by this positioning
    std::vector<CCPACSPositioning*> dependencies = GetDependentPositionings();
    for (size_t i = 0; i < dependencies.size(); i++) {
        if ( moveDependentPositionings ) {
            dependencies.at(i)->Invalidate();
        }
        else {
            dependencies.at(i)->SetFromPointKeepingToPoint(newToPoint);
        }

    }
}

const CTiglPoint& CCPACSPositioning::GetToPoint()
{
    Update();
    return _toPoint;
}

CTiglTransformation CCPACSPositioning::GetToTransformation()
{
    Update();
    return _toTransformation;
}

void CCPACSPositioning::AddDependentPositioning(CCPACSPositioning* child)
{
    if (!child) {
        throw CTiglError("Dependent positioning is nullptr");
    }

    if (*child->m_fromSectionUID != m_toSectionUID) {
        throw CTiglError("Incompatible positioning connection in CCPACSPositioning::ConnectChildPositioning");
    }

    _dependentPositionings.push_back(child);
}

void CCPACSPositioning::DisconnectDependentPositionings()
{
    _dependentPositionings.clear();
}

const std::vector<CCPACSPositioning*> CCPACSPositioning::GetDependentPositionings() const
{
    return _dependentPositionings;
}

void CCPACSPositioning::SetParametersFromVector(const CTiglPoint& delta,  bool rounding )
{
    double length = delta.norm2();
    double dihedral = Degrees(atan2(delta.z,delta.y));
    double sweep = Degrees(atan2(delta.x, sqrt( pow(delta.y,2) + pow(delta.z,2) ))) ;

    if (rounding) {
        length = SnapUnitInterval(length);
        dihedral = SnapAngle(dihedral);
        sweep = SnapAngle(sweep);
    }

    SetDihedralAngle(dihedral);
    SetLength(length);
    SetSweepAngle(sweep);
    Invalidate();
}

} // namespace tigl
