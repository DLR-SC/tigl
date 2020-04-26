/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Partial implementation of the ITiglGeometricComponent interface.
*/

#ifndef CTIGLABSTRACTGEOMETRICCOMPONENT_H
#define CTIGLABSTRACTGEOMETRICCOMPONENT_H

#include <gp_Pnt.hxx>
#include <string>

#include "PNamedShape.h"
#include "ITiglGeometricComponent.h"
#include "Cache.h"

#include "Bnd_Box.hxx"

namespace tigl
{
class CCPACSTransformation;

// TODO(bgruber): think about merging CTiglAbstractGeometricComponent into ITiglGeometricComponent, I don't think we need a separate interface
class CTiglAbstractGeometricComponent : public ITiglGeometricComponent
{
public:
    CTiglAbstractGeometricComponent();

    TIGL_EXPORT void Reset() const;

    TIGL_EXPORT virtual TiglSymmetryAxis GetSymmetryAxis() const;

    // Gets the loft of a geometric component
    TIGL_EXPORT PNamedShape GetLoft() const override;

    // Get the loft mirrored at the mirror plane
    TIGL_EXPORT virtual PNamedShape GetMirroredLoft();

    // return if pnt lies on the loft
    TIGL_EXPORT virtual bool GetIsOn(const gp_Pnt &pnt);
    
    // return if pnt lies on the mirrored loft
    // if the loft as no symmetry, false is returned
    TIGL_EXPORT bool GetIsOnMirrored(const gp_Pnt &pnt);

    // returns the bounding box of this component's loft
    TIGL_EXPORT Bnd_Box const& GetBoundingBox() const;

protected:
    virtual PNamedShape BuildLoft() const = 0;

    Cache<PNamedShape, CTiglAbstractGeometricComponent> loft;
    Cache<Bnd_Box, CTiglAbstractGeometricComponent> bounding_box;

private:
    CTiglAbstractGeometricComponent(const CTiglAbstractGeometricComponent&);
    void operator=(const CTiglAbstractGeometricComponent&);

    void BuildLoft(PNamedShape& cache) const;
    void CalcBoundingBox(Bnd_Box& bb) const;
};

} // end namespace tigl

#endif // CTIGLABSTRACTGEOMETRICCOMPONENT_H
