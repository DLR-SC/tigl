/*
 * Copyright (C) 2018 German Aerospace Center (DLR/SC)
 *
 * Created: 2018-11-16 Jan Kleinert <jan.kleinert@dlr.de>
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

#include "CTiglVehicleElementBuilder.h"
#include "UniquePtr.h"
#include "CNamedShape.h"

#include <BRepPrimAPI_MakeWedge.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>

namespace tigl
{

CTiglVehicleElementBuilder::CTiglVehicleElementBuilder(const CCPACSVehicleElementBase& vehicleElement)
    : m_vehicleElement(vehicleElement) {};

CTiglVehicleElementBuilder::CTiglVehicleElementBuilder(const CCPACSVehicleElementBase& vehicleElement,
                                                       const CTiglTransformation& transformation)
    : m_vehicleElement(vehicleElement)
    , m_transformation(&transformation) {};

PNamedShape CTiglVehicleElementBuilder::BuildShape()
{
    const auto& geom = m_vehicleElement.GetGeometry();
    TopoDS_Shape elementShape;

    if (auto& p = geom.GetCuboid_choice1()) {
        elementShape = BuildCuboidShape(*p);
    }
    else if (auto& c = geom.GetCylinder_choice2()) {
        elementShape = BuildCylinderShape(*c);
    }
    else if (auto& c = geom.GetCone_choice3()) {
        elementShape = BuildConeShape(*c);
    }
    else if (auto& e = geom.GetEllipsoid_choice4()) {
        elementShape = BuildEllipsoidShape(*e);
    }
    else {
        throw CTiglError("Unsupported geometry type");
    }

    std::string loftName = m_vehicleElement.GetUID().c_str();
    PNamedShape loft(new CNamedShape(elementShape, loftName));

    if (m_transformation) {
        loft = m_transformation->Transform(loft);
    }

    return loft;
};

TopoDS_Shape CTiglVehicleElementBuilder::BuildCuboidShape(const CCPACSCuboid& c)
{
    const double lengthX = c.GetLengthX();
    const double depthY  = c.GetDepthY();
    const double heightZ = c.GetHeightZ();

    const double xmin = c.GetUpperFaceXmin().get_value_or(0);
    const double xmax = c.GetUpperFaceXmax().get_value_or(lengthX);
    const double ymin = c.GetUpperFaceYmin().get_value_or(0);
    const double ymax = c.GetUpperFaceYmax().get_value_or(depthY);

    TopoDS_Shape wedge = BRepPrimAPI_MakeWedge(lengthX, depthY, heightZ, xmin, ymin, xmax, ymax).Shape();

    // Rotate and translate from OCC to CPACS convention:
    gp_Ax1 xAxis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
    gp_Trsf rot;
    gp_Trsf trl;
    rot.SetRotation(xAxis, M_PI / 2.0);
    trl.SetTranslation(gp_Vec(0, depthY, 0));

    gp_Trsf comb = trl * rot;
    BRepBuilderAPI_Transform transformer(wedge, comb, /*copy=*/true);

    return transformer.Shape();
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildCylinderShape(const CCPACSCylinder& c)
{
    const double radius = c.GetRadius();
    const double height = c.GetHeight();

    if (radius < 0.0 || height <= 0.0) {
        throw tigl::CTiglError("Invalid cylinder parameters: Radius must be non-negative and height must be positive.",
                               TIGL_INVALID_VALUE);
    }

    TopoDS_Shape cylinder;

    cylinder = BRepPrimAPI_MakeCylinder(radius, height).Shape();

    return cylinder;
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildConeShape(const CCPACSCone& c)
{
    const double lowerRadius = c.GetLowerRadius();
    const double upperRadius = c.GetUpperRadius().get_value_or(0);
    const double height      = c.GetHeight();

    if (lowerRadius < 0.0 || upperRadius < 0.0 || height <= 0.0) {
        throw tigl::CTiglError("Invalid cone parameters: Radii must be non-negative and height must be positive.",
                               TIGL_INVALID_VALUE);
    }

    TopoDS_Shape cylinder;

    if (std::abs(lowerRadius - upperRadius) < 1e-8) {
        cylinder = BRepPrimAPI_MakeCylinder(lowerRadius, height).Shape();
        auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        LOG(WARNING) << "Element with uID=\"" << uID
                     << "\" defines a cylinder via the cone definition! It is strongly recommended to use the cylinder definition instead.";
    }
    else {
        cylinder = BRepPrimAPI_MakeCone(lowerRadius, upperRadius, height).Shape();
    }

    return cylinder;
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildEllipsoidShape(const CCPACSEllipsoid& e)
{
    double radiusX = e.GetRadiusX();
    double radiusY = e.GetRadiusY().get_value_or(radiusX);
    double radiusZ = e.GetRadiusZ().get_value_or(radiusX);

    if (radiusX <= 0.0 || radiusY <= 0.0 || radiusZ <= 0.0) {
        throw tigl::CTiglError("Invalid ellipsoid parameters: All radii must be positive.", TIGL_INVALID_VALUE);
    }

    TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(1.0).Shape();

    gp_Mat M(radiusX, 0.0, 0.0, 0.0, radiusY, 0.0, 0.0, 0.0, radiusZ);
    gp_GTrsf gtrsf(M, gp_XYZ(0.0, 0.0, 0.0));
    BRepBuilderAPI_GTransform transformer(sphere, gtrsf, true);

    return transformer.Shape();
}

CTiglVehicleElementBuilder::operator PNamedShape()
{
    return BuildShape();
};

} // namespace tigl
