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
* @brief  Implementation of CPACS wing handling routines.
*/

#ifndef CCPACSWING_H
#define CCPACSWING_H

#include <string>

#include "tigl_config.h"
#include "tixi.h"
#include "CTiglTransformation.h"
#include "CTiglAbstractPhysicalComponent.h"
#include "CCPACSWingSections.h"
#include "CCPACSWingSegments.h"
#include "CCPACSWingComponentSegments.h"
#include "CCPACSWingPositionings.h"
#include "CTiglAbstractSegment.h"

#include "TopoDS_Shape.hxx"


namespace tigl {

    class CCPACSConfiguration;

    class CCPACSWing : public CTiglAbstractPhysicalComponent
    {

    public:
        // Constructor
        CCPACSWing(CCPACSConfiguration* config);

        // Virtual destructor
        virtual ~CCPACSWing(void);

        // Invalidates internal state
        void Invalidate(void);

        // Read CPACS wing elements
        void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & wingXPath);

        // Returns the name of the wing
        const std::string & GetName(void) const;

        // Returns the parent configuration
        CCPACSConfiguration & GetConfiguration(void) const;

        // Get section count
        int GetSectionCount(void) const;

        // Returns the section for a given index
        CCPACSWingSection & GetSection(int index) const;

        // Get segment count
        int GetSegmentCount(void) const;

        // Returns the segment for a given index or uid
        CTiglAbstractSegment & GetSegment(const int index);
        CTiglAbstractSegment & GetSegment(std::string uid);

        // Return the WingComponentSegment for a given index
        CCPACSWingComponentSegment& GetWingComponentSegment(int index);

        // Return the number of WingComponentSegments
        int GetWingComponentSegmentCount();

        // Get segment count
        int GetComponentSegmentCount(void);

        // Extends given flap of this wing
        TopoDS_Shape ExtendFlap(std::string flapUID, double flapDeflectionPercantage );

        // Adds all Segments of this wing and flaps to one shape
        TopoDS_Shape BuildFusedSegmentsWithFlaps(bool splitWingInUpperAndLower, std::map<std::string,double> flapStatus);

#ifdef TIGL_USE_XCAF
        // builds data structure for a TDocStd_Application
        // mostly used for export
        TDF_Label ExportDataStructure(CCPACSConfiguration& config, Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label);
#endif

        // Returns the segment for a given index or uid
        CTiglAbstractSegment & GetComponentSegment(const int index);
        CTiglAbstractSegment & GetComponentSegment(std::string uid);

        // Gets the wing transformation
        CTiglTransformation GetWingTransformation(void);

        // Get the positioning transformation for a given section uid
        CTiglTransformation GetPositioningTransformation(std::string sectionUID);

        // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
        gp_Pnt GetUpperPoint(int segmentIndex, double eta, double xsi);

        // Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
        gp_Pnt GetLowerPoint(int segmentIndex, double eta, double xsi);

        // Gets the loft of the whole wing
        TopoDS_Shape & GetLoftWithLeadingEdge(void);

        TopoDS_Shape & GetUpperShape();
        TopoDS_Shape & GetLowerShape();

        TopoDS_Shape & GetWingWithoutFlaps();

        // Gets the volume of this wing
        double GetVolume(void);

        // Get the Transformation object
        CTiglTransformation GetTransformation(void);

        // Sets a Transformation object
        void Translate(CTiglPoint trans);

        // Get Translation
        CTiglPoint GetTranslation(void);

        // Gets the surfade area of this wing
        double GetSurfaceArea();

        // Returns the reference area of the wing by taking account the drilateral portions
        // of each wing segment by projecting the wing segments into the plane defined by the user
        double GetReferenceArea(TiglSymmetryAxis symPlane);

        // Returns wetted Area
        double GetWettedArea(TopoDS_Shape parent);

        // Returns the wingspan of the wing
        double GetWingspan(void);

        // Returns the mean aerodynamic chord of the wing
        void  GetWingMAC(double& mac_chord, double& mac_x, double& mac_y, double& mac_z);

        // Calculates the segment coordinates from global (x,y,z) coordinates
        // Returns the segment index of the according segment
        // If x,y,z does not belong to any segment, -1 is returned
        int GetSegmentEtaXsi(const gp_Pnt& xyz, double& eta, double& xsi, bool &onTop);

        // Returns the Component Type TIGL_COMPONENT_WING.
        TiglGeometricComponentType GetComponentType(void) {return TIGL_COMPONENT_WING | TIGL_COMPONENT_PHYSICAL;}

        // Returns the lower Surface of a Segment
        Handle(Geom_Surface) GetLowerSegmentSurface(int index);

        // Returns the upper Surface of a Segment
        Handle(Geom_Surface) GetUpperSegmentSurface(int index);

        virtual void SetSymmetryAxis(const std::string& axis);

    protected:
        // Cleanup routine
        void Cleanup(void);

        // Build transformation matrix for the wing
        void BuildMatrix(void);

        // Update internal wing data
        void Update(void);

        // Adds all Segments of this wing to one shape
        TopoDS_Shape BuildFusedSegments(bool splitWingInUpperAndLower);

        TopoDS_Shape BuildLoft(void);

        void BuildUpperLowerShells();


    private:
        // Copy constructor
        CCPACSWing(const CCPACSWing & );

        // Assignment operator
        void operator=(const CCPACSWing & );
        double linearInterpolation(std::vector<double> list1, std::vector<double> list2, double valueRelList1);

    private:
        std::string                    name;                     /**< Wing name           */
        CCPACSWingSections             sections;                 /**< Wing sections       */
        CCPACSWingSegments             segments;                 /**< Wing segments       */
        CCPACSWingComponentSegments    componentSegments;        /**< Wing ComponentSegments */
        CCPACSWingPositionings         positionings;             /**< Wing positionings   */
        CCPACSConfiguration*           configuration;            /**< Parent configuration*/
        TopoDS_Shape                   fusedSegmentWithEdge;     /**< All Segments in one shape plus modelled leading edge */
        TopoDS_Shape                   upperShape;
        TopoDS_Shape                   lowerShape;
        TopoDS_Shape                   wingCutOutShape;          /**< Wing without flaps / flaps removed */
        TopoDS_Shape                   wingCleanShape;           /**< Clean wing surface without flaps cutout*/
        bool                           invalidated;              /**< Internal state flag */
        bool                           rebuildFusedSegments;     /**< Indicates if segmentation fusing need rebuild */
        bool                           rebuildFusedSegWEdge;     /**< Indicates if segmentation fusing need rebuild */
        bool                           rebuildShells;
        FusedElementsContainerType     fusedElements;            /**< Stores already fused segments */
        double                         myVolume;                 /**< Volume of this Wing           */
    };

} // end namespace tigl

#endif // CCPACSWING_H
