/* 
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-10-21 Jonas Jepsen <Jonas.Jepsen@dlr.de>
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
* @brief  Implementation of CPACS wing handling routines.
*/

#ifndef CCPACSGENERICSYSTEM_H
#define CCPACSGENERICSYSTEM_H

#include <string>

#include "tixi.h"
#include "tigl_internal.h"
#include "CTiglAbstractPhysicalComponent.h"


namespace tigl
{

class CCPACSConfiguration;

class CCPACSGenericSystem : public CTiglAbstractPhysicalComponent
{

public:
    // Constructor
    TIGL_EXPORT CCPACSGenericSystem(CCPACSConfiguration* config);

    // Virtual destructor
    TIGL_EXPORT virtual ~CCPACSGenericSystem();

    TIGL_EXPORT virtual const std::string& GetUID() const OVERRIDE;
    TIGL_EXPORT virtual void SetUID(const std::string& uid) OVERRIDE;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS generic system elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & genericSysXPath);

    // Returns the name of the generic system
    TIGL_EXPORT const std::string & GetName() const;

    // Returns the parent configuration
    TIGL_EXPORT CCPACSConfiguration & GetConfiguration() const;

    // Sets a Transformation object
    TIGL_EXPORT void Translate(CTiglPoint trans);

    // Returns the Component Type TIGL_COMPONENT_GENERICSYSTEM.
    TIGL_EXPORT TiglGeometricComponentType GetComponentType() {return TIGL_COMPONENT_GENERICSYSTEM | TIGL_COMPONENT_PHYSICAL;}


protected:
    // Cleanup routine
    void Cleanup();

    // Update internal wing data
    void Update();
        
    // Build the shape of the system
    PNamedShape BuildLoft();

private:
    // get short name for loft
    std::string GetShortShapeName();

    // Copy constructor
    CCPACSGenericSystem(const CCPACSGenericSystem & );

    // Assignment operator
    void operator=(const CCPACSGenericSystem & );

private:
    std::string                    uid;
    std::string                    name;                     /**< System name         */
    std::string                    geometricBaseType;        /**< Geometric base type */
    TiglSymmetryAxis               symmetryAxis;
    CCPACSTransformation           transformation;
    CCPACSConfiguration*           configuration;            /**< Parent configuration*/
    bool                           invalidated;              /**< Internal state flag */
};

} // end namespace tigl

#endif // CCPACSGENERICSYSTEM_H
