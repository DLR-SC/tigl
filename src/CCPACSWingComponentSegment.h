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
* @brief  Implementation of CPACS wing ComponentSegment handling routines.
*/

#ifndef CCPACSWINGCOMPONENTSEGMENT_H
#define CCPACSWINGCOMPONENTSEGMENT_H

#include <string>

#include "tigl_config.h"
#include "tixi.h"
#include "CCPACSWingConnection.h"
#include "CCPACSWingCSStructure.h"
#include "CTiglPoint.h"
#include "CTiglAbstractSegment.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Wire.hxx"
#include "Geom_BSplineSurface.hxx"
#include "CCPACSMaterial.h"

namespace tigl {

    class CCPACSWingSegment;

    typedef std::vector<const CCPACSMaterial*>    MaterialList;
    typedef std::vector<CCPACSWingSegment*>       SegmentList;

    class CCPACSWing;

    class CCPACSWingComponentSegment : public CTiglAbstractSegment
    {

    public:
        // Constructor
        CCPACSWingComponentSegment(CCPACSWing* aWing, int aSegmentIndex);

        // Virtual Destructor
        virtual ~CCPACSWingComponentSegment(void);

        // Invalidates internal state
        void Invalidate(void);

        // Read CPACS segment elements
        void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string & segmentXPath);

        // Returns the wing this segment belongs to
        CCPACSWing& GetWing(void) const;

        // Gets a point in relative wing coordinates for a given eta and xsi
        gp_Pnt GetPoint(double eta, double xsi);
        
        // Get the eta xsi coordinate from a segment point (given by seta, sxsi)
        void GetEtaXsiFromSegmentEtaXsi(const std::string &segmentUID, double seta, double sxsi, double &eta, double &xsi);

        // Gets the volume of this segment
        double GetVolume();

        // Gets the surface area of this segment
        double GetSurfaceArea();

        // Gets the fromElementUID of this segment
        const std::string & GetFromElementUID(void) const;

        // Gets the toElementUID of this segment
        const std::string & GetToElementUID(void) const;

        // Returns the segment to a given point on the componentSegment and the nearest point projected onto the loft.
        // Returns null if the point is not an that wing, i.e. deviates more than 1 cm from the wing
        const CTiglAbstractSegment* findSegment(double x, double y, double z, gp_Pnt& nearestPoint);

        TiglGeometricComponentType GetComponentType(){ return TIGL_COMPONENT_WINGCOMPSEGMENT | TIGL_COMPONENT_SEGMENT | TIGL_COMPONENT_LOGICAL; }

#ifdef TIGL_USE_XCAF
        // builds data structure for a TDocStd_Application
        // mostly used for export
        TDF_Label ExportDataStructure(class CCPACSConfiguration &, Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label);
#endif
        
        MaterialList GetMaterials(double eta, double xsi, TiglStructureType);

        // returns a list of segments that belong to this component segment
        SegmentList& GetSegmentList();
        
        // creates an (iso) component segment line 
        TopoDS_Wire GetCSLine(double eta1, double xsi1, double eta2, double xsi2, int NSTEPS=101);
        
        // calculates the intersection of a segment iso eta line with a component segment line (defined by its start and end point)
        // returns the xsi coordinate of the intersection
        void GetSegmentIntersection(const std::string& segmentUID, double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, double& xsi);
    protected:
        // Cleanup routine
        void Cleanup(void);

        // Update internal segment data
        void Update(void);

        // Builds the loft between the two segment sections
        TopoDS_Shape BuildLoft(void);

        // Returns an upper or lower point on the segment surface in
        // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
        // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
        // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
        // edge on the outer wing profile. If fromUpper is true, a point
        // on the upper surface is returned, otherwise from the lower.
//        gp_Pnt GetPoint(double eta, double xsi, bool fromUpper);

    private:
        // Copy constructor
        CCPACSWingComponentSegment(const CCPACSWingComponentSegment& );

        // Assignment operator
        void operator=(const CCPACSWingComponentSegment& );

        std::vector<int> findPath(const std::string& fromUid, const::std::string& toUID, const std::vector<int>& curPath, bool forward) const;

        void UpdateProjectedLeadingEdge();


    private:
        std::string          name;                 /**< Segment name                            */
        std::string          fromElementUID;       /**< Inner segment uid (root                 */
        std::string          toElementUID;         /**< Outer segment uid (tip)                 */
        CCPACSWing*          wing;                 /**< Parent wing                             */
        double               myVolume;             /**< Volume of this segment                  */
        double               mySurfaceArea;        /**< Surface area of this segment            */
        TopoDS_Shape         upperShape;           /**< Upper shape of this componentSegment    */
        TopoDS_Shape         lowerShape;           /**< Lower shape of this componentSegment    */
        TopoDS_Wire          projLeadingEdge;      /**< (Extended) Leading edge projected into y-z plane */
        SegmentList          wingSegments;         /**< List of segments belonging to the component segment */
        Handle(Geom_Surface) upperSurface;
        Handle(Geom_Surface) lowerSurface;
        bool                 surfacesAreValid;
        CCPACSWingCSStructure structure;

    };

} // end namespace tigl

#endif // CCPACSWINGCOMPONENTSEGMENT_H
