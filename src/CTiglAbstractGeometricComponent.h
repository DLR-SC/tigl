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

#include <list>
#include <string>
#include <TopoDS_Shape.hxx>

#include "ITiglGeometricComponent.h"


namespace tigl {

    class CTiglAbstractGeometricComponent : public ITiglGeometricComponent
    {

    public:
        // Constructor
        CTiglAbstractGeometricComponent(void);

        // Virtual Destructor
        virtual ~CTiglAbstractGeometricComponent(void);

           // Gets the loft of a geometric component
        virtual TopoDS_Shape& GetLoft(void);

        // Get the loft mirrored at the mirror plane
        virtual TopoDS_Shape GetMirroredLoft(void);

        // Gets the component uid
        virtual const std::string& GetUID(void);

        // Sets the component uid
        virtual void SetUID(const std::string& uid);

        // Gets symmetry axis
        virtual TiglSymmetryAxis GetSymmetryAxis(void);

        // Sets symmetry axis
        virtual void SetSymmetryAxis(const std::string& axis);

        // Returns a unique Hashcode for a specific geometric component
        int GetComponentHashCode(void);

        // Get transformation object
        virtual CTiglTransformation GetTransformation(void);

        // Get component translation
        virtual CTiglPoint GetTranslation(void);

        // Set transformation object
        virtual void Translate(CTiglPoint trans);
        
        // return if pnt lies on the loft
        bool GetIsOn(const gp_Pnt &pnt);
        
        // return if pnt lies on the mirrored loft
        // if the loft as no symmetry, false is returned
        bool GetIsOnMirrored(const gp_Pnt &pnt);
    protected:
        // Resets the geometric component.
        virtual void Reset(void);
        
        virtual TopoDS_Shape BuildLoft(void) = 0;

        CTiglTransformation        transformation;
        CTiglTransformation        backTransformation;
        CTiglPoint                 translation;
        CTiglPoint                 scaling;
        CTiglPoint                 rotation;
        TopoDS_Shape               loft;

    private:
        // Copy constructor
        CTiglAbstractGeometricComponent(const CTiglAbstractGeometricComponent& ) { /* Do nothing */ }

        // Assignment operator
        void operator=(const CTiglAbstractGeometricComponent& );

    private:
        std::string        myUID;           /**< UID of this component               */
        TiglSymmetryAxis   mySymmetryAxis;  /**< SymmetryAxis of this component      */
    };

} // end namespace tigl

#endif // CTIGLABSTRACTGEOMETRICCOMPONENT_H
