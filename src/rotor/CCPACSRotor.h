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
* @brief  Implementation of CPACS rotor handling routines.
*/

#ifndef CCPACSROTOR_H
#define CCPACSROTOR_H

#include <string>

#include "tigl_config.h"
#include "tixi.h"
#include "CTiglTransformation.h"
#include "CTiglAbstractPhysicalComponent.h"
#include "CCPACSRotorHub.h"

#include "TopoDS_Shape.hxx"


namespace tigl
{

enum TiglRotorType {
    TIGLROTOR_UNDEFINED,
    TIGLROTOR_MAIN_ROTOR,
    TIGLROTOR_TAIL_ROTOR,
    TIGLROTOR_FENESTRON,
    TIGLROTOR_PROPELLER
};
typedef enum TiglRotorType TiglRotorType;

class CCPACSConfiguration;

class CCPACSRotor : public CTiglAbstractPhysicalComponent
{

public:
    // Constructor
    CCPACSRotor(CCPACSConfiguration* config);

    // Virtual destructor
    virtual ~CCPACSRotor(void);

    // Invalidates internal state
    void Invalidate(void);

    // Read CPACS rotor elements
    void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& rotorXPath);

    // Returns the name of the rotor
    const std::string& GetName(void) const;

    // Returns the description of the rotor
    const std::string& GetDescription(void) const;

    // Get the Transformation object
    CTiglTransformation GetTransformation(void);

    // Sets the Translation object
    void Translate(CTiglPoint trans);

    // Get Translation
    CTiglPoint GetTranslation(void);

    // Returns the type of the rotor
    const TiglRotorType& GetType(void) const;

    // Returns the nominal rotations per minute (rpm) of the rotor
    const double& GetNominalRotationsPerMinute(void) const;

    // Returns the parent configuration
    CCPACSConfiguration& GetConfiguration(void) const;

    // Returns the rotor hub object
    const CCPACSRotorHub& GetRotorHub(void) const;

    // Returns the rotor disk geometry
    TopoDS_Shape GetRotorDisk(void);

    /*
            // Get rotor blade count
            int GetRotorBladeCount(void) const;

            // Returns the rotor blade for a given index
            CCPACSRotorBlade& GetRotorBlade(int index) const;

    #ifdef TIGL_USE_XCAF
            // builds data structure for a TDocStd_Application
            // mostly used for export
            TDF_Label ExportDataStructure(CCPACSConfiguration& config, Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label);
    #endif

            // Get the rotor blade transformation for a rotor blade given by its index
            CTiglTransformation GetRotorBladeTransformation(int index);

            // Gets the loft of a rotor blade given by its index
            TopoDS_Shape & GetRotorBladeGeometry(int index);

            // Gets the volume of this rotor
            double GetVolume(void);
    */
    /*TODO:
            // Gets the surface area of this rotor
            double GetSurfaceArea();

            // Returns the reference area of this rotor.
            // Here, we always take the reference rotor disk area projected to a plane normal to the rotor hub direction
            double GetReferenceArea();

            // Returns the radius of the rotor
            double GetRotorRadius(void);
    */
    // Returns the Component Type TIGL_COMPONENT_ROTOR.
    TiglGeometricComponentType GetComponentType(void)
    {
        return TIGL_COMPONENT_ROTOR | TIGL_COMPONENT_PHYSICAL;
    }

    virtual void SetSymmetryAxis(const std::string& axis);

protected:
    // Cleanup routine
    void Cleanup(void);

    // Build transformation matrix for the rotor
    void BuildMatrix(void);

    // Update internal rotor data
    void Update(void);

    virtual TopoDS_Shape BuildLoft(void);

    TopoDS_Shape BuildRotorDisk(void);

    /*TODO:
            // Adds all Segments of this rotor to one shape
            TopoDS_Shape BuildRotorGeometry(void);
    */

private:
    // Copy constructor
    CCPACSRotor(const CCPACSRotor&);

    // Assignment operator
    void operator=(const CCPACSRotor&);

private:
    std::string            name;                        /**< Rotor name           */
    std::string            description;                 /**< Rotor description    */
    TiglRotorType          type;                        /**< Rotor type           */
    double                 nominalRotationsPerMinute;   /**< Rotor type           */
    CCPACSRotorHub         rotorHub;                    /**< Rotor hub            */
//TODO:        CCPACSRotorBlades      rotorBlades;                 /**< Rotor blades         */
    CCPACSConfiguration*   configuration;               /**< Parent configuration */
//TODO:        TopoDS_Shape           rotorGeometry;               /**< All blades           */
    bool                   invalidated;                 /**< Internal state flag  */
    bool                   rebuildGeometry;             /**< Indicates if geometry needs to be rebuilt */
//TODO:        double                 myVolume;                    /**< Volume of this Rotor */
};

} // end namespace tigl

#endif // CCPACSROTOR_H
