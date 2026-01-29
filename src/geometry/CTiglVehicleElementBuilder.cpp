/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-01-25 Marko Alder <marko.alder@dlr.de>
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

#include "CTiglVehicleElementBuilder.h"
#include "UniquePtr.h"
#include "CNamedShape.h"

#include "CTiglImporterFactory.h"
#include "CGroupShapes.h"

#include "tiglcommonfunctions.h"
#include "tiglexternalfilehelpers.h"

#include <BRepPrimAPI_MakeWedge.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>

namespace tigl
{

CTiglVehicleElementBuilder::CTiglVehicleElementBuilder(const CCPACSElementGeometry& geometry,
                                                       const CTiglTransformation& transformation,
                                                       const std::string& shapeName,
                                                       const std::string& cpacsDocumentPath)
    : m_geometry(&geometry)
    , m_transformation(&transformation)
    , m_shapeName(shapeName)
    , m_cpacsDocumentPath(cpacsDocumentPath)
{
}

PNamedShape CTiglVehicleElementBuilder::BuildShape()
{
    const auto& geom = *m_geometry;
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
    else if (auto& e = geom.GetExternal_choice5()) {
        elementShape = BuildExternalShape(*e);
    }
    else {
        throw CTiglError("Unsupported geometry type");
    }

    // Set shape name
    std::string loftName = "unnamed";
    if (!m_shapeName.empty()) {
        loftName = m_shapeName;
    }
    else if (const auto* parent = geom.GetNextUIDParent()) {
        loftName = parent->GetObjectUID().get_value_or(loftName);
    }
    PNamedShape loft(new CNamedShape(elementShape, loftName));

    // Apply transformation if available
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

    if (lengthX <= 0.0 || depthY <= 0.0 || heightZ <= 0.0) {
        auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        std::string errorMsg =
            "Invalid cuboid parameters for uID=\"" + uID + "\" : lengthX, depthY and heightZ must be positive.";
        throw tigl::CTiglError(errorMsg, TIGL_INVALID_VALUE);
    }

    const double xmin = c.GetUpperFaceXmin().get_value_or(0);
    const double xmax = c.GetUpperFaceXmax().get_value_or(lengthX);
    const double ymin = c.GetUpperFaceYmin().get_value_or(0);
    const double ymax = c.GetUpperFaceYmax().get_value_or(depthY);

    TopoDS_Shape wedge = BRepPrimAPI_MakeWedge(lengthX, heightZ, depthY, xmin, ymin, xmax, ymax).Shape();

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
        auto uID             = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        std::string errorMsg = "Invalid cylinder parameters for uID=\"" + uID +
                               "\" : Radius must be non-negative and height must be positive.";
        throw tigl::CTiglError(errorMsg, TIGL_INVALID_VALUE);
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
        auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        std::string errorMsg =
            "Invalid cone parameters for uID=\"" + uID + "\" : Radii must be non-negative and height must be positive.";
        throw tigl::CTiglError(errorMsg, TIGL_INVALID_VALUE);
    }

    TopoDS_Shape cylinder;

    if (std::abs(lowerRadius - upperRadius) < 1e-8) {
        cylinder = BRepPrimAPI_MakeCylinder(lowerRadius, height).Shape();
        auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        LOG(WARNING) << "Element with uID=\"" << uID
                     << "\" defines a cylinder via the cone definition! It is strongly recommended to use the cylinder "
                        "definition instead.";
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

TopoDS_Shape CTiglVehicleElementBuilder::BuildExternalShape(const CCPACSExternalGeometry& e)
{
    const auto& link = e.GetLinkToFile();

    if (!link.GetFormat()) {
        throw CTiglError("Cannot open external file. No file format specified.", TIGL_XML_ERROR);
    }

    const std::string fileType = CPACSLinkToFileType_formatToString(*link.GetFormat());

    PTiglCADImporter importer = CTiglImporterFactory::Instance().Create(fileType);
    if (!importer) {
        throw CTiglError("Cannot open externalComponent. Unknown file format " + fileType);
    }

    std::string filePath = link.GetValue();
    if (!m_cpacsDocumentPath.empty()) {
        filePath = evaluatePathRelativeToApp(m_cpacsDocumentPath, filePath);
    }
    ListPNamedShape shapes = importer->Read(filePath);

    // ToDo: Transformation should be slightly different from the one used in relatively positioned components:
    //       as there is no parent, there should not be a refType attribute.
    //       This change in XSD would affect CCPACSTransformation and CTiglTransformation.
    PNamedShape shapeGroup       = CGroupShapes(shapes);
    const CTiglTransformation tr = e.GetTransformation().getTransformationMatrix();
    shapeGroup->SetShape(tr.Transform(shapeGroup->Shape()));

    return shapeGroup->Shape();
}

CTiglVehicleElementBuilder::operator PNamedShape()
{
    return BuildShape();
};

} // namespace tigl
