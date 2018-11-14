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
* @brief  Implementation of rotor blade handling routines.
*/

#include "CTiglAttachedRotorBlade.h"
#include "CCPACSRotorHinge.h"
#include "CCPACSRotorBladeAttachment.h"
#include "CCPACSWing.h"
#include "CCPACSRotor.h"
#include "CTiglError.h"
#include "CNamedShape.h"
#include "math/tiglmathfunctions.h"

#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "BRepPrimAPI_MakeRevol.hxx"

namespace tigl
{

// Constructor
CTiglAttachedRotorBlade::CTiglAttachedRotorBlade(CCPACSRotorBladeAttachment* parent, CCPACSWing& rotorBlade, int index)
    : CTiglRelativelyPositionedComponent(static_cast<std::string*>(NULL), &transformation)
    , parent(parent)
    , rotorBlade(&rotorBlade)
    , rotorBladeIndex(index)
    , rebuildRotorDisk(true)
    , invalidated(true)
    , transformation(NULL)
{
    Update();
}

std::string CTiglAttachedRotorBlade::GetDefaultedUID() const {
    return rotorBlade->GetUID();
}

// Invalidates internal state
void CTiglAttachedRotorBlade::Invalidate()
{
    rebuildRotorDisk = true;
}

// Cleanup routine
void CTiglAttachedRotorBlade::Cleanup()
{
    Reset();
    Invalidate();
}

// Builds transformation matrix for the rotor blade including rotor transformation
void CTiglAttachedRotorBlade::BuildMatrix()
{
    const double thetaDeg = 0.; // current azimuthal position of the rotor in degrees
    const double azimuth = GetAzimuthAngle();
    transformation.setTransformationMatrix(parent->GetRotorBladeTransformationMatrix(thetaDeg, azimuth, true, true, true));
}

// Update internal rotor blade data
void CTiglAttachedRotorBlade::Update()
{
    if (!invalidated) {
        return;
    }

    BuildMatrix();
    invalidated = false;
}

// Returns the original unattached rotor blade
CCPACSWing& CTiglAttachedRotorBlade::GetUnattachedRotorBlade() const
{
    return *rotorBlade;
}

// Returns the parent rotor blade attachment this rotor blade belongs to
CCPACSRotorBladeAttachment& CTiglAttachedRotorBlade::GetRotorBladeAttachment() const
{
    return *parent;
}

// Returns the parent rotor
CCPACSRotor& CTiglAttachedRotorBlade::GetRotor() const
{
    return parent->GetRotor();
}

// Returns the parent configuration
CCPACSConfiguration& CTiglAttachedRotorBlade::GetConfiguration() const
{
    return parent->GetConfiguration();
}

// Returns the azimuth angle of this rotor blade
double CTiglAttachedRotorBlade::GetAzimuthAngle() const
{
    return parent->GetAzimuthAngle(rotorBladeIndex);
}

// Returns the volume of this rotor blade
double CTiglAttachedRotorBlade::GetVolume()
{
    TopoDS_Shape fusedSegments = GetLoft()->Shape();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Returns the surface area of this rotor blade
double CTiglAttachedRotorBlade::GetSurfaceArea()
{
    TopoDS_Shape fusedSegments = GetLoft()->Shape();

    // Calculate surface area
    GProp_GProps System;
    BRepGProp::SurfaceProperties(fusedSegments, System);
    double myArea = System.Mass();
    return myArea;
}

// Returns the planform area of this rotor blade
double CTiglAttachedRotorBlade::GetPlanformArea()
{
    double planformArea = 0.0;

    if (rotorBlade->GetSegmentCount() < 1) {
        return planformArea;
    }

    CTiglTransformation bladeTransformation = parent->GetRotorBladeTransformationMatrix(0., 0., false, false, false);

    for (int i=1; i<=rotorBlade->GetSegmentCount(); ++i) {
        // Get corner points of the segment
        gp_Pnt P1 = bladeTransformation.Transform(rotorBlade->GetChordPoint(i, 1., 0.)).XYZ();
        gp_Pnt P2 = bladeTransformation.Transform(rotorBlade->GetChordPoint(i, 1., 1.)).XYZ();
        gp_Pnt P3 = bladeTransformation.Transform(rotorBlade->GetChordPoint(i, 0., 1.)).XYZ();
        gp_Pnt P4 = bladeTransformation.Transform(rotorBlade->GetChordPoint(i, 0., 0.)).XYZ();

        // Project points to xy-plane
        P1.SetZ(0.);
        P2.SetZ(0.);
        P3.SetZ(0.);
        P4.SetZ(0.);

        // Apply rotor transformation, calculate and add segment planform area
        const CTiglTransformation& rotorTrafo = parent->GetRotor().GetTransformationMatrix();
        planformArea += quadrilateral_area(rotorTrafo.Transform(P1).XYZ(),
                                           rotorTrafo.Transform(P2).XYZ(),
                                           rotorTrafo.Transform(P3).XYZ(),
                                           rotorTrafo.Transform(P4).XYZ());
    }

    return planformArea;
}

// Returns the radius of this rotor blade by calculating the maximum distance of quarter chord points from the rotor axis
double CTiglAttachedRotorBlade::GetRadius()
{
    double rotorRadius = 0.0;

    if (rotorBlade->GetSegmentCount() < 1) {
        return rotorRadius;
    }

    CTiglPoint rotorAxisOrigin(    parent->GetRotor().GetTransformationMatrix().Transform(gp_Pnt(0., 0., 0.)).XYZ() );
    CTiglPoint rotorAxisDirection( parent->GetRotor().GetTransformationMatrix().Transform(gp_Pnt(0., 0., 1.)).XYZ() );
    rotorAxisDirection -= rotorAxisOrigin;

    gp_Pnt maxDistPoint;
    double curDist;

    CTiglTransformation bladeTransformation = parent->GetRotorBladeTransformationMatrix(0., 0., false, false, true);
    CTiglPoint quarterChordPointTransformed = bladeTransformation.Transform(rotorBlade->GetChordPoint(1, 0., 0.25)).XYZ();
    curDist = distance_point_from_line(quarterChordPointTransformed, rotorAxisOrigin, rotorAxisDirection);
    rotorRadius = curDist;
    for (int i=1; i<=rotorBlade->GetSegmentCount(); ++i) {
        quarterChordPointTransformed = bladeTransformation.Transform(rotorBlade->GetChordPoint(i, 1., 0.25)).XYZ();
        curDist = distance_point_from_line(quarterChordPointTransformed, rotorAxisOrigin, rotorAxisDirection);
        if (curDist > rotorRadius) {
            rotorRadius = curDist;
        }
    }

    return rotorRadius;
}

// Returns the tip speed this rotor blade
double CTiglAttachedRotorBlade::GetTipSpeed()
{
    // return GetRotor().GetNominalRotationsPerMinute()/60. * 2.*M_PI*GetRadius();
    return *GetRotor().GetNominalRotationsPerMinute() / 30. * M_PI * GetRadius();
}

// Returns the radius of a point on the rotor blade quarter chord line for a given segment index and eta
double CTiglAttachedRotorBlade::GetLocalRadius(const int& segmentIndex, const double& eta)
{
    double radius = 0.0;

    if (rotorBlade->GetSegmentCount() < 1) {
        return radius;
    }

    CTiglPoint rotorAxisOrigin(    parent->GetRotor().GetTransformationMatrix().Transform(gp_Pnt(0., 0., 0.)).XYZ() );
    CTiglPoint rotorAxisDirection( parent->GetRotor().GetTransformationMatrix().Transform(gp_Pnt(0., 0., 1.)).XYZ() );
    rotorAxisDirection -= rotorAxisOrigin;

    CTiglTransformation bladeTransformation = parent->GetRotorBladeTransformationMatrix(0., 0., false, false, true);
    CTiglPoint quarterChordPointTransformed = bladeTransformation.Transform(rotorBlade->GetChordPoint(segmentIndex, eta, 0.25)).XYZ();
    radius = distance_point_from_line(quarterChordPointTransformed, rotorAxisOrigin, rotorAxisDirection);

    return radius;
}

// Returns the rotor blade chord length for a given segment index and eta
double CTiglAttachedRotorBlade::GetLocalChord(const int& segmentIndex, const double& eta)
{
    double chordLength = 0.0;

    if (rotorBlade->GetSegmentCount() < 1) {
        return chordLength;
    }

    CTiglTransformation bladeTransformation = parent->GetRotorBladeTransformationMatrix(0., 0., false, false, true);
    gp_Pnt LePoint = bladeTransformation.Transform(rotorBlade->GetChordPoint(segmentIndex, eta, 0.0));
    gp_Pnt TePoint = bladeTransformation.Transform(rotorBlade->GetChordPoint(segmentIndex, eta, 1.0));
    chordLength = LePoint.Distance(TePoint);

    return chordLength;
}

// Returns the local rotor blade twist angle (in degrees) for a given segment index and eta
double CTiglAttachedRotorBlade::GetLocalTwistAngle(const int& segmentIndex, const double& eta)
{
    double twistAngle = 0.0;

    if (rotorBlade->GetSegmentCount() < 1) {
        return twistAngle;
    }

    CTiglPoint rotorAxisOrigin(    parent->GetRotor().GetTransformationMatrix().Transform(gp_Pnt(0., 0., 0.)).XYZ() );
    CTiglPoint rotorAxisDirection( parent->GetRotor().GetTransformationMatrix().Transform(gp_Pnt(0., 0., 1.)).XYZ() );
    rotorAxisDirection -= rotorAxisOrigin;

    CTiglTransformation bladeTransformation = parent->GetRotorBladeTransformationMatrix(0., 0., false, false, true);
    CTiglPoint LePoint = bladeTransformation.Transform(rotorBlade->GetChordPoint(segmentIndex, eta, 0.00)).XYZ();
    CTiglPoint QcPoint = bladeTransformation.Transform(rotorBlade->GetChordPoint(segmentIndex, eta, 0.25)).XYZ();
    CTiglPoint TePoint = bladeTransformation.Transform(rotorBlade->GetChordPoint(segmentIndex, eta, 1.00)).XYZ();
    CTiglPoint radialDirection = QcPoint - (rotorAxisOrigin + CTiglPoint::vector_projection(QcPoint-rotorAxisOrigin, rotorAxisDirection));
    CTiglPoint zeroTwistDirection = CTiglPoint::cross_prod(radialDirection, rotorAxisDirection);
    CTiglPoint chordLine = TePoint - LePoint;
    twistAngle = - atan2( CTiglPoint::scalar_projection(chordLine,rotorAxisDirection), CTiglPoint::scalar_projection(chordLine,zeroTwistDirection) ) * 180./M_PI;

    return twistAngle;
}


// Create the rotor blade geometry by copying and transforming the original unattached rotor blade geometry
PNamedShape CTiglAttachedRotorBlade::BuildLoft() const
{
    // Create a new instance of the referenced unattached rotor blade and apply the transformations to it
    PNamedShape rotorBladeCopy = rotorBlade->GetLoft()->DeepCopy();
    TopoDS_Shape transformedShape = transformation.getTransformationMatrix().Transform(rotorBladeCopy->Shape());
    rotorBladeCopy->SetShape(transformedShape);
    return rotorBladeCopy;
}

// Returns the rotor disk geometry
TopoDS_Shape CTiglAttachedRotorBlade::GetRotorDisk()
{
    if (rebuildRotorDisk) {
        rotorDisk = BuildRotorDisk();
        rebuildRotorDisk = false;
    }
    return rotorDisk;
}

CTiglTransformation CTiglAttachedRotorBlade::GetTransformationMatrix() const
{
    const_cast<CTiglAttachedRotorBlade*>(this)->Update(); // TODO: hack
    return transformation.getTransformationMatrix();
}

// Creates a rotor disk (only using information of the current blade)
TopoDS_Shape CTiglAttachedRotorBlade::BuildRotorDisk()
{

    double thetaDeg = 0.; // current azimuthal position of the rotor in degrees

    TopoDS_Shape rotorDisk;

    // Make PolyLine quarterChordLine: BRepBuilderAPI_MakePolygon;
    if (rotorBlade->GetSegmentCount() < 1) {
        return rotorDisk;
    }
    BRepBuilderAPI_MakePolygon P;
    P.Add(rotorBlade->GetChordPoint(1, 0., 0.25));
    for (int i=1; i<=rotorBlade->GetSegmentCount(); ++i) {
        P.Add(rotorBlade->GetChordPoint(i, 1., 0.25));
    }
    TopoDS_Shape quarterChordLine = P.Shape();

    // Apply blade transformations without rotor transformation
    quarterChordLine = parent->GetRotorBladeTransformationMatrix(thetaDeg, GetAzimuthAngle(), true, true, false).Transform(quarterChordLine);

    // Make surface of Revolution from PolyLine
    gp_Ax1 axis(gp_Pnt(0., 0., 0.), gp_Dir(0., 0., 1.));
    rotorDisk = BRepPrimAPI_MakeRevol(quarterChordLine, axis);

    // Apply rotor transformation
    rotorDisk = parent->GetRotor().GetTransformationMatrix().Transform(rotorDisk);

    // Return the generated geometry
    return rotorDisk;
}

} // end namespace tigl
