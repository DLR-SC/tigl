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
/**
 * @file
 * @brief  Implementation of CPACS ...  handling routines.
 */

#include <iostream>
#include <sstream>
#include <exception>
#include <cassert>

#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglControlSurfaceHingeLine.h"


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

#ifdef TIGL_USE_XCAF
#include "XCAFDoc_ShapeTool.hxx"
#include "XCAFApp_Application.hxx"
#include "XCAFDoc_DocumentTool.hxx"
#include "TDataStd_Name.hxx"
#include "TDataXtd_Shape.hxx"
#endif

namespace tigl {

CCPACSControlSurfaceDevice::CCPACSControlSurfaceDevice(CCPACSWingComponentSegment* segment)
    : _segment(segment)
{
    uID = "not loaded";
}

// Read CPACS trailingEdgeDevice elements
void CCPACSControlSurfaceDevice::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& trailingEdgeDeviceXPath, TiglControlSurfaceType type)
{
    char*       elementPath;
    std::string tempString;

    // Get sublement "outerShape"
    char* ptrName = NULL;
    tempString    = trailingEdgeDeviceXPath + "/outerShape";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS)
    {
        outerShape.ReadCPACS(tixiHandle, elementPath, type);
    }

    tempString = trailingEdgeDeviceXPath + "/path";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        path.ReadCPACS(tixiHandle, elementPath);
    }

    char* atrName = NULL;
    if ( tixiGetAttributeName(tixiHandle,trailingEdgeDeviceXPath.c_str(),1,&atrName) == SUCCESS ) {
        if (tixiCheckAttribute(tixiHandle, trailingEdgeDeviceXPath.c_str(), atrName) == SUCCESS ) {
            char* uIDtmp;
            tixiGetTextAttribute(tixiHandle,trailingEdgeDeviceXPath.c_str(),atrName, &uIDtmp);
            uID = uIDtmp;
        }
    }

    std::string loftName = GetUID();
    std::string loftShortName = GetShortShapeName();
    loft  = PNamedShape(new CNamedShape(TopoDS_Shape(), loftName.c_str(), loftShortName.c_str()));

    _type = type;
    _hingeLine = new CTiglControlSurfaceHingeLine(getOuterShape(),getMovementPath(),_segment);
}

void CCPACSControlSurfaceDevice::setLoft(TopoDS_Shape loft)
{
    this->loft->SetShape(loft);
}

PNamedShape CCPACSControlSurfaceDevice::BuildLoft()
{
    return loft;
}

CCPACSControlSurfaceOuterShape CCPACSControlSurfaceDevice::getOuterShape()
{
    return outerShape;
}

CCPACSControlSurfaceDevicePath CCPACSControlSurfaceDevice::getMovementPath()
{
    return path;
}

std::string CCPACSControlSurfaceDevice::getUID()
{
    return uID;
}

gp_Trsf CCPACSControlSurfaceDevice::getTransformation(double flapStatusInPercent)
{
    /*
     * this block of code calculates all needed values to rotate and move the controlSurfaceDevice according
     * to the given relDeflection by using a linearInterpolation.
     */
    std::vector<double> relDeflections = this->getMovementPath().getRelDeflections();
    double inputDeflection = ( relDeflections[relDeflections.size()-1] - relDeflections[0] ) * ( flapStatusInPercent/100 ) + relDeflections[0];
    double rotation = linearInterpolation( relDeflections, this->getMovementPath().getHingeLineRotations(), inputDeflection );
    double innerTranslationX = linearInterpolation( relDeflections, this->getMovementPath().getInnerHingeTranslationsX(), inputDeflection );
    double innerTranslationY = linearInterpolation( relDeflections, this->getMovementPath().getInnerHingeTranslationsY(), inputDeflection );
    double innerTranslationZ = linearInterpolation( relDeflections, this->getMovementPath().getInnerHingeTranslationsZ(), inputDeflection );
    double outerTranslationX = linearInterpolation( relDeflections, this->getMovementPath().getOuterHingeTranslationsX(), inputDeflection );
    double outerTranslationZ = linearInterpolation( relDeflections, this->getMovementPath().getOuterHingeTranslationsZ(), inputDeflection );
    relDeflections.clear();

    gp_Pnt innerHingeOld = _hingeLine->getInnerHingePoint();;
    gp_Pnt outerHingeOld = _hingeLine->getOuterHingePoint();;

    // may use innerTranslationY instead of 0.
    gp_Pnt hingePoint1 = _hingeLine->getTransformedOuterHingePoint(gp_Vec(outerTranslationX, innerTranslationY, outerTranslationZ));
    gp_Pnt hingePoint2 = _hingeLine->getTransformedInnerHingePoint(gp_Vec(innerTranslationX, innerTranslationY, innerTranslationZ));

    // calculating the needed transformations
    CTiglControlSurfaceTransformation transformation(innerHingeOld, outerHingeOld, hingePoint2, hingePoint1, rotation);

    return transformation.getTotalTransformation();
}

TopoDS_Shape CCPACSControlSurfaceDevice::getCutOutShape()
{
    TopoDS_Face face = getFace();
    gp_Vec vec = getNormalOfControlSurfaceDevice();

    if (_type == SPOILER) {
        vec.Multiply(0.8f);
    } else {
        vec.Multiply(determineCutOutPrismThickness()*2);
    }

    TopoDS_Shape prism = BRepPrimAPI_MakePrism(face,vec);

    loft->SetShape(prism);
    return loft->Shape();
}

TopoDS_Face CCPACSControlSurfaceDevice::getFace()
{

    tigl::CCPACSControlSurfaceBorder outerBorder = getOuterShape().getOuterBorder();
    gp_Pnt point1 = _segment->GetPoint(outerBorder.getEtaLE(),outerBorder.getXsiLE());
    gp_Pnt point2 = _segment->GetPoint(outerBorder.getEtaTE(),outerBorder.getXsiTE());

    tigl::CCPACSControlSurfaceBorder innerBorder = getOuterShape().getInnerBorder();
    gp_Pnt point3 = _segment->GetPoint(innerBorder.getEtaLE(),innerBorder.getXsiLE());
    gp_Pnt point4 = _segment->GetPoint(innerBorder.getEtaTE(), innerBorder.getXsiTE());

    gp_Vec p1, p2, p3, p4;
    getProjectedPoints(point1,point2,point3,point4,p1,p2,p3,p4);

    gp_Vec dirP1P2(p2 - p1);
    gp_Vec dirP3P4(p4 - p3);
    dirP1P2.Normalize();
    dirP3P4.Normalize();

    double fac = 0.1;
    if ( _type == LEADING_EDGE_DEVICE ) {
        p1 = p1 - dirP1P2.Multiplied(fac);
        p3 = p3 - dirP3P4.Multiplied(fac);
    }
    else if ( _type == TRAILING_EDGE_DEVICE) {
        p2 = p2 + dirP1P2.Multiplied(fac);
        p4 = p4 + dirP3P4.Multiplied(fac);
    }

    Handle(Geom_TrimmedCurve) segment1 = GC_MakeSegment(gp_Pnt(p1.XYZ()),gp_Pnt(p2.XYZ()));
    Handle(Geom_TrimmedCurve) segment2 = GC_MakeSegment(gp_Pnt(p2.XYZ()),gp_Pnt(p4.XYZ()));
    Handle(Geom_TrimmedCurve) segment3 = GC_MakeSegment(gp_Pnt(p3.XYZ()),gp_Pnt(p4.XYZ()));
    Handle(Geom_TrimmedCurve) segment4 = GC_MakeSegment(gp_Pnt(p3.XYZ()),gp_Pnt(p1.XYZ()));

    TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(segment1);
    TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(segment2);
    TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(segment3);
    TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(segment4);

    TopoDS_Wire wire = BRepBuilderAPI_MakeWire(edge4,edge3,edge2,edge1);
    TopoDS_Face face = BRepBuilderAPI_MakeFace(wire);

    return face;
}

double CCPACSControlSurfaceDevice::linearInterpolation(std::vector<double> list1, std::vector<double> list2, double valueRelList1)
{
    double min = 0;
    double max = 0;
    int idefRem = 0;
    for ( std::vector<double>::size_type idef = 1; idef < list1.size(); idef++ ) {
        if ( list1[idef-1] <= valueRelList1 && list1[idef] >= valueRelList1 ) {
            min = list1[idef-1];
            max = list1[idef];
            idefRem = idef;
            break;
        }
    }
    double value = ( valueRelList1 - list1[idefRem-1] ) / ( list1[idefRem] - list1[idefRem-1] );
    double min2 = list2[idefRem-1];
    double max2 = list2[idefRem];
    return value * ( max2 - min2 ) + min2;
}

void CCPACSControlSurfaceDevice::getProjectedPoints(gp_Pnt point1, gp_Pnt point2, gp_Pnt point3, gp_Pnt point4,
                                                  gp_Vec& projectedPoint1, gp_Vec& projectedPoint2, gp_Vec&
                                                  projectedPoint3, gp_Vec& projectedPoint4 )
{
    gp_Vec nvV = getNormalOfControlSurfaceDevice();

    gp_Vec sv;
    gp_Dir nv;

    if (_type == SPOILER) {
        nvV.Multiply(0.1f);
        sv = gp_Vec(point1.XYZ()) - (nvV);
    } else {
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
            } else if(_type == TRAILING_EDGE_DEVICE) {
                shortName << tmp << "TED" << j;
            } else if(_type == SPOILER) {
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
} // end namespace tigl
