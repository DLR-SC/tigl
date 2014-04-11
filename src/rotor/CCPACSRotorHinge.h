/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-04-11 Philipp Kunze <Philipp.Kunze@dlr.de>
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
* @brief  Implementation of CPACS rotor hinge handling routines.
*/

#ifndef CCPACSROTORHINGE_H
#define CCPACSROTORHINGE_H

#include <string>
#include <vector>

#include "tixi.h"
#include "CTiglPoint.h"
#include "CTiglTransformation.h"


namespace tigl
{

enum TiglRotorHingeType {
    TIGLROTORHINGE_UNDEFINED,
    TIGLROTORHINGE_FLAP,
    TIGLROTORHINGE_PITCH,
    TIGLROTORHINGE_LEAD_LAG
};
typedef enum TiglRotorHingeType TiglRotorHingeType;

class CCPACSConfiguration;

class CCPACSRotorHinge
{

public:
    // Constructor
    CCPACSRotorHinge(CCPACSConfiguration* config);

    // Virtual destructor
    virtual ~CCPACSRotorHinge(void);

    // Invalidates internal state
    void Invalidate(void);

    // Read CPACS rotorHinge elements
    void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorHingeXPath);

    // Returns the UID of the rotor blade attachment
    const std::string& GetUID(void) const;

    // Returns the name of the rotor blade attachment
    const std::string& GetName(void) const;

    // Returns the description of the rotor blade attachment
    const std::string& GetDescription(void) const;

    // Returns the type of the rotor blade attachment
    const TiglRotorHingeType& GetType(void) const;

    // Gets the rotor hinge transformation
    const CTiglTransformation& GetTransformation(void) const;

    // Gets the scaling vector of the rotor hinge transformation
    const CTiglPoint& GetScaling(void) const;

    // Gets the rotation vector of the rotor hinge transformation
    const CTiglPoint& GetRotation(void) const;

    // Gets the translation vector of the rotor hinge transformation
    const CTiglPoint& GetTranslation(void) const;

    // Gets the rotor hinge neutral position
    const double& GetNeutralPosition(void) const;

    // Gets the rotor hinge static stiffness
    const double& GetStaticStiffness(void) const;

    // Gets the rotor hinge dynamic stiffness
    const double& GetDynamicStiffness(void) const;

    // Gets the rotor hinge damping
    const double& GetDamping(void) const;

    // Gets the rotor hinge damping
    double GetHingeAngle(double thetaDeg=0., double a0=0., std::vector<double> aSin=std::vector<double>(), std::vector<double> aCos=std::vector<double>()) const;

    // Returns the parent configuration
    CCPACSConfiguration& GetConfiguration(void) const;

protected:
    // Cleanup routine
    void Cleanup(void);

    // Build transformation matrix for the hinge
    void BuildMatrix(void);

    // Update internal hinge data
    void Update(void);

private:
    // Copy constructor
    CCPACSRotorHinge(const CCPACSRotorHinge&);

    // Assignment operator
    void operator=(const CCPACSRotorHinge&);

private:
    std::string           uID;                      /**< Hinge uID            */
    std::string           name;                     /**< Hinge name           */
    std::string           description;              /**< Hinge description    */
    CTiglTransformation   transformation;           /**< Hinge transformation */
    CTiglPoint            translation;              /**< Hinge transformation: translation */
    CTiglPoint            scaling;                  /**< Hinge transformation: scaling */
    CTiglPoint            rotation;                 /**< Hinge transformation: rotation */
    TiglRotorHingeType    type;                     /**< Hinge type           */
    double                neutralPosition;          /**< Neutral position of the hinge */
    double                staticStiffness;          /**< Static stiffness of the hinge */
    double                dynamicStiffness;         /**< Dynamic stiffness of the hinge */
    double                damping;                  /**< Damping of the hinge */
    CCPACSConfiguration*  configuration;            /**< Parent configuration */
    bool                  invalidated;              /**< Internal state flag  */
};

} // end namespace tigl

#endif // CCPACSROTORHINGE_H
