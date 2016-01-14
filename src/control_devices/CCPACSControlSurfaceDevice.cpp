/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#include <iostream>
#include <sstream>
#include <exception>
#include <cassert>
#include <algorithm>

#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglControlSurfaceHingeLine.h"
#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "CCPACSControlSurfaceDeviceBorderLeadingEdgeShape.h"
#include "CTiglControlSurfaceTransformation.h"
#include "CCPACSControlSurfaceDeviceSteps.h"
#include "tiglcommonfunctions.h"

#include "Geom_Plane.hxx"
#include "Geom_Surface.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepPrimAPI_MakePrism.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "gp_Pln.hxx"
#include "Geom2dAPI_Interpolate.hxx"
#include "TColgp_HArray1OfPnt2d.hxx"
#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepTools.hxx"
#include "BRepLib.hxx"
#include "TopoDS_Edge.hxx"
#include "Geom2d_Curve.hxx"
#include "Geom2d_BSplineCurve.hxx"

#define USE_ADVANCED_MODELING 1

namespace
{
    TopoDS_Edge pointsToEdge(Handle(Geom_Surface) surf, gp_Pnt2d p1, gp_Pnt2d p2)
    {
        gp_Pnt p13d = surf->Value(p1.X(), p1.Y());
        gp_Pnt p23d = surf->Value(p2.X(), p2.Y());
        BRepBuilderAPI_MakeEdge edgemaker(p13d, p23d);
        TopoDS_Edge edge =  edgemaker.Edge();
        return edge;
    }
}


namespace tigl
{

CCPACSControlSurfaceDevice::CCPACSControlSurfaceDevice(CCPACSWingComponentSegment* segment)
    : _segment(segment)
{
    SetUID("ControlDevice");
    _hingeLine = CSharedPtr<CTiglControlSurfaceHingeLine>(new CTiglControlSurfaceHingeLine(&outerShape, &path,_segment));
}

// Read CPACS trailingEdgeDevice elements
void CCPACSControlSurfaceDevice::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& controlSurfaceDeviceXPath, TiglControlSurfaceType type)
{
    char*       elementPath;
    std::string tempString;

    // Get sublement "outerShape"
    char* ptrName = NULL;
    tempString    = controlSurfaceDeviceXPath + "/outerShape";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        outerShape.ReadCPACS(tixiHandle, elementPath, type);
    }

    // Get Path
    tempString = controlSurfaceDeviceXPath + "/path";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        path.ReadCPACS(tixiHandle, elementPath);
    }


    char* ptrUID = NULL;
    if (tixiGetTextAttribute(tixiHandle, controlSurfaceDeviceXPath.c_str(), "uID", &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get WingCutOut, this is not implemented yet.
    tempString = controlSurfaceDeviceXPath + "/wingCutOut";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        wingCutOut.ReadCPACS(tixiHandle, elementPath);
    }

    _type = type;
}

void CCPACSControlSurfaceDevice::setLoft(PNamedShape loft)
{
    this->loft = loft;
}

PNamedShape CCPACSControlSurfaceDevice::BuildLoft()
{
    return loft;
}

const CCPACSControlSurfaceDeviceOuterShape& CCPACSControlSurfaceDevice::getOuterShape() const
{
    return outerShape;
}

const CCPACSControlSurfaceDevicePath& CCPACSControlSurfaceDevice::getMovementPath() const
{
    return path;
}

gp_Trsf CCPACSControlSurfaceDevice::GetFlapTransform(double deflection) const
{
    // this block of code calculates all needed values to rotate and move the controlSurfaceDevice according
    // to the given relDeflection by using a linearInterpolation.
    std::vector<double> relDeflections, innerXTrans, outerXTrans, innerYTrans, innerZTrans, outerZTrans, rotations;
    
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    for (int istep = 1; istep <= steps.GetStepCount(); ++istep) {
        CCPACSControlSurfaceDeviceStep& step = steps.GetStep(istep);
        relDeflections.push_back(step.getRelDeflection());
        innerXTrans.push_back(step.getInnerHingeTranslation().x);
        innerYTrans.push_back(step.getInnerHingeTranslation().y);
        innerZTrans.push_back(step.getInnerHingeTranslation().z);
        outerXTrans.push_back(step.getOuterHingeTranslation().x);
        outerZTrans.push_back(step.getOuterHingeTranslation().z);
        rotations.push_back(step.getHingeLineRotation());
    }
    
    double rotation = Interpolate( relDeflections, rotations, deflection );
    double innerTranslationX = Interpolate( relDeflections, innerXTrans, deflection );
    double innerTranslationY = Interpolate( relDeflections, innerYTrans, deflection );
    double innerTranslationZ = Interpolate( relDeflections, innerZTrans, deflection );
    double outerTranslationX = Interpolate( relDeflections, outerXTrans, deflection );
    double outerTranslationZ = Interpolate( relDeflections, outerZTrans, deflection );

    gp_Pnt innerHingeOld = _hingeLine->getInnerHingePoint();;
    gp_Pnt outerHingeOld = _hingeLine->getOuterHingePoint();;

    // innerTranslationY on hingePoint1 on purpose, maybe consider setting it to zero as default. See CPACS definition on
    // Path/Step/HingeLineTransformation for more informations.
    gp_Pnt hingePoint1 = _hingeLine->getOuterHingePoint().XYZ() + gp_XYZ(outerTranslationX, innerTranslationY, outerTranslationZ);
    gp_Pnt hingePoint2 = _hingeLine->getInnerHingePoint().XYZ() + gp_XYZ(innerTranslationX, innerTranslationY, innerTranslationZ);

    // calculating the needed transformations
    CTiglControlSurfaceTransformation transformation(innerHingeOld, outerHingeOld, hingePoint2, hingePoint1, rotation);

    return transformation.getTotalTransformation();
}

PNamedShape CCPACSControlSurfaceDevice::getCutOutShape()
{
    /*
     * If this is true, then there is an optional more detailed definition of the controlSurfaceDevice, that
     * can be created. This is true when a trailingEdgeDevice has a leadingEgeShape Defined in his OuterShape.
     * And if the leadingEdge Eta is the same as the trailingEdge Eta. Even if Etas arent the same, the Algorithm
     * would work, but the results wouldnt be 100% Correct.
     */
#if USE_ADVANCED_MODELING
    if (_type == TRAILING_EDGE_DEVICE && getOuterShape().getInnerBorder().isLeadingEdgeShapeAvailible()
            && getOuterShape().getOuterBorder().isLeadingEdgeShapeAvailible() &&
            getOuterShape().getOuterBorder().getEtaLE() == getOuterShape().getOuterBorder().getEtaTE() &&
            getOuterShape().getInnerBorder().getEtaLE() == getOuterShape().getInnerBorder().getEtaTE()) {


        // Get Wires definng the Shape of the more complex CutOutShape.
        TopoDS_Wire innerWire = getCutoutWire(true);
        TopoDS_Wire outerWire = getCutoutWire(false);

        // make one shape out of the 2 wires and build connections inbetween.
        BRepOffsetAPI_ThruSections thrusections(true,true);
        thrusections.AddWire(outerWire);
        thrusections.AddWire(innerWire);
        thrusections.Build();

        loft = PNamedShape(new CNamedShape(thrusections.Shape(), GetUID().c_str()));
        loft->SetShortName(GetShortShapeName().c_str());

#ifdef DEBUG
        std::stringstream filenamestr;
        filenamestr << GetUID() << "_cutout.brep";
        BRepTools::Write(loft->Shape(), filenamestr.str().c_str());
#endif

        return loft;
    }
    else {
#else
    {
#endif
        // Build Simple Flap Geometry.
        TopoDS_Face face = GetBasePlane();
        gp_Vec vec = getNormalOfControlSurfaceDevice();

        if (_type == SPOILER) {
            /*
             * This has to be adjusted if Spoilers should get implemented.
             * This is just a Placeholder.
             */
            vec.Multiply(determineSpoilerThickness()*100);
        }
        else {
            vec.Multiply(determineCutOutPrismThickness()*2);
        }

        TopoDS_Shape prism = BRepPrimAPI_MakePrism(face,vec);
        loft = PNamedShape(new CNamedShape(prism, GetUID().c_str()));
        loft->SetShortName(GetShortShapeName().c_str());
        return loft;
    }
}

TopoDS_Face CCPACSControlSurfaceDevice::GetBasePlane()
{
    // Building a Face defining the simple Flap Geometrie.
    tigl::CCPACSControlSurfaceDeviceOuterShapeBorder outerBorder = getOuterShape().getOuterBorder();
    gp_Pnt point1 = _segment->GetPoint(outerBorder.getEtaLE(),outerBorder.getXsiLE());
    gp_Pnt point2 = _segment->GetPoint(outerBorder.getEtaTE(),outerBorder.getXsiTE());

    tigl::CCPACSControlSurfaceDeviceOuterShapeBorder innerBorder = getOuterShape().getInnerBorder();
    gp_Pnt point3 = _segment->GetPoint(innerBorder.getEtaLE(),innerBorder.getXsiLE());
    gp_Pnt point4 = _segment->GetPoint(innerBorder.getEtaTE(), innerBorder.getXsiTE());

    gp_Vec p1, p2, p3, p4;
    getProjectedPoints(point1,point2,point3,point4,p1,p2,p3,p4);

    gp_Vec dirP1P2(p2 - p1);
    gp_Vec dirP3P4(p4 - p3);
    dirP1P2.Normalize();
    dirP3P4.Normalize();

    double fac = 2.0;
    if ( _type == LEADING_EDGE_DEVICE ) {
        p1 = p1 - dirP1P2.Multiplied(fac);
        p3 = p3 - dirP3P4.Multiplied(fac);
    }
    else if ( _type == TRAILING_EDGE_DEVICE) {
        p2 = p2 + dirP1P2.Multiplied(fac);
        p4 = p4 + dirP3P4.Multiplied(fac);
    }

    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(gp_Pnt(p1.XYZ()),gp_Pnt(p2.XYZ()));
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(gp_Pnt(p2.XYZ()),gp_Pnt(p4.XYZ()));
    TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(gp_Pnt(p3.XYZ()),gp_Pnt(p4.XYZ()));
    TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(gp_Pnt(p3.XYZ()),gp_Pnt(p1.XYZ()));

    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge4,edge3,edge2,edge1);
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

    return face;
}

void CCPACSControlSurfaceDevice::getProjectedPoints(gp_Pnt point1, gp_Pnt point2, gp_Pnt point3, gp_Pnt point4,
                                                    gp_Vec& projectedPoint1, gp_Vec& projectedPoint2, gp_Vec&
                                                    projectedPoint3, gp_Vec& projectedPoint4 )
{
    // This Method Projects 4 Points on a given Surface.
    gp_Vec sv;
    gp_Dir nv;
    gp_Vec nvV = getNormalOfControlSurfaceDevice();

    if (_type == SPOILER) {
        /*
         * This is a Placeholder, to implement Spoilers Properly this has to be changed. Probably
         * a different Code is needed to create Spoilers Correctly.
         */
        nvV.Multiply(determineSpoilerThickness());
        sv = gp_Vec(point1.XYZ()) - (nvV);
    }
    else {
        nvV.Multiply(determineCutOutPrismThickness());
        sv = gp_Vec(point1.XYZ()) - (nvV);
    }
    nv.SetXYZ(nvV.XYZ());

    Handle(Geom_Plane) plane = new Geom_Plane(gp_Pnt(sv.XYZ()),nv);

    gp_Pnt p1;
    gp_Pnt p2;
    gp_Pnt p3;
    gp_Pnt p4;

    //TODO: this is wrong
    // First, the for loops dont make any sense
    // Second, We need a directional projection - the direction must be the direction if the prism sweep
    GeomAPI_ProjectPointOnSurf projection1(point1,plane);
    for ( int i = 1; i <= projection1.NbPoints(); i++ ) {
        p1 = projection1.Point(1);
    }
    GeomAPI_ProjectPointOnSurf projection2(point2,plane);
    for ( int i = 1; i <= projection2.NbPoints(); i++ ) {
        p2 = projection2.Point(1);
    }
    GeomAPI_ProjectPointOnSurf projection3(point3,plane);
    for ( int i = 1; i <= projection3.NbPoints(); i++ ) {
        p3 = projection3.Point(1);
    }
    GeomAPI_ProjectPointOnSurf projection4(point4,plane);
    for ( int i = 1; i <= projection4.NbPoints(); i++ ) {
        p4 = projection4.Point(1);
    }

    projectedPoint1 = gp_Vec(p1.XYZ());
    projectedPoint2 = gp_Vec(p2.XYZ());
    projectedPoint3 = gp_Vec(p3.XYZ());
    projectedPoint4 = gp_Vec(p4.XYZ());
}

double CCPACSControlSurfaceDevice::determineCutOutPrismThickness()
{
    TopoDS_Shape wcsShape = _segment->GetLoft()->Shape();
    Bnd_Box B;
    double Xmin,Xmax,Ymin,Ymax,Zmin,Zmax;
    BRepBndLib::Add(wcsShape,B);
    B.Get(Xmin,Ymin,Zmin,Xmax,Ymax,Zmax);

    double minThickness = Xmax-Xmin;

    /*
     * This may lead to bugs in some really special cases, like when
     * a WCS is thicker than wide or long -> return maxThickness instead.
     */

    if ( Ymax-Ymin < minThickness ) {
        minThickness = Ymax-Ymin;
    }
    if ( Zmax-Zmin < minThickness ) {
        minThickness = Zmax-Zmin;
    }
    return minThickness;
}

gp_Vec CCPACSControlSurfaceDevice::getNormalOfControlSurfaceDevice()
{
    gp_Pnt point1 = _segment->GetPoint(0,0);
    gp_Pnt point2 = _segment->GetPoint(0,1);
    gp_Pnt point3 = _segment->GetPoint(1,0);

    gp_Vec dir1to2 = -(gp_Vec(point1.XYZ()) - gp_Vec(point2.XYZ()));
    gp_Vec dir1to3 = -(gp_Vec(point1.XYZ()) - gp_Vec(point3.XYZ()));

    gp_Vec nvV = dir1to2^dir1to3;
    nvV.Normalize();
    return nvV;
}

CCPACSWingComponentSegment* CCPACSControlSurfaceDevice::getSegment()
{
    return _segment;
}

// get short name for loft
std::string CCPACSControlSurfaceDevice::GetShortShapeName()
{
    std::string tmp = _segment->GetLoft()->ShortName();
    for (int j = 1; j <= _segment->getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount(); j++) {
        tigl::CCPACSControlSurfaceDevice& csd = _segment->getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceByID(j);
        if (GetUID() == csd.GetUID()) {
            std::stringstream shortName;
            if (_type == LEADING_EDGE_DEVICE) {
                shortName << tmp << "LED" << j;
            }
            else if (_type == TRAILING_EDGE_DEVICE) {
                shortName << tmp << "TED" << j;
            }
            else if (_type == SPOILER) {
                shortName << tmp << "SPO" << j;
            }
            return shortName.str();
        }
    }
    return "UNKNOWN";
}

TiglControlSurfaceType CCPACSControlSurfaceDevice::getType()
{
    return _type;
}

double CCPACSControlSurfaceDevice::determineSpoilerThickness()
{
    /*Placeholder*/
    return 0.01f;
}


gp_Pnt CCPACSControlSurfaceDevice::getLeadingEdgeShapeLeadingEdgePoint(bool isInnerBorder)
{
    // This Method returns the LeadingEdgePoint of the LeadingEdgeShape.
    gp_Vec controlSurfaceNormal = getNormalOfControlSurfaceDevice();
    double leadingPointEta,leadingPointXsi,relHeight;
    if (isInnerBorder) {
        leadingPointEta = getOuterShape().getInnerBorder().getEtaLE();
        leadingPointXsi = getOuterShape().getInnerBorder().getXsiLE();
        relHeight = getOuterShape().getInnerBorder().getLeadingEdgeShape().getRelHeightLE();
    }
    else {
        leadingPointEta = getOuterShape().getOuterBorder().getEtaLE();
        leadingPointXsi = getOuterShape().getOuterBorder().getXsiLE();
        relHeight = getOuterShape().getOuterBorder().getLeadingEdgeShape().getRelHeightLE();
    }

    gp_Pnt helperUpper = _segment->GetPointDirection(leadingPointEta,
                                                     leadingPointXsi,
                                                     controlSurfaceNormal.X(),
                                                     controlSurfaceNormal.Y(),
                                                     controlSurfaceNormal.Z(),true);

    gp_Pnt helperLower = _segment->GetPointDirection(leadingPointEta,
                                                     leadingPointXsi,
                                                     controlSurfaceNormal.X(),
                                                     controlSurfaceNormal.Y(),
                                                     controlSurfaceNormal.Z(),false);

    gp_Vec upperToLower = (gp_Vec(helperLower.XYZ()) - gp_Vec(helperUpper.XYZ())).Multiplied(relHeight);
    return gp_Pnt(( gp_Vec(helperUpper.XYZ()) + gp_Vec(upperToLower.XYZ() )).XYZ());
}

gp_Pln CCPACSControlSurfaceDevice::getBorderPlane(bool isInnerBorder)
{
    double lEta, lXsi, tEta, tXsi;
    if (isInnerBorder) {
        lEta = getOuterShape().getInnerBorder().getEtaLE();
        lXsi = getOuterShape().getInnerBorder().getXsiLE();
        tEta = getOuterShape().getInnerBorder().getEtaLE();
        tXsi = getOuterShape().getInnerBorder().getXsiTE();
    }
    else {
        lEta = getOuterShape().getOuterBorder().getEtaLE();
        lXsi = getOuterShape().getOuterBorder().getXsiLE();
        tEta = getOuterShape().getOuterBorder().getEtaLE();
        tXsi = getOuterShape().getOuterBorder().getXsiTE();
    }

    gp_Pnt pLE = _segment->GetPoint(lEta, lXsi);
    gp_Pnt pTE = _segment->GetPoint(tEta, tXsi);

    gp_Dir pX = pLE.XYZ() - pTE.XYZ();
    gp_Dir planeNormal = getNormalOfControlSurfaceDevice().Crossed(gp_Vec(1,0,0));

    gp_Pln plane(gp_Ax3(pLE, planeNormal, pX));

    return plane;
}

// Compute the point and tangent of the leading edge of the control surface
// It is tangential to the wing surface.
void CCPACSControlSurfaceDevice::getLeadingEdgeTangent(gp_Pln borderPlane, bool isInnerBorder, bool isUpper, gp_Pnt& point, gp_Vec& result)
{
    // Returns the Tangent of the Upper or the lower Point of the LeadingEdgeShape.
    gp_Vec controlSurfaceNormal = getNormalOfControlSurfaceDevice();

    double eta;
    double xsi;

    tigl::CCPACSControlSurfaceDeviceOuterShapeBorder border;
    if (!isInnerBorder) {
        eta = getOuterShape().getOuterBorder().getEtaLE();
        border = getOuterShape().getOuterBorder();
    }
    else {
        eta = getOuterShape().getInnerBorder().getEtaLE();
        border = getOuterShape().getInnerBorder();
    }

    if (!isUpper) {
        controlSurfaceNormal.Multiply(-1);
        xsi = border.getLeadingEdgeShape().getXsiLowerSkin();
    }
    else {
        xsi = border.getLeadingEdgeShape().getXsiUpperSkin();
    }

    gp_Vec normal; gp_Pnt pnt;
    _segment->GetPointDirectionNormal(eta, xsi, controlSurfaceNormal.X(),
                                       controlSurfaceNormal.Y(),controlSurfaceNormal.Z(),
                                       isUpper, pnt, normal);

    // the tangent is the cross product of the two normal vectors
    result = normal.Crossed(borderPlane.Axis().Direction());
    result.Normalize();

    gp_Vec leading(_segment->GetPoint(eta,0).XYZ());
    gp_Vec trailing(_segment->GetPoint(eta,1).XYZ());
    gp_Vec trailingToLeading = leading - trailing;

    if (trailingToLeading * result < 0) {
        result.Multiply(-1);
    }

    point = pnt;
}

// Computes the wire for the cutout loft. This is done as follows
// 1. Compute the cutout plane
// 2. Compute all relevant points, i.e. upper wing point, leading edge point, lower wing point
// 3. Project all points onto the cutout plane
// 4. Add some points to close the wire
// 5. Compute the leading edge by interpolation
TopoDS_Wire CCPACSControlSurfaceDevice::getCutoutWire(bool isInnerBorder)
{
    // Compute cutout plane
    gp_Pln plane = getBorderPlane(isInnerBorder);
    Handle(Geom_Surface) surf = new Geom_Plane(plane);

    gp_Vec csNormal = getNormalOfControlSurfaceDevice();
    csNormal.Normalize();

    gp_Pnt trailingEdgePoint;
    if (isInnerBorder) {
        trailingEdgePoint = _segment->GetPoint(
                   getOuterShape().getInnerBorder().getEtaTE(),
                   getOuterShape().getInnerBorder().getXsiTE());
    }
    else {
        trailingEdgePoint = _segment->GetPoint(
                   getOuterShape().getOuterBorder().getEtaTE(),
                   getOuterShape().getOuterBorder().getXsiTE());
    }

    gp_Vec tangentUpper, tangentLower;
    gp_Pnt lowerPoint, upperPoint, leadingPoint;

    // compute relevant points
    leadingPoint = getLeadingEdgeShapeLeadingEdgePoint(isInnerBorder);
    getLeadingEdgeTangent(plane, isInnerBorder, true, upperPoint, tangentUpper);
    getLeadingEdgeTangent(plane, isInnerBorder,false, lowerPoint, tangentLower);
    tangentUpper.Multiply(-1);

    // project points and vectors onto cutout plane
    gp_Pnt2d lp2d = ProjectPointOnPlane(plane, lowerPoint);
    gp_Pnt2d up2d = ProjectPointOnPlane(plane, upperPoint);
    gp_Pnt2d le2d = ProjectPointOnPlane(plane, leadingPoint);
    gp_Pnt2d te2d = ProjectPointOnPlane(plane, trailingEdgePoint);

    gp_Vec2d upTan2d = ProjectVecOnPlane(plane, tangentUpper);
    gp_Vec2d loTan2d = ProjectVecOnPlane(plane, tangentLower);
    gp_Vec2d upNorm2d(-upTan2d.Y(), upTan2d.X());
    gp_Vec2d loNorm2d(-loTan2d.Y(), loTan2d.X());

    // compute some extra points with enough offset, to close the wire outside the wing
    double offset_factor = 5.;
    double ymax = std::max(fabs(lp2d.Y()), fabs(up2d.Y())) * offset_factor;
    double alphaUp = (ymax - up2d.Y())/upNorm2d.Y();
    double alphaLo = (-ymax - lp2d.Y())/loNorm2d.Y();

    gp_Pnt2d upFront2d = up2d.XY() + alphaUp*upNorm2d.XY();
    gp_Pnt2d loFront2d = lp2d.XY() + alphaLo*loNorm2d.XY();

    double xmax = le2d.X() + offset_factor*(te2d.X() - le2d.X());

    gp_Pnt2d upBack2d(xmax, ymax);
    gp_Pnt2d loBack2d(xmax, -ymax);

    // Compute the leading edge by interpolating upper, lower and leading edge point
    Handle(TColgp_HArray1OfPnt2d) points = new TColgp_HArray1OfPnt2d(1,3);
    points->SetValue(1, lp2d);
    points->SetValue(2, le2d);
    points->SetValue(3, up2d);
    
    Geom2dAPI_Interpolate interp(points, false, Precision::Confusion());

    // only create c1 continous cutouts, if leading edge is in front of upper and lower points
    if (le2d.X() > lp2d.X() && le2d.X() > up2d.X()) {
        interp.Load(loTan2d, upTan2d);
    }
    interp.Perform();

    TopoDS_Edge leadEdge = BRepBuilderAPI_MakeEdge(interp.Curve(), surf);
    BRepLib::BuildCurves3d(leadEdge);

    // create the wire
    BRepBuilderAPI_MakeWire wiremaker;
    wiremaker.Add(leadEdge);
    wiremaker.Add(pointsToEdge( surf, up2d, upFront2d));
    wiremaker.Add(pointsToEdge( surf, upFront2d, upBack2d));
    wiremaker.Add(pointsToEdge( surf, upBack2d, loBack2d));
    wiremaker.Add(pointsToEdge( surf, loBack2d, loFront2d));
    wiremaker.Add(pointsToEdge( surf, loFront2d, lp2d));

#ifdef DEBUG
    std::stringstream filenamestr;
    if (isInnerBorder) {
        filenamestr << GetUID() << "_cutout_innerwire.brep";
    }
    else {
        filenamestr << GetUID() << "_cutout_outerwire.brep";
    }
    BRepTools::Write(wiremaker.Wire(), filenamestr.str().c_str());
#endif

    return wiremaker.Wire();
}

double CCPACSControlSurfaceDevice::GetMinDeflection() const
{
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    CCPACSControlSurfaceDeviceStep step = steps.GetStep(1);
    
    return step.getRelDeflection();
}

double CCPACSControlSurfaceDevice::GetMaxDeflection() const
{
    CCPACSControlSurfaceDeviceSteps steps = getMovementPath().getSteps();
    CCPACSControlSurfaceDeviceStep step = steps.GetStep(steps.GetStepCount());
    
    return step.getRelDeflection();
}

} // end namespace tigl
