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
* @brief  Implementation of CPACS wing segment handling routines.
*/

#ifndef CCPACSWINGSEGMENT_H
#define CCPACSWINGSEGMENT_H

#include <string>

#include "generated/CPACSWingSegment.h"
#include "tigl_config.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CCPACSWingConnection.h"
#include "CCPACSGuideCurves.h"
#include "CTiglPoint.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSTransformation.h"
#include "math/CTiglPointTranslator.h"

#include "TopoDS_Shape.hxx"
#include "TopoDS_Wire.hxx"
#include "TopTools_SequenceOfShape.hxx"
#include "Geom_BSplineSurface.hxx"


namespace tigl
{

class CCPACSWing;

class CCPACSWingSegment : public generated::CPACSWingSegment, public CTiglAbstractSegment
{
public:
    enum CoordinateSystem {
        GLOBAL_COORDINATE_SYSTEM,
        WING_COORDINATE_SYSTEM
    };

public:
    // Constructor
    TIGL_EXPORT CCPACSWingSegment(CCPACSWingSegments* parent);
    TIGL_EXPORT CCPACSWingSegment(CCPACSWing* aWing, int aSegmentIndex);

    // Virtual Destructor
    TIGL_EXPORT virtual ~CCPACSWingSegment();

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& segmentXPath);

    TIGL_EXPORT virtual const std::string& GetUID() const override;
    TIGL_EXPORT virtual void SetUID(const std::string& uid) override;

    TIGL_EXPORT virtual TiglSymmetryAxis GetSymmetryAxis() override;
    TIGL_EXPORT virtual void SetSymmetryAxis(const TiglSymmetryAxis& axis) override;

    // Returns the wing this segment belongs to
    TIGL_EXPORT CCPACSWing& GetWing() const;

    TIGL_EXPORT TopoDS_Shape GetInnerClosure(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);
    TIGL_EXPORT TopoDS_Shape GetOuterClosure(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Gets the upper point in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetUpperPoint(double eta, double xsi);

    // Gets the lower point in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetLowerPoint(double eta, double xsi);

    // Gets the point on the wing chord surface in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetChordPoint(double eta, double xsi);

    // Gets the point on the wing chord surface in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetChordNormal(double eta, double xsi);

    // Returns the inner section UID of this segment
    TIGL_EXPORT const std::string& GetInnerSectionUID();

    // Returns the outer section UID of this segment
    TIGL_EXPORT const std::string& GetOuterSectionUID();

    // Returns the inner section element UID of this segment
    TIGL_EXPORT const std::string& GetInnerSectionElementUID() const;

    // Returns the outer section element UID of this segment
    TIGL_EXPORT const std::string& GetOuterSectionElementUID() const;

    // Returns the inner section index of this segment
    TIGL_EXPORT int GetInnerSectionIndex();

    // Returns the outer section index of this segment
    TIGL_EXPORT int GetOuterSectionIndex();

    // Returns the inner section element index of this segment
    TIGL_EXPORT int GetInnerSectionElementIndex();

    // Returns the outer section element index of this segment
    TIGL_EXPORT int GetOuterSectionElementIndex();

    // Returns the starting(inner) Segment Connection
    TIGL_EXPORT CCPACSWingConnection& GetInnerConnection();

    // Return the end(outer) Segment Connection
    TIGL_EXPORT CCPACSWingConnection& GetOuterConnection();

    // Gets the count of segments connected to the inner section of this segment
    TIGL_EXPORT int GetInnerConnectedSegmentCount();

    // Gets the count of segments connected to the outer section of this segment
    TIGL_EXPORT int GetOuterConnectedSegmentCount();

    // Gets the index (number) of the n-th segment connected to the inner section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetInnerConnectedSegmentIndex(int n);

    // Gets the index (number) of the n-th segment connected to the outer section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetOuterConnectedSegmentIndex(int n);

    // Gets the volume of this segment
    TIGL_EXPORT double GetVolume();

    // Gets the surface area of this segment
    TIGL_EXPORT double GetSurfaceArea();
    
    TIGL_EXPORT double GetSurfaceArea(bool fromUpper, 
                                      double eta1, double xsi1,
                                      double eta2, double xsi2,
                                      double eta3, double xsi3,
                                      double eta4, double xsi4);

    // helper function to get the inner transformed chord line wire, used in GetLoft and when determining triangulation midpoints projection on segments in VtkExport
    TIGL_EXPORT TopoDS_Wire GetInnerWire(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // helper function to get the outer transformed chord line wire, used in GetLoft and when determining triangulation midpoints projection on segments in VtkExport
    TIGL_EXPORT TopoDS_Wire GetOuterWire(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Getter for inner wire of opened profile (containing trailing edge)
    TIGL_EXPORT TopoDS_Wire GetInnerWireOpened(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Getter for outer wire of opened profile (containing trailing edge)
    TIGL_EXPORT TopoDS_Wire GetOuterWireOpened(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Returns eta as parametric distance from a given point on the surface
    // Get information about a point beeing on upper/lower side with "GetIsOnTop"
    TIGL_EXPORT double GetEta(gp_Pnt pnt, bool isUpper);

    // calculates eta from a given XSI and
    TIGL_EXPORT double GetEta(gp_Pnt pnt, double xsi);

    // Returns zeta as parametric distance from a given point on the surface
    // Get information about a point beeing on upper/lower side with "GetIsOnTop"
    TIGL_EXPORT double GetXsi(gp_Pnt pnt, bool isUpper);

    // projects a point unto the wing and returns its coordinates
    TIGL_EXPORT void GetEtaXsi(gp_Pnt pnt, double& eta, double& xsi);


    // Returns if the given point is ont the Top of the wing or on the lower side.
    TIGL_EXPORT bool GetIsOnTop(gp_Pnt pnt);

    // Returns the reference area of the quadrilateral portion of the wing segment
    // by projecting the wing segment into the plane defined by the user
    TIGL_EXPORT double GetReferenceArea(TiglSymmetryAxis symPlane);

    // Returns the lower Surface of this Segment
    TIGL_EXPORT Handle(Geom_Surface) GetLowerSurface(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Returns the upper Surface of this Segment
    TIGL_EXPORT Handle(Geom_Surface) GetUpperSurface(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    TIGL_EXPORT TopoDS_Shape& GetUpperShape(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);
    TIGL_EXPORT TopoDS_Shape& GetLowerShape(CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Returns the guide curves of the segment as wires
    TIGL_EXPORT TopTools_SequenceOfShape& GetGuideCurveWires();

    // get guide curve for given UID
    TIGL_EXPORT const CCPACSGuideCurve& GetGuideCurve(std::string UID);

    // check if guide curve with a given UID exists
    TIGL_EXPORT bool GuideCurveExists(std::string UID);

    // Returns an upper or lower point on the segment surface in
    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
    // edge on the outer wing profile. If fromUpper is true, a point
    // on the upper surface is returned, otherwise from the lower.
    TIGL_EXPORT gp_Pnt GetPoint(double eta, double xsi, bool fromUpper, CoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM);

    // Returns an upper or lower point on the segment surface in
    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
    // edge on the outer wing profile. If fromUpper is true, a point
    // on the upper surface is returned, otherwise from the lower.
    // The intersection is with the wing surface is computed with a line
    // starting at a point on the chord face with the given direction dir.
    TIGL_EXPORT gp_Pnt GetPointDirection(double eta, double xsi, double dirx, double diry, double dirz, bool fromUpper, double& deviation);

    TIGL_EXPORT TiglGeometricComponentType GetComponentType()
    {
        return TIGL_COMPONENT_WINGSEGMENT | TIGL_COMPONENT_SEGMENT | TIGL_COMPONENT_LOGICAL;
    }

    // Getter for the number of guide curves
    TIGL_EXPORT int GetGuideCurveCount() const;

protected:
    // Cleanup routine
    void Cleanup();

    // Update internal segment data
    void Update();

    // builds all guide curve wires
    void BuildGuideCurveWires();

    // Builds the loft between the two segment sections
    PNamedShape BuildLoft();

private:
    // get short name for loft
    std::string GetShortShapeName ();

    // Builds upper and lower surfaces
    void MakeSurfaces();
    
    // converts segment eta xsi coordinates to face uv koordinates
    void etaXsiToUV(bool isFromUpper, double eta, double xsi, double& u, double& v);

    CCPACSTransformation dummyTrans; // TODO: CCPACSWingSegment must provide a CCPACSTransformation as it is a CTiglAbstractGeometricalComponent, is this correct? Can we remove the base class CTiglAbstractGeometricalComponent?
    CCPACSWingConnection innerConnection;      /**< Inner segment connection (root)         */
    CCPACSWingConnection outerConnection;      /**< Outer segment connection (tip)          */
    TopTools_SequenceOfShape guideCurveWires;  /**< container for the guide curve wires     */
    CCPACSWing*          wing;                 /**< Parent wing                             */
    double               myVolume;             /**< Volume of this segment                  */
    double               mySurfaceArea;        /**< Surface area of this segment            */
    TopoDS_Shape         upperShape;           /**< Upper shape of this segment             */
    TopoDS_Shape         lowerShape;           /**< Lower shape of this segment             */
    TopoDS_Shape         upperShapeLocal;      /**< Upper shape of this segment in wing coordinate system */
    TopoDS_Shape         lowerShapeLocal;      /**< Lower shape of this segment in wing coordinate system */
    TopoDS_Shape         upperShapeOpened;
    TopoDS_Shape         lowerShapeOpened;
    TopoDS_Shape         trailingEdgeShape;
    Handle(Geom_Surface) upperSurface;
    Handle(Geom_Surface) lowerSurface;
    Handle(Geom_Surface) upperSurfaceLocal;
    Handle(Geom_Surface) lowerSurfaceLocal;
    CTiglPointTranslator cordSurface;
    bool                 surfacesAreValid;
};

} // end namespace tigl

#endif // CCPACSWINGSEGMENT_H

