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

#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglControlSurfaceHingeLine.h"
#include "CCPACSControlSurfaceDeviceWingCutOut.h"
#include "CCPACSControlSurfaceDeviceBorderLeadingEdgeShape.h"
#include "tiglcommonfunctions.h"

#include "Handle_Geom_Plane.hxx"
#include "Geom_Plane.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "Handle_Geom_TrimmedCurve.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakePolygon.hxx"
#include "GeomAPI_ProjectPointOnSurf.hxx"
#include "GC_MakeSegment.hxx"
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "gp_Pln.hxx"
#include "Handle_Geom_BSplineCurve.hxx"
#include "Geom_BSplineCurve.hxx"
#include "GeomAPI_Interpolate.hxx"
#include "Handle_TColgp_HArray1OfPnt.hxx"
#include "TColgp_HArray1OfPnt.hxx"
#include "GeomAPI_IntSS.hxx"
#include "Handle_Geom_Line.hxx"
#include "Geom_Line.hxx"
#include "BRepOffsetAPI_ThruSections.hxx"
#include "gp_Lin.hxx"
#include "BRepBuilderAPI_Sewing.hxx"
#include "BRepBuilderAPI_MakeSolid.hxx"


#define USE_ADVANCED_MODELING 1


namespace tigl
{

CCPACSControlSurfaceDevice::CCPACSControlSurfaceDevice(CCPACSWingComponentSegment* segment)
    : _segment(segment)
{
    SetUID("ControlDevice");
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

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    loft  = PNamedShape(new CNamedShape(TopoDS_Shape(), loftName.c_str(), loftShortName.c_str()));

    _type = type;
    _hingeLine = new CTiglControlSurfaceHingeLine(&getOuterShape(),&getMovementPath(),_segment);
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

gp_Trsf CCPACSControlSurfaceDevice::getTransformation(double flapStatusInPercent) const
{
    /*
     * this block of code calculates all needed values to rotate and move the controlSurfaceDevice according
     * to the given relDeflection by using a linearInterpolation.
     */
    std::vector<double> relDeflections = this->getMovementPath().getRelDeflections();
    double inputDeflection = ( relDeflections[relDeflections.size()-1] - relDeflections[0] ) * ( flapStatusInPercent/100 ) + relDeflections[0];
    double rotation = Interpolate( relDeflections, this->getMovementPath().getHingeLineRotations(), inputDeflection );
    double innerTranslationX = Interpolate( relDeflections, this->getMovementPath().getInnerHingeTranslationsX(), inputDeflection );
    double innerTranslationY = Interpolate( relDeflections, this->getMovementPath().getInnerHingeTranslationsY(), inputDeflection );
    double innerTranslationZ = Interpolate( relDeflections, this->getMovementPath().getInnerHingeTranslationsZ(), inputDeflection );
    double outerTranslationX = Interpolate( relDeflections, this->getMovementPath().getOuterHingeTranslationsX(), inputDeflection );
    double outerTranslationZ = Interpolate( relDeflections, this->getMovementPath().getOuterHingeTranslationsZ(), inputDeflection );
    relDeflections.clear();

    gp_Pnt innerHingeOld = _hingeLine->getInnerHingePoint();;
    gp_Pnt outerHingeOld = _hingeLine->getOuterHingePoint();;

    /*
     * innerTranslationY on hingePoint1 on purpose, maybe consider setting it to zero as default. See CPACS definition on
     * Path/Step/HingeLineTransformation for more informations.
     */
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
        TopoDS_Wire innerWire = buildLeadingEdgeShapeWire(true);
        TopoDS_Wire outerWire = buildLeadingEdgeShapeWire(false);

        // make one shape out of the 2 wires and build connections inbetween.
        BRepOffsetAPI_ThruSections thrusections(false,true);
        thrusections.AddWire(outerWire);
        thrusections.AddWire(innerWire);
        thrusections.Build();

        gp_Pnt lowerPoint = getLeadingEdgeShapeLowerPoint(false);
        gp_Pnt upperPoint = getLeadingEdgeShapeUpperPoint(false);
        gp_Pnt leadingPoint = getLeadingEdgeShapeLeadingEdgePoint(false);

        gp_Vec leadingToUpper = gp_Vec(upperPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());
        gp_Vec leadingToLower = gp_Vec(lowerPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());

        // generate the Top of the newly formed shape.
        TopoDS_Face outerFace = BRepBuilderAPI_MakeFace(gp_Pln(leadingPoint, leadingToLower^leadingToUpper),outerWire);
        outerFace.Reverse();

        lowerPoint = getLeadingEdgeShapeLowerPoint(true);
        upperPoint = getLeadingEdgeShapeUpperPoint(true);
        leadingPoint = getLeadingEdgeShapeLeadingEdgePoint(true);

        leadingToUpper = gp_Vec(upperPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());
        leadingToLower = gp_Vec(lowerPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());

        // generate the Bottom of the newly formed shape.
        TopoDS_Face innerFace = BRepBuilderAPI_MakeFace(gp_Pln(leadingPoint, leadingToLower^leadingToUpper),innerWire);
        innerFace.Reverse(); // Maybe reverse only one of the two Faces ( outer and innerFace ).

        // Fuse all shapes together.
        BRepBuilderAPI_Sewing sewer;
        sewer.Add(outerFace);
        sewer.Add(thrusections.Shape());
        sewer.Add(innerFace);
        sewer.Perform();

        TopoDS_Shape sh = BRepBuilderAPI_MakeSolid(TopoDS::Shell(sewer.SewedShape()));
        loft = PNamedShape(new CNamedShape(sh, GetUID().c_str()));
        return loft;
    }
    else {
#else
    {
#endif
        // Build Simple Flap Geometry.
        TopoDS_Face face = getFace();
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
        return loft;
    }
}

TopoDS_Face CCPACSControlSurfaceDevice::getFace()
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

gp_Pnt CCPACSControlSurfaceDevice::getLeadingEdgeShapeLowerPoint(bool isInnerBorder)
{
    gp_Vec controlSurfaceNormal = getNormalOfControlSurfaceDevice();
    if (isInnerBorder) {
        return _segment->GetPointDirection(getOuterShape().getInnerBorder().getEtaLE(),
                                           getOuterShape().getInnerBorder().getLeadingEdgeShape().getXsiLowerSkin(),
                                           controlSurfaceNormal.X(),controlSurfaceNormal.Y(),controlSurfaceNormal.Z(),false);
    }
    else {
        return _segment->GetPointDirection(getOuterShape().getOuterBorder().getEtaLE(),
                                           getOuterShape().getOuterBorder().getLeadingEdgeShape().getXsiLowerSkin(),
                                           controlSurfaceNormal.X(),controlSurfaceNormal.Y(),controlSurfaceNormal.Z(),false);
    }
}

gp_Pnt CCPACSControlSurfaceDevice::getLeadingEdgeShapeUpperPoint(bool isInnerBorder)
{
    gp_Vec controlSurfaceNormal = getNormalOfControlSurfaceDevice();
    if (isInnerBorder) {
        return _segment->GetPointDirection(getOuterShape().getInnerBorder().getEtaLE(),
                                           getOuterShape().getInnerBorder().getLeadingEdgeShape().getXsiUpperSkin(),
                                           controlSurfaceNormal.X(),controlSurfaceNormal.Y(),controlSurfaceNormal.Z(),true);
    }
    else {
        return _segment->GetPointDirection(getOuterShape().getOuterBorder().getEtaLE(),
                                           getOuterShape().getOuterBorder().getLeadingEdgeShape().getXsiUpperSkin(),
                                           controlSurfaceNormal.X(),controlSurfaceNormal.Y(),controlSurfaceNormal.Z(),true);
    }
}

gp_Vec CCPACSControlSurfaceDevice::getLeadingEdgeShapeTangent(gp_Pnt leadingPoint, gp_Pnt lowerPoint, gp_Pnt upperPoint, bool isInnerBorder, bool isUpper)
{
    // Returns the Tangent of the Upper or the lower Point of the LeadingEdgeShape.
    gp_Vec result;
    gp_Vec controlSurfaceNormal = getNormalOfControlSurfaceDevice();
    gp_Vec leadingToUpper = gp_Vec(upperPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());
    gp_Vec leadingToLower = gp_Vec(lowerPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());

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
        xsi = getOuterShape().getOuterBorder().getLeadingEdgeShape().getXsiLowerSkin();
    }
    else {
        xsi = getOuterShape().getOuterBorder().getLeadingEdgeShape().getXsiUpperSkin();
    }

    result = _segment->GetPointTangent(eta, xsi, controlSurfaceNormal.X(),
                                       controlSurfaceNormal.Y(),controlSurfaceNormal.Z(),
                                       gp_Pln(leadingPoint, leadingToLower^leadingToUpper),isUpper);

    gp_Vec leading(_segment->GetPoint(eta,0).XYZ());
    gp_Vec trailing(_segment->GetPoint(eta,1).XYZ());
    gp_Vec trailingToLeading = leading - trailing;

    if (trailingToLeading * result < 0) {
        result.Multiply(-1);
    }
    return result;
}

gp_Vec CCPACSControlSurfaceDevice::getLeadingEdgeShapeNormal(gp_Pnt point, gp_Vec tangent,gp_Pln etaPlane,gp_Vec checker)
{
    // This Method Gets The LeadingEdgeShapeNormal, build out of the LeadingEdgeShapeTangent.
    gp_Pln tangentPlane = gp_Pln(point,tangent);

    Handle_Geom_Plane geomEtaPlane = new Geom_Plane(etaPlane);
    Handle_Geom_Plane geomTangentPlane = new Geom_Plane(tangentPlane);

    // Get intersection curve
    GeomAPI_IntSS     intersection;
    intersection.Perform(geomEtaPlane, geomTangentPlane, 1.0e-7);
    Handle(Geom_Curve) curve = intersection.Line(1);

    gp_Vec value((Handle(Geom_Line)::DownCast(curve))->Lin().Direction().XYZ());

    if (checker * value < 0) {
        value.Multiply(-1);
    }
    return value;
}

TopoDS_Wire CCPACSControlSurfaceDevice::buildLeadingEdgeShapeWire(bool isInnerBorder)
{
    // This Methods builds a Wire defining the LeadingEdgeShape when combined
    // With the wing Geometry.
    gp_Pnt lowerPoint = getLeadingEdgeShapeLowerPoint(isInnerBorder);
    gp_Pnt upperPoint = getLeadingEdgeShapeUpperPoint(isInnerBorder);
    gp_Pnt leadingPoint = getLeadingEdgeShapeLeadingEdgePoint(isInnerBorder);
    gp_Vec lowerToUpper = gp_Vec(upperPoint.XYZ()) - gp_Vec(lowerPoint.XYZ());

    gp_Vec leadingToUpper = gp_Vec(upperPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());
    gp_Vec leadingToLower = gp_Vec(lowerPoint.XYZ()) - gp_Vec(leadingPoint.XYZ());

    upperPoint = gp_Pnt((gp_Vec(upperPoint.XYZ()) + leadingToUpper.Normalized().Multiplied(0.1)).XYZ());
    lowerPoint = gp_Pnt((gp_Vec(lowerPoint.XYZ()) + leadingToLower.Normalized().Multiplied(0.1)).XYZ());

    gp_Vec tangentUpper = getLeadingEdgeShapeTangent(leadingPoint,lowerPoint,upperPoint,isInnerBorder,true);
    gp_Vec tangentLower = getLeadingEdgeShapeTangent(leadingPoint,lowerPoint,upperPoint,isInnerBorder,false);
    tangentLower.Multiply(-1);

    gp_Pnt leadingEdge = _segment->GetPoint(getOuterShape().getInnerBorder().getEtaLE(),0);
    gp_Pnt trailingEdge = _segment->GetPoint(getOuterShape().getInnerBorder().getEtaLE(),1);

    gp_Vec etaUpperNormalDirection = getLeadingEdgeShapeNormal(upperPoint,tangentUpper,gp_Pln(leadingPoint, leadingToLower^leadingToUpper)
                                                               ,lowerToUpper);
    gp_Vec etaLowerNormalDirection = getLeadingEdgeShapeNormal(lowerPoint,tangentLower,gp_Pln(leadingPoint, leadingToLower^leadingToUpper)
                                                               ,lowerToUpper.Multiplied(-1));


    tangentUpper.Normalize();
    tangentUpper.Multiply(leadingToUpper.Magnitude()/2);
    tangentLower.Normalize();
    tangentLower.Multiply(leadingToLower.Magnitude()/2);

    Handle_TColgp_HArray1OfPnt array = new TColgp_HArray1OfPnt(1,3);
    array->SetValue(1,lowerPoint);
    array->SetValue(2,leadingPoint);
    array->SetValue(3,upperPoint);

    GeomAPI_Interpolate Interp(array,false,0.00001);
    Interp.Load(tangentUpper,tangentLower,false);
    Interp.Perform();
    Handle(Geom_BSplineCurve) C = Interp.Curve();

    TopoDS_Edge edgeLeading = BRepBuilderAPI_MakeEdge(C);

    double distance = (gp_Vec(leadingEdge.XYZ()) - gp_Vec(trailingEdge.XYZ())).Magnitude();

    gp_Vec upperOuterPoint = gp_Vec(upperPoint.XYZ()) + etaUpperNormalDirection.Multiplied(distance);
    TopoDS_Edge edgeUpperOuter = BRepBuilderAPI_MakeEdge(upperPoint,gp_Pnt(upperOuterPoint.XYZ()));

    gp_Vec lowerOuterPoint = gp_Vec(lowerPoint.XYZ()) + etaLowerNormalDirection.Multiplied(distance);
    TopoDS_Edge edgeLowerOuter = BRepBuilderAPI_MakeEdge(gp_Pnt(lowerOuterPoint.XYZ()),lowerPoint);


    gp_Vec upperOuterClosingPoint = upperOuterPoint + (gp_Vec(trailingEdge.XYZ()) - gp_Vec(leadingEdge.XYZ()));
    gp_Vec lowerOuterClosingPoint = lowerOuterPoint + (gp_Vec(trailingEdge.XYZ()) - gp_Vec(leadingEdge.XYZ()));

    TopoDS_Edge edgeUpperClosing = BRepBuilderAPI_MakeEdge(gp_Pnt(upperOuterPoint.XYZ()),gp_Pnt(upperOuterClosingPoint.XYZ()));
    TopoDS_Edge edgeLowerClosing = BRepBuilderAPI_MakeEdge(gp_Pnt(lowerOuterClosingPoint.XYZ()),gp_Pnt(lowerOuterPoint.XYZ()));
    TopoDS_Edge edgeClosing = BRepBuilderAPI_MakeEdge(gp_Pnt(upperOuterClosingPoint.XYZ()),gp_Pnt(lowerOuterClosingPoint.XYZ()));

    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(edgeLeading);
    wireMaker.Add(edgeUpperOuter);
    wireMaker.Add(edgeUpperClosing);
    wireMaker.Add(edgeClosing);
    wireMaker.Add(edgeLowerClosing);
    wireMaker.Add(edgeLowerOuter);
    wireMaker.Build();
    return wireMaker.Wire();
}

} // end namespace tigl
