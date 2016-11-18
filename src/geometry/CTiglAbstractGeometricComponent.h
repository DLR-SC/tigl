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

#include "tigl_internal.h"

#include <list>
#include <string>
#include "CNamedShape.h"
#include "PNamedShape.h"

#include "ITiglGeometricComponent.h"


namespace tigl 
{

class CTiglAbstractGeometricComponent : public ITiglGeometricComponent
{

public:
    // Constructor
    TIGL_EXPORT CTiglAbstractGeometricComponent(void);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CTiglAbstractGeometricComponent(void);

       // Gets the loft of a geometric component
    TIGL_EXPORT virtual PNamedShape GetLoft(void);

    // Get the loft mirrored at the mirror plane
    TIGL_EXPORT virtual PNamedShape GetMirroredLoft(void);

    // Gets the component uid
    TIGL_EXPORT virtual const std::string& GetUID(void) const;

    // Sets the component uid
    TIGL_EXPORT virtual void SetUID(const std::string& uid);

    // Gets symmetry axis
    TIGL_EXPORT virtual TiglSymmetryAxis GetSymmetryAxis(void);

    // Gets symmetry axis as string
    TIGL_EXPORT virtual const char* GetSymmetryAxisString(void) const;

    // Sets symmetry axis
    TIGL_EXPORT virtual void SetSymmetryAxis(const std::string& axis);

    // Get transformation object
    TIGL_EXPORT virtual CTiglTransformation GetTransformation(void);

    // Get component translation
    TIGL_EXPORT virtual CTiglPoint GetTranslation(void) const;
    
    // Get type of translation (global or local)
    TIGL_EXPORT virtual ECPACSTranslationType GetTranslationType(void) const;

    // Get component rotation
    TIGL_EXPORT virtual CTiglPoint GetRotation() const;

    // Get component scaling
    TIGL_EXPORT virtual CTiglPoint GetScaling() const;

    // Set transformation object
    TIGL_EXPORT virtual void Translate(CTiglPoint trans);
    
    // return if pnt lies on the loft
    TIGL_EXPORT virtual bool GetIsOn(const gp_Pnt &pnt);
    
    // return if pnt lies on the mirrored loft
    // if the loft as no symmetry, false is returned
    TIGL_EXPORT bool GetIsOnMirrored(const gp_Pnt &pnt);
protected:
    // Resets the geometric component.
    virtual void Reset(void);
    
    virtual PNamedShape BuildLoft(void) = 0;

    CCPACSTransformation       transformation;
    PNamedShape                loft;

private:
    // Copy constructor
    CTiglAbstractGeometricComponent(const CTiglAbstractGeometricComponent& ) { /* Do nothing */ }

    // Assignment operator
    void operator=(const CTiglAbstractGeometricComponent& );

    std::string        myUID;           /**< UID of this component               */
    TiglSymmetryAxis   mySymmetryAxis;  /**< SymmetryAxis of this component      */
};

} // end namespace tigl

#endif // CTIGLABSTRACTGEOMETRICCOMPONENT_H
