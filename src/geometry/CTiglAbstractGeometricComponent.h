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
* @brief  Partial implementation of the ITiglGeometricComponent interface.
*/

#ifndef CTIGLABSTRACTGEOMETRICCOMPONENT_H
#define CTIGLABSTRACTGEOMETRICCOMPONENT_H

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include "tigl_internal.h"

#include <list>
#include <string>
#include "CNamedShape.h"
#include "PNamedShape.h"

#include "ITiglGeometricComponent.h"

namespace tigl
{
class CCPACSTransformation;

class CTiglAbstractGeometricComponent : public ITiglGeometricComponent
{
public:
    // Constructor
    TIGL_EXPORT CTiglAbstractGeometricComponent(TiglSymmetryAxis* symmetryAxis = NULL);
    TIGL_EXPORT CTiglAbstractGeometricComponent(boost::optional<TiglSymmetryAxis>* symmetryAxis);
    TIGL_EXPORT CTiglAbstractGeometricComponent(CCPACSTransformation* trans, TiglSymmetryAxis* symmetryAxis = NULL);
    TIGL_EXPORT CTiglAbstractGeometricComponent(CCPACSTransformation* trans, boost::optional<TiglSymmetryAxis>* symmetryAxis);

    // Gets the loft of a geometric component
    TIGL_EXPORT virtual PNamedShape GetLoft();

    // Get the loft mirrored at the mirror plane
    TIGL_EXPORT virtual PNamedShape GetMirroredLoft();

    // Gets symmetry axis
    TIGL_EXPORT TiglSymmetryAxis GetSymmetryAxis();

    // Sets symmetry axis
    TIGL_EXPORT void SetSymmetryAxis(const TiglSymmetryAxis& axis);

    // Gets symmetry axis as string
    DEPRECATED TIGL_EXPORT std::string GetSymmetryAxisString();

    // Sets symmetry axis as string
    DEPRECATED TIGL_EXPORT void SetSymmetryAxis(const std::string& axis);

    // Get transformation object
    TIGL_EXPORT virtual CTiglTransformation GetTransformation() const override;

    // Get component translation
    TIGL_EXPORT virtual CTiglPoint GetTranslation() const override;
    TIGL_EXPORT virtual ECPACSTranslationType GetTranslationType() const override;

    // Get component rotation
    TIGL_EXPORT virtual CTiglPoint GetRotation() const;

    // Get component scaling
    TIGL_EXPORT virtual CTiglPoint GetScaling() const;

    // Set transformation object
    TIGL_EXPORT virtual void Translate(CTiglPoint trans) override;

    // return if pnt lies on the loft
    TIGL_EXPORT virtual bool GetIsOn(const gp_Pnt &pnt);
    
    // return if pnt lies on the mirrored loft
    // if the loft as no symmetry, false is returned
    TIGL_EXPORT bool GetIsOnMirrored(const gp_Pnt &pnt);
protected:
    // Resets the geometric component.
    virtual void Reset();
    
    virtual PNamedShape BuildLoft() = 0;

    PNamedShape           loft;

private:
    // Copy constructor
    CTiglAbstractGeometricComponent(const CTiglAbstractGeometricComponent&) = delete;

    // Assignment operator
    void operator=(const CTiglAbstractGeometricComponent& ) = delete;

private:
    CCPACSTransformation* transformation;                                                 // references down to the transformation of the derived class (may be empty in case derived class does not have transformation)
    boost::variant<TiglSymmetryAxis*, boost::optional<TiglSymmetryAxis>*> symmetryAxis;   // references down to the symmetryAxis of the derived class (may be empty in case derived class does not have symmetry)
};

} // end namespace tigl

#endif // CTIGLABSTRACTGEOMETRICCOMPONENT_H
