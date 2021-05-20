/* 
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-02-17 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief Helper functions for tests
*/

#include"testUtils.h"
#include<fstream>
#include<iomanip>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <GeomConvert.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <BRepTools.hxx>
#include <TopoDS.hxx>

// save x-y data
void outputXY(const int & i, const double& x, const double&y, const std::string& filename)
{
    std::ofstream out;
    if (i>0) {
        out.open(filename.c_str(), std::ios::app);
    }
    else {
        out.open(filename.c_str());
    }
    out << std::setprecision(17) << std::scientific  << x << "\t" << y << std::endl;
    out.close();
} 
void outputXYVector(const int& i, const double& x, const double& y, const double& vx, const double& vy, const std::string& filename)
{
    std::ofstream out;
    if (i>0) {
        out.open(filename.c_str(), std::ios::app);
    }
    else {
        out.open(filename.c_str());
    }
    out << std::setprecision(17) << std::scientific  << x << "\t" << y << "\t" << vx << "\t" << vy << "\t" << std::endl;
    out.close();
}

void StoreResult(const std::string& filename, const Handle(Geom_BSplineCurve)& curve, const TColgp_Array1OfPnt& pt)
{
    TopoDS_Compound c;
    BRep_Builder b;
    b.MakeCompound(c);

    TopoDS_Shape e = BRepBuilderAPI_MakeEdge(curve);
    b.Add(c, e);

    for (Standard_Integer i = pt.Lower(); i <= pt.Upper(); ++i) {
        const gp_Pnt& p = pt.Value(i);
        TopoDS_Shape v = BRepBuilderAPI_MakeVertex(p);
        b.Add(c, v);
    }

    BRepTools::Write(c, filename.c_str());
}

Handle(Geom_BSplineCurve) LoadBSplineCurve(const std::string& filename)
{
    BRep_Builder b;
    TopoDS_Shape e;
    if (BRepTools::Read(e, filename.c_str(), b) != true) {
        return nullptr;
    }

    double umin, umax;
    return Handle(Geom_BSplineCurve)::DownCast(BRep_Tool::Curve(TopoDS::Edge(e), umin, umax));
}


Handle(Geom_BSplineSurface) LoadBSplineSurface(const std::string& filename)
{
    BRep_Builder builder;
    TopoDS_Shape shape;

    BRepTools::Read(shape, filename.c_str(), builder);
    return GeomConvert::SurfaceToBSplineSurface(BRep_Tool::Surface(TopoDS::Face(shape)));
}
