/*
* Copyright (c) 2018 Airbus Defence and Space and RISC Software GmbH
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

#include "CCPACSFuselageStringerFramePosition.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Pln.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include "generated/CPACSStringer.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"
#include "CNamedShape.h"
#include "CCPACSFuselage.h"
#include "CCPACSFrame.h"
#include "CCPACSFuselageStringer.h"

namespace tigl{
// NOTE:
// CPACS stores absolute positionings, from which we calculate cached relative ones
// if relative positions are set, they are cached and immediately converted to absolute ones
// if absolute positions are set, the cache is invalidated

CCPACSFuselageStringerFramePosition::CCPACSFuselageStringerFramePosition(CCPACSFrame* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSStringerFramePosition(parent, uidMgr)
    , m_relCache(*this, &CCPACSFuselageStringerFramePosition::UpdateRelativePositioning)
{
}

CCPACSFuselageStringerFramePosition::CCPACSFuselageStringerFramePosition(CCPACSFuselageStringer* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSStringerFramePosition(parent, uidMgr)
    , m_relCache(*this, &CCPACSFuselageStringerFramePosition::UpdateRelativePositioning)
{
}

gp_Pnt CCPACSFuselageStringerFramePosition::GetRefPoint() const
{
    return gp_Pnt(m_positionX, m_referenceY, m_referenceZ);
}

void CCPACSFuselageStringerFramePosition::SetPositionX(const double& value)
{
    generated::CPACSStringerFramePosition::SetPositionX(value);
    Invalidate();
}

void CCPACSFuselageStringerFramePosition::SetReferenceY(const double& value)
{
    generated::CPACSStringerFramePosition::SetReferenceY(value);
    Invalidate();
}

void CCPACSFuselageStringerFramePosition::SetReferenceZ(const double& value)
{
    generated::CPACSStringerFramePosition::SetReferenceZ(value);
    Invalidate();
}

double CCPACSFuselageStringerFramePosition::GetPositionXRel() const {
    return m_relCache->positionXRel;
}

double CCPACSFuselageStringerFramePosition::GetReferenceYRel() const {
    return m_relCache->referenceYRel;
}

double CCPACSFuselageStringerFramePosition::GetReferenceZRel() const {
    return m_relCache->referenceZRel;
}

void CCPACSFuselageStringerFramePosition::SetPositionXRel(double positionXRel)
{
    //m_relCache->positionXRel = positionXRel;
    m_positionX = m_relCache->xmin + (m_relCache->xmax - m_relCache->xmin) * positionXRel;
    if (fabs(m_positionX) < 1e-6)
        m_positionX = 0.;
    Invalidate();
}

void CCPACSFuselageStringerFramePosition::SetReferenceYRel(double referenceYRel)
{
    //m_relCache->referenceYRel = referenceYRel;
    m_referenceY = m_relCache->ymin + (m_relCache->ymax - m_relCache->ymin) * referenceYRel;
    if (fabs(m_referenceY) < 1e-6)
        m_referenceY = 0.;
    Invalidate();
}

void CCPACSFuselageStringerFramePosition::SetReferenceZRel(double referenceZRel)
{
    //m_relCache->referenceZRel = referenceZRel;
    m_referenceZ = m_relCache->zmin + (m_relCache->zmax - m_relCache->zmin) * referenceZRel;
    if (fabs(m_referenceZ) < 1e-6)
        m_referenceZ = 0.;
    Invalidate();
}

void CCPACSFuselageStringerFramePosition::Invalidate() {
    m_relCache.clear();
}

void CCPACSFuselageStringerFramePosition::GetXBorders(double& xmin, double& xmax)
{
    xmin = m_relCache->xmin;
    xmax = m_relCache->xmax;
}

void CCPACSFuselageStringerFramePosition::GetYBorders(double& ymin, double& ymax)
{
    ymin = m_relCache->ymin;
    ymax = m_relCache->ymax;
}
    
void CCPACSFuselageStringerFramePosition::GetZBorders(double& zmin, double& zmax)
{
    zmin = m_relCache->zmin;
    zmax = m_relCache->zmax;
}

void CCPACSFuselageStringerFramePosition::UpdateRelativePositioning(RelativePositionCache& cache) const
{
    const TopoDS_Shape fuselageLoft = GetFuselage().GetLoft(FUSELAGE_COORDINATE_SYSTEM)->Shape();
        
    Bnd_Box bBox1;
    BRepBndLib::Add(fuselageLoft, bBox1);
    double XMN = 0., XMX = 0., YMN = 0., YMX = 0., ZMN = 0., ZMX = 0.;
    bBox1.Get(XMN, YMN, ZMN, XMX, YMX, ZMX);
        
    if (fabs(XMN) < 1e-6)
        XMN = 0.;
        
    const double xmin = XMN;
    const double xmax = XMX;
    const double positionXRel = (m_positionX - XMN) / (XMX - XMN);
        
    if (positionXRel > 1. || positionXRel < 0.) {
        // m_positionX is outside bounding box
        throw CTiglError("Error during relative fuselage structure X positioning calculation");
    }
        
    gp_Pnt pRef(m_positionX, 0., 0.);
    gp_Pln cutPlane(pRef, gp_Dir(1,0,0));
        
    TopoDS_Shape Section = CutShapes(fuselageLoft, BRepBuilderAPI_MakeFace(cutPlane).Face());
        
    Bnd_Box bBox2;
    BRepBndLib::Add(Section, bBox2);
    bBox2.Get(XMN, YMN, ZMN, XMX, YMX, ZMX);
        
    if (fabs(YMN) < 1e-6)
        YMN = 0.;
        
    if (fabs(ZMN) < 1e-6)
        ZMN = 0.;
        
    const double ymin = YMN;
    const double ymax = YMX;
    const double referenceYRel = (m_referenceY - YMN) / (YMX - YMN);
        
    if (referenceYRel > 1. || referenceYRel < 0.) {
        throw CTiglError("Error during relative fuselage structure Y positioning calculation");
    }
        
    const double zmin = ZMN;
    const double zmax = ZMX;
    const double referenceZRel = (m_referenceZ - ZMN) / (ZMX - ZMN);
        
    if (referenceZRel > 1. || referenceZRel < 0.) {
        throw CTiglError("Error during relative fuselage structure Z positioning calculation");
    }

    cache.xmin = xmin;
    cache.xmax = xmax;
    cache.ymin = ymin;
    cache.ymax = ymax;
    cache.zmin = zmin;
    cache.zmax = zmax;
    cache.positionXRel = positionXRel;
    cache.referenceYRel = referenceYRel;
    cache.referenceZRel = referenceZRel;
}

const CCPACSFuselage& CCPACSFuselageStringerFramePosition::GetFuselage() const {
    const CCPACSFuselageStructure* s = nullptr;
    if (IsParent<CCPACSFrame>())
        s = GetParent<CCPACSFrame>()->GetParent()->GetParent();
    else if (IsParent<CCPACSFuselageStringer>())
        s = GetParent<CCPACSFuselageStringer>()->GetParent()->GetParent();
    else
        throw CTiglError("Invalid parent");
    return *s->GetParent();
}

} // namespace tigl
