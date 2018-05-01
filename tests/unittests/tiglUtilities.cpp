/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2012-11-21 Martin Siggel <Martin.Siggel@dlr.de>
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
* @brief  Miscellaneous tests
*/

#include "test.h"
#include "tigl.h"
#include "tiglcommonfunctions.h"
#include "CTiglLogging.h"
#include "CTiglInterpolateBsplineWire.h"
#include "typename.h"

#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "GCPnts_AbscissaPoint.hxx"
#include "TopoDS_Wire.hxx"
#include "gp_Circ.hxx"

TEST(Misc, WireGetPoint)
{
    gp_Pnt p1(0.,0.,0.);
    gp_Pnt p2(1.,0.,0.);
    gp_Pnt p3(1.,3.,0.);
    
    BRepBuilderAPI_MakeWire wireBuilder;
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(p1, p2));
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(p2, p3));
    
    ASSERT_NEAR(4.0, GetLength(wireBuilder.Wire()), 1e-7);
    gp_Pnt p1_ = WireGetPoint(wireBuilder.Wire(), 0.0);
    gp_Pnt p2_ = WireGetPoint(wireBuilder.Wire(), 0.25);
    gp_Pnt p3_ = WireGetPoint(wireBuilder.Wire(), 1.0);
    
    ASSERT_NEAR(0.0, p1_.Distance(p1), 1e-7);
    ASSERT_NEAR(0.0, p2_.Distance(p2), 1e-7);
    ASSERT_NEAR(0.0, p3_.Distance(p3), 1e-7);
    
    p1_ = WireGetPoint(wireBuilder.Wire(), 0.0);
    p2_ = WireGetPoint(wireBuilder.Wire(), 0.25);
    p3_ = WireGetPoint(wireBuilder.Wire(), 1.0);
    
    ASSERT_NEAR(0.0, p1_.Distance(p1), 1e-7);
    ASSERT_NEAR(0.0, p2_.Distance(p2), 1e-7);
    ASSERT_NEAR(0.0, p3_.Distance(p3), 1e-7);
    
    ASSERT_NEAR(0.0 , ProjectPointOnWire(wireBuilder.Wire(), p1_), 1e-7);
    ASSERT_NEAR(0.25, ProjectPointOnWire(wireBuilder.Wire(), p2_), 1e-7);
    ASSERT_NEAR(1.0 , ProjectPointOnWire(wireBuilder.Wire(), p3_), 1e-7);
}

TEST(Misc, GetPointOnCirc)
{
    // add a half circle
    BRepBuilderAPI_MakeWire wireBuilder(BRepBuilderAPI_MakeEdge(gp_Circ(gp::XOY() ,1.), -M_PI/2, M_PI/2));
    // add a line
    wireBuilder.Add(BRepBuilderAPI_MakeEdge(gp_Pnt(0,1,0), gp_Pnt(-M_PI, 1, 0)));
    const TopoDS_Wire& wire = wireBuilder.Wire();
    
    
    gp_Pnt p1(0, -1, 0);
    gp_Pnt p2(1, 0, 0);
    gp_Pnt p3(0, 1, 0);
    gp_Pnt p4(-M_PI/2., 1, 0);
    gp_Pnt p5(-M_PI, 1, 0);
    
    ASSERT_NEAR(0.0, p1.Distance(WireGetPoint(wire,0.00)), 1e-7);
    ASSERT_NEAR(0.0, p2.Distance(WireGetPoint(wire,0.25)), 1e-7);
    ASSERT_NEAR(0.0, p3.Distance(WireGetPoint(wire,0.50)), 1e-7);
    ASSERT_NEAR(0.0, p4.Distance(WireGetPoint(wire,0.75)), 1e-7);
    ASSERT_NEAR(0.0, p5.Distance(WireGetPoint(wire,1.00)), 1e-7);

    ASSERT_NEAR(0.0, p1.Distance(WireGetPoint(wire,0.00)), 1e-7);
    ASSERT_NEAR(0.0, p2.Distance(WireGetPoint(wire,0.25)), 1e-7);
    ASSERT_NEAR(0.0, p3.Distance(WireGetPoint(wire,0.50)), 1e-7);
    ASSERT_NEAR(0.0, p4.Distance(WireGetPoint(wire,0.75)), 1e-7);
    ASSERT_NEAR(0.0, p5.Distance(WireGetPoint(wire,1.00)), 1e-7);
    
    ASSERT_NEAR(0.00, ProjectPointOnWire(wire, p1), 1e-7);
    ASSERT_NEAR(0.25, ProjectPointOnWire(wire, p2), 1e-7);
    ASSERT_NEAR(0.50, ProjectPointOnWire(wire, p3), 1e-7);
    ASSERT_NEAR(0.75, ProjectPointOnWire(wire, p4), 1e-7);
    ASSERT_NEAR(1.00, ProjectPointOnWire(wire, p5), 1e-7);
}

TEST(Misc, GetErrorString)
{
    //check valid error codes
    ASSERT_STREQ("TIGL_SUCCESS", tiglGetErrorString(TIGL_SUCCESS));
    ASSERT_STREQ("TIGL_ERROR", tiglGetErrorString(TIGL_ERROR));
    ASSERT_STREQ("TIGL_NULL_POINTER", tiglGetErrorString(TIGL_NULL_POINTER));
    ASSERT_STREQ("TIGL_NOT_FOUND", tiglGetErrorString(TIGL_NOT_FOUND));
    ASSERT_STREQ("TIGL_XML_ERROR", tiglGetErrorString(TIGL_XML_ERROR));
    ASSERT_STREQ("TIGL_OPEN_FAILED", tiglGetErrorString(TIGL_OPEN_FAILED));
    ASSERT_STREQ("TIGL_CLOSE_FAILED", tiglGetErrorString(TIGL_CLOSE_FAILED));
    ASSERT_STREQ("TIGL_INDEX_ERROR", tiglGetErrorString(TIGL_INDEX_ERROR));
    ASSERT_STREQ("TIGL_STRING_TRUNCATED", tiglGetErrorString(TIGL_STRING_TRUNCATED));
    ASSERT_STREQ("TIGL_WRONG_TIXI_VERSION", tiglGetErrorString(TIGL_WRONG_TIXI_VERSION));
    ASSERT_STREQ("TIGL_UID_ERROR", tiglGetErrorString(TIGL_UID_ERROR));
    ASSERT_STREQ("TIGL_WRONG_CPACS_VERSION", tiglGetErrorString(TIGL_WRONG_CPACS_VERSION));
    ASSERT_STREQ("TIGL_UNINITIALIZED", tiglGetErrorString(TIGL_UNINITIALIZED));

    //Check invalid error code
    ASSERT_STREQ("TIGL_UNKNOWN_ERROR", tiglGetErrorString((TiglReturnCode)-1));
    ASSERT_STREQ("TIGL_UNKNOWN_ERROR", tiglGetErrorString((TiglReturnCode)100));
}

TEST(Misc, getLogLevelString)
{
    //check valid log levels
    ASSERT_STREQ("SLT" , tigl::getLogLevelString( TILOG_SILENT).c_str());
    ASSERT_STREQ("ERR" , tigl::getLogLevelString(  TILOG_ERROR).c_str());
    ASSERT_STREQ("WRN" , tigl::getLogLevelString(TILOG_WARNING).c_str());
    ASSERT_STREQ("INF" , tigl::getLogLevelString(   TILOG_INFO).c_str());
    ASSERT_STREQ("DBG" , tigl::getLogLevelString(  TILOG_DEBUG).c_str());
    ASSERT_STREQ("DBG1", tigl::getLogLevelString( TILOG_DEBUG1).c_str());
    ASSERT_STREQ("DBG2", tigl::getLogLevelString( TILOG_DEBUG2).c_str());
    ASSERT_STREQ("DBG3", tigl::getLogLevelString( TILOG_DEBUG3).c_str());
    ASSERT_STREQ("DBG4", tigl::getLogLevelString( TILOG_DEBUG4).c_str());
}

TEST(Misc, typeName)
{
    ASSERT_STREQ("tigl::CCPACSWing", tigl::typeName(typeid(tigl::CCPACSWing)).c_str());
    ASSERT_STREQ("tigl::CCPACSWing", tigl::typeName<tigl::CCPACSWing>().c_str());
}
