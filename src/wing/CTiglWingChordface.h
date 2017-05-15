/*
* Copyright (C) 2017 German Aerospace Center (DLR/SC)
*
* Created: 2017-05-15 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef CTIGLWINGCHORDFACE_H
#define CTIGLWINGCHORDFACE_H

#include "tigl_internal.h"

#include "CTiglAbstractGeometricComponent.h"
#include "CTiglUIDManager.h"
#include "PNamedShape.h"
#include "CCPACSWingSegment.h"

#include <Geom_BSplineSurface.hxx>

#include <vector>

namespace tigl
{

class CTiglWingChordface : public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CTiglWingChordface(const std::vector<CCPACSWingSegment*>& segments, CTiglUIDManager* uidMgr);
    TIGL_EXPORT virtual ~CTiglWingChordface();

    TIGL_EXPORT gp_Pnt GetPoint(double eta, double xsi) const;

    TIGL_EXPORT void SetUID(const std::string& uid);

    TIGL_EXPORT virtual void Reset();

    TIGL_EXPORT virtual std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT virtual TiglGeometricComponentType GetComponentType() const OVERRIDE
    {
        return TIGL_COMPONENT_PHYSICAL;
    }

    TIGL_EXPORT void BuildChordSurface() const;

    /**
     * @brief Returns the Eta coordinate of each element
     */
    TIGL_EXPORT std::vector<double> GetElementEtas() const;

protected:
    virtual PNamedShape BuildLoft() OVERRIDE;

private:
    std::string _uid;
    typedef std::vector<CCPACSWingSegment*> SegmentList;
    const SegmentList& _segments;

    CTiglUIDManager* _uidManager;

    mutable Handle(Geom_BSplineSurface) _chordSurface;
};

}

#endif // CTIGLWINGCHORDFACE_H
