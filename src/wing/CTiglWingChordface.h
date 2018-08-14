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
#include "PNamedShape.h"
#include "CTiglWingSegmentList.h"
#include "Cache.h"

#include <Geom_BSplineSurface.hxx>

#include <vector>

namespace tigl
{

class CTiglUIDManager;

class CTiglWingChordface : public CTiglAbstractGeometricComponent
{
public:
    TIGL_EXPORT CTiglWingChordface(const CTiglWingSegmentList& segments, CTiglUIDManager* uidMgr);
    TIGL_EXPORT virtual ~CTiglWingChordface();

    TIGL_EXPORT gp_Pnt GetPoint(double eta, double xsi) const;

    TIGL_EXPORT void GetEtaXsi(gp_Pnt point, double& eta, double& xsi) const;

    TIGL_EXPORT void SetUID(const std::string& uid);

    TIGL_EXPORT virtual void Reset();

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE
    {
        return TIGL_COMPONENT_PHYSICAL;
    }


    /**
     * @brief Returns the Eta coordinate of each element
     */
    TIGL_EXPORT const std::vector<double>& GetElementEtas() const;

    TIGL_EXPORT const Handle(Geom_BSplineSurface) GetSurface() const;

protected:
    PNamedShape BuildLoft() const OVERRIDE;

private:
    struct ChordSurfaceCache {
        Handle(Geom_BSplineSurface) chordSurface;
        std::vector<double> elementEtas;
    };


    CTiglWingChordface(const CTiglWingChordface&); // disabled copy constructor

    void unregisterShape();
    void BuildChordSurface(ChordSurfaceCache& cache) const;

    CTiglWingSegmentList _segments;
    std::string _uid;

    CTiglUIDManager* _uidManager;

    Cache<ChordSurfaceCache, CTiglWingChordface> _cache;
};

}

#endif // CTIGLWINGCHORDFACE_H
