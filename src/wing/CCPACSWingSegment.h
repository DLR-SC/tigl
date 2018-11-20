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
* @brief  Implementation of CPACS wing segment handling routines.
*/

#ifndef CCPACSWINGSEGMENT_H
#define CCPACSWINGSEGMENT_H

#include <string>

#include "generated/CPACSWingSegment.h"
#include "tigl_config.h"
#include "tigl_internal.h"
#include "tixi.h"
#include "CTiglWingConnection.h"
#include "CCPACSGuideCurves.h"
#include "CCPACSGuideCurve.h"
#include "CTiglPoint.h"
#include "CTiglAbstractSegment.h"
#include "Cache.h"
#include "CCPACSTransformation.h"
#include "math/CTiglPointTranslator.h"

#include <TopoDS_Face.hxx>
#include "TopoDS_Shape.hxx"
#include "TopoDS_Wire.hxx"
#include "TopTools_SequenceOfShape.hxx"
#include "Geom_BSplineSurface.hxx"
#include "Cache.h"


namespace tigl
{

class CCPACSWing;

class CCPACSWingSegment : public generated::CPACSWingSegment, public CTiglAbstractSegment<CCPACSWingSegment>
{
public:
    // Constructor
    TIGL_EXPORT CCPACSWingSegment(CCPACSWingSegments* parent, CTiglUIDManager* uidMgr);

    // Virtual Destructor
    TIGL_EXPORT ~CCPACSWingSegment() OVERRIDE;

    // Invalidates internal state
    TIGL_EXPORT void Invalidate();

    // Read CPACS segment elements
    TIGL_EXPORT void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& segmentXPath) OVERRIDE;

    TIGL_EXPORT std::string GetDefaultedUID() const OVERRIDE;

    TIGL_EXPORT void SetFromElementUID(const std::string& value) OVERRIDE;
    TIGL_EXPORT void SetToElementUID(const std::string& value) OVERRIDE;

    TIGL_EXPORT TopoDS_Shape GetInnerClosure(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                             TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Shape GetOuterClosure(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                             TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;

    // Returns points on the inner/outer profile without building the shape
    TIGL_EXPORT gp_Pnt GetInnerProfilePoint(double xsi) const;
    TIGL_EXPORT gp_Pnt GetOuterProfilePoint(double xsi) const;

    // Gets the upper point in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetUpperPoint(double eta, double xsi) const;

    // Gets the lower point in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetLowerPoint(double eta, double xsi) const;

    // Gets the point on the wing chord surface in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetChordPoint(double eta, double xsi) const;

    // Gets the point on the wing chord surface in relative wing coordinates for a given eta and xsi
    TIGL_EXPORT gp_Pnt GetChordNormal(double eta, double xsi) const;

    // Returns the inner section UID of this segment
    TIGL_EXPORT const std::string& GetInnerSectionUID() const;

    // Returns the outer section UID of this segment
    TIGL_EXPORT const std::string& GetOuterSectionUID() const;

    // Returns the inner section element UID of this segment
    TIGL_EXPORT const std::string& GetInnerSectionElementUID() const;

    // Returns the outer section element UID of this segment
    TIGL_EXPORT const std::string& GetOuterSectionElementUID() const;

    // Returns the inner section index of this segment
    TIGL_EXPORT int GetInnerSectionIndex() const;

    // Returns the outer section index of this segment
    TIGL_EXPORT int GetOuterSectionIndex() const;

    // Returns the inner section element index of this segment
    TIGL_EXPORT int GetInnerSectionElementIndex() const;

    // Returns the outer section element index of this segment
    TIGL_EXPORT int GetOuterSectionElementIndex() const;

    // Returns the starting(inner) Segment Connection
    TIGL_EXPORT CTiglWingConnection& GetInnerConnection();
    TIGL_EXPORT const CTiglWingConnection& GetInnerConnection() const;

    // Return the end(outer) Segment Connection
    TIGL_EXPORT CTiglWingConnection& GetOuterConnection();
    TIGL_EXPORT const CTiglWingConnection& GetOuterConnection() const;

    // Gets the count of segments connected to the inner section of this segment
    TIGL_EXPORT int GetInnerConnectedSegmentCount() const;

    // Gets the count of segments connected to the outer section of this segment
    TIGL_EXPORT int GetOuterConnectedSegmentCount() const;

    // Gets the index (number) of the n-th segment connected to the inner section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetInnerConnectedSegmentIndex(int n) const;

    // Gets the index (number) of the n-th segment connected to the outer section
    // of this segment. n starts at 1.
    TIGL_EXPORT int GetOuterConnectedSegmentIndex(int n) const;

    // Gets the volume of this segment
    TIGL_EXPORT double GetVolume();

    // Gets the surface area of this segment
    TIGL_EXPORT double GetSurfaceArea() const;
    
    TIGL_EXPORT double GetSurfaceArea(bool fromUpper, 
                                      double eta1, double xsi1,
                                      double eta2, double xsi2,
                                      double eta3, double xsi3,
                                      double eta4, double xsi4) const;

    // helper function to get the inner transformed chord line wire, used in GetLoft and when determining triangulation midpoints projection on segments in VtkExport
    TIGL_EXPORT TopoDS_Wire GetInnerWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                         TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;

    // helper function to get the outer transformed chord line wire, used in GetLoft and when determining triangulation midpoints projection on segments in VtkExport
    TIGL_EXPORT TopoDS_Wire GetOuterWire(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                         TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;


    // projects a point unto the wing and returns its coordinates
    TIGL_EXPORT void GetEtaXsi(gp_Pnt pnt, double& eta, double& xsi) const;


    // Returns if the given point is ont the Top of the wing or on the lower side.
    TIGL_EXPORT bool GetIsOnTop(gp_Pnt pnt) const;

    // return if pnt lies on the loft or on the segment chord face
    TIGL_EXPORT bool GetIsOn(const gp_Pnt &pnt) OVERRIDE;

    // Returns the reference area of the quadrilateral portion of the wing segment
    // by projecting the wing segment into the plane defined by the user
    TIGL_EXPORT double GetReferenceArea(TiglSymmetryAxis symPlane) const;

    using CTiglAbstractSegment<CCPACSWingSegment>::GetLoft;
    TIGL_EXPORT PNamedShape GetLoft(TiglShapeModifier mod) const;

    // Returns the lower Surface of this Segment
    TIGL_EXPORT Handle(Geom_Surface) GetLowerSurface(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                                     TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;

    // Returns the upper Surface of this Segment
    TIGL_EXPORT Handle(Geom_Surface) GetUpperSurface(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                                     TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;

    TIGL_EXPORT TopoDS_Shape GetUpperShape(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                           TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Shape GetLowerShape(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                           TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;
    TIGL_EXPORT TopoDS_Shape GetTrailingEdgeShape(TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM,
                                                  TiglShapeModifier mod            = UNMODIFIED_SHAPE) const;

    // Returns an upper or lower point on the segment surface in
    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
    // edge on the outer wing profile. If fromUpper is true, a point
    // on the upper surface is returned, otherwise from the lower.
    TIGL_EXPORT gp_Pnt GetPoint(double eta, double xsi, bool fromUpper, TiglCoordinateSystem referenceCS = GLOBAL_COORDINATE_SYSTEM) const;

    // Returns an upper or lower point on the segment surface in
    // dependence of parameters eta and xsi, which range from 0.0 to 1.0.
    // For eta = 0.0, xsi = 0.0 point is equal to leading edge on the
    // inner wing profile. For eta = 1.0, xsi = 1.0 point is equal to the trailing
    // edge on the outer wing profile. If fromUpper is true, a point
    // on the upper surface is returned, otherwise from the lower.
    // The intersection is with the wing surface is computed with a line
    // starting at a point on the chord face with the given direction dir.
    TIGL_EXPORT gp_Pnt GetPointDirection(double eta, double xsi, double dirx, double diry, double dirz, bool fromUpper, double& deviation) const;

    TIGL_EXPORT TiglGeometricComponentType GetComponentType() const OVERRIDE
    {
        return TIGL_COMPONENT_WINGSEGMENT;
    }

    TIGL_EXPORT TiglGeometricComponentIntent GetComponentIntent() const OVERRIDE
    {
        return TIGL_INTENT_LOGICAL;
    }

    TIGL_EXPORT CTiglTransformation GetParentTransformation() const;

protected:
    // Cleanup routine
    void Cleanup();

    // Update internal segment data
    void Update();

    // Builds the loft between the two segment sections
    PNamedShape BuildLoft() const OVERRIDE;

private:
    struct SurfaceCoordCache
    {
        CTiglPointTranslator cordSurface;
        Handle(Geom_Surface) cordFace;
    };

    // get short name for loft
    std::string GetShortShapeName () const;

    // Builds upper and lower surfaces
    void ComputeArea(double& cache) const;

    // Builds the chord surface
    void MakeChordSurface(SurfaceCoordCache& cache) const;

    void ComputeVolume(double& cache) const;

    // Returns the chord surface (and builds it if required)
    const CTiglPointTranslator& ChordFace() const;


    // converts segment eta xsi coordinates to face uv koordinates
    void etaXsiToUV(bool isFromUpper, double eta, double xsi, double& u, double& v) const;

    CTiglWingConnection  innerConnection;      /**< Inner segment connection (root)         */
    CTiglWingConnection  outerConnection;      /**< Outer segment connection (tip)          */

    bool                 loftLinearly = false; /**< Set to true to speed up lofting of the
                                                 * segment. This removes the dependency on
                                                 * the fuselage loft at the price of a
                                                 * nonsmooth fuselage                       */

    Cache<SurfaceCoordCache, CCPACSWingSegment> surfaceCoordCache;
    Cache<double, CCPACSWingSegment>            areaCache;
    Cache<double, CCPACSWingSegment> volumeCache;

    unique_ptr<IGuideCurveBuilder> m_guideCurveBuilder;
};

} // end namespace tigl

#endif // CCPACSWINGSEGMENT_H

