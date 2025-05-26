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
#include "CPACSCuboid.h"
#include "CCPACSFrustum.h"
#include "CCPACSEllipsoid.h"

#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>

#include <BRepOffsetAPI_ThruSections.hxx>
#include <Geom_Ellipse.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepPrimAPI_MakeCone.hxx>

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
    else if (auto& f = geom.GetFrustum_choice2()) {
        elementShape = BuildFrustumShape(*f);
    }
    else if (auto& e = geom.GetEllipsoid_choice3()) {
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

TopoDS_Shape CTiglVehicleElementBuilder::BuildCuboidShape(const CCPACSCuboid& p)
{
    const double a = p.GetLengthX();
    const double b = p.GetDepthY();
    const double c = p.GetHeightZ();

    const double alpha = p.getAlpha() * M_PI / 180.0;
    const double beta  = p.getBeta() * M_PI / 180.0;
    const double gamma = p.getGamma() * M_PI / 180.0;

    gp_Vec vA{a, 0, 0};
    gp_Vec vB{b * std::cos(gamma), b * std::sin(gamma), 0};
    const double zC =
        c * std::sqrt(1 - std::pow(std::cos(beta), 2) -
                      std::pow((std::cos(alpha) - std::cos(beta) * std::cos(gamma)) / std::sin(gamma), 2));
    gp_Vec vC{c * std::cos(beta), c * (std::cos(alpha) - std::cos(beta) * std::cos(gamma)) / std::sin(gamma), zC};

    BRepBuilderAPI_MakePolygon poly;
    const gp_Pnt p0{0, 0, 0};
    poly.Add(p0);
    poly.Add(p0.Translated(vA));
    poly.Add(p0.Translated(vA).Translated(vB));
    poly.Add(p0.Translated(vB));
    poly.Close();
    TopoDS_Face base = BRepBuilderAPI_MakeFace(poly.Wire());

    TopoDS_Shape parallelepiped = BRepPrimAPI_MakePrism(base, vC).Shape();

    gp_Vec center = (vA + vB + vC) * 0.5;
    gp_Trsf tr;
    tr.SetTranslation(gp_Vec(-center.X(), -center.Y(), -center.Z()));
    TopoDS_Shape centered_parallelepiped = BRepBuilderAPI_Transform(parallelepiped, tr, true).Shape();

    return centered_parallelepiped;
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildFrustumShape(const CCPACSFrustum& f)
{
    try {
        const double lowerRadius = f.GetLowerRadius();
        const double upperRadius = f.getUpperRadius();
        const double height      = f.GetHeight();

        if (lowerRadius < 0.0 || upperRadius < 0.0 || height <= 0.0) {
            throw std::invalid_argument(
                "Invalid frustum parameters: Radii must be non-negative and height must be positive.");
        }

        TopoDS_Shape frustum;

        if (std::abs(lowerRadius - upperRadius) < 1e-8) {
            frustum = BRepPrimAPI_MakeCylinder(lowerRadius, height).Shape();
        }
        else {
            frustum = BRepPrimAPI_MakeCone(lowerRadius, upperRadius, height).Shape();
        }

        gp_Trsf translation;
        translation.SetTranslation(gp_Vec(0, 0, -height * 0.5));
        BRepBuilderAPI_Transform transformer(frustum, translation);
        frustum = transformer.Shape();

        return frustum;
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OpenCASCADE error while building frustum: " << e.GetMessageString() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception while building frustum: " << e.what() << std::endl;
    }

    return TopoDS_Shape();
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildEllipsoidShape(const CCPACSEllipsoid& e)
{
    try {
        double radiusX = e.GetRadiusX();
        double radiusY = e.getRadiusY();
        double radiusZ = e.getRadiusZ();

        if (radiusX <= 0.0 || radiusY <= 0.0 || radiusZ <= 0.0) {
            throw std::invalid_argument("Invalid ellipsoid parameters: All radii must be positive.");
        }

        TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(1.0).Shape();

        gp_Mat M(radiusX, 0.0, 0.0, 0.0, radiusY, 0.0, 0.0, 0.0, radiusZ);
        gp_GTrsf gtrsf(M, gp_XYZ(0.0, 0.0, 0.0));
        BRepBuilderAPI_GTransform transformer(sphere, gtrsf, true);

        return transformer.Shape();
    }
    catch (const Standard_Failure& e) {
        std::cerr << "OpenCASCADE error while building ellipsoid: " << e.GetMessageString() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception while building ellipsoid: " << e.what() << std::endl;
    }

    return TopoDS_Shape();
}

CTiglVehicleElementBuilder::operator PNamedShape()
{
    return BuildShape();
};

} // namespace tigl
