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

#include <generated/CPACSSubElements.h>
#include <generated/CPACSSubElement.h>
#include <CCPACSFuselageSegment.h>
#include <CCPACSFuselageSegments.h>
#include "CTiglMakeLoft.h"

namespace tigl
{

CTiglVehicleElementBuilder::CTiglVehicleElementBuilder(const CTiglRelativelyPositionedComponent& refComponent,
                                                       const CCPACSConfiguration& refConfig,
                                                       const CCPACSElementGeometry& geometry,
                                                       const std::string& shapeName,
                                                       const std::string& cpacsDocumentPath)
    : m_refComponent(&refComponent)
    , m_refConfig(&refConfig)
    , m_geometry(&geometry)
    , m_transformation(refComponent.GetTransformationMatrix())
    , m_shapeName(shapeName)
    , m_cpacsDocumentPath(cpacsDocumentPath)
{
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildSingleShape(const CCPACSElementGeometry& geom)
{
    return BuildSingleShapeImpl(geom);
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildSingleShape(const CCPACSSubElement& geom)
{
    return BuildSingleShapeImpl(geom);
}

PNamedShape CTiglVehicleElementBuilder::BuildShape()
{
    const auto& geom = *m_geometry;

    // Set shape name
    std::string baseName = "unnamed";
    if (!m_shapeName.empty()) {
        baseName = m_shapeName;
    }
    else if (const auto* parent = geom.GetNextUIDParent()) {
        baseName = parent->GetObjectUID().get_value_or(baseName);
    }

    // Base shape
    TopoDS_Shape baseShape = BuildSingleShape(geom);

    // Subelement shapes
    ListPNamedShape shapes;
    shapes.push_back(PNamedShape(new CNamedShape(baseShape, baseName)));

    // Apply subelement shapes
    if (geom.GetSubElements()) {
        const auto& subElements = *geom.GetSubElements();

        const auto& elementList = subElements.GetSubElements();
        for (size_t i = 0; i < elementList.size(); ++i) {
            const auto& element = *elementList[i];

            TopoDS_Shape elementShape = BuildSingleShape(element);

            // Transformation w.r.t. base shape
            const CTiglTransformation elementTr = element.GetTransformation().getTransformationMatrix();
            elementShape                        = elementTr.Transform(elementShape);

            const std::string elementName = baseName + "_subElement_" + std::to_string(i + 1);

            shapes.push_back(PNamedShape(new CNamedShape(elementShape, elementName)));
        }
    }
    PNamedShape groupedShape = CGroupShapes(shapes);

    // Apply transformation if available
    if (m_transformation) {
        groupedShape = m_transformation->Transform(groupedShape);
    }

    return groupedShape;
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildCuboidShape(const CCPACSCuboid& c)
{
    const double lengthX = c.GetLengthX();
    const double depthY  = c.GetDepthY();
    const double heightZ = c.GetHeightZ();

    if (lengthX <= 0.0 || depthY <= 0.0 || heightZ <= 0.0) {
        auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        std::string errorMsg =
            "Invalid cuboid parameters for uID=\"" + uID + "\": lengthX, depthY and heightZ must be positive.";
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

    if (radius <= 0.0 || height <= 0.0) {
        auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        std::string errorMsg =
            "Invalid cylinder parameters for uID=\"" + uID + "\": Radius and height must be positive.";
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

    if (lowerRadius < 0.0 || upperRadius < 0.0 || height <= 0.0 || (lowerRadius == 0.0 && upperRadius == 0.0)) {
        auto uID             = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        std::string errorMsg = "Invalid cone parameters for uID=\"" + uID +
                               "\": At least one radius must be positive and height must be positive.";
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
    double radiusX     = e.GetRadiusX();
    double radiusY     = e.GetRadiusY().get_value_or(radiusX);
    double radiusZ     = e.GetRadiusZ().get_value_or(radiusX);
    const double angle = e.GetDiskAngle().get_value_or(2.0 * M_PI);

    if (radiusX <= 0.0 || radiusY <= 0.0 || radiusZ <= 0.0) {
        throw tigl::CTiglError("Invalid ellipsoid parameters: All radii must be positive.", TIGL_INVALID_VALUE);
    }

    if (angle <= 0.0 || angle > 2.0 * M_PI) {
        throw tigl::CTiglError("Invalid ellipsoid diskAngle: must be in range (0, 2*pi].", TIGL_INVALID_VALUE);
    }

    TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(1.0, angle).Shape();

    gp_Mat M(radiusX, 0.0, 0.0, 0.0, radiusY, 0.0, 0.0, 0.0, radiusZ);
    gp_GTrsf gtrsf(M, gp_XYZ(0.0, 0.0, 0.0));
    BRepBuilderAPI_GTransform transformer(sphere, gtrsf, true);

    return transformer.Shape();
}

TopoDS_Shape CTiglVehicleElementBuilder::BuildMultiSegmentShape(const CCPACSMultiSegmentShape& m)
{
    auto const& segments = m.GetSegments();
    segments.SetReferenceParent(m_refComponent);
    segments.SetConfiguration(m_refConfig);

    const int nSeg = segments.GetSegmentCount();

    if (nSeg < 1) {
        throw CTiglError("Cannot build multi-segment shape: no segments defined.", TIGL_INVALID_VALUE);
    }

    const TiglContinuity cont     = segments.GetSegment(1).GetContinuity();
    const Standard_Boolean smooth = (cont != ::C0);

    CTiglMakeLoft lofter;

    // profiles: start wire of each segment
    for (int i = 1; i <= nSeg; ++i) {
        lofter.addProfiles(segments.GetSegment(i).GetStartWire(FUSELAGE_COORDINATE_SYSTEM));
    }

    // final profile: end wire of last segment
    lofter.addProfiles(segments.GetSegment(nSeg).GetEndWire(FUSELAGE_COORDINATE_SYSTEM));

    // guides
    lofter.addGuides(segments.GetGuideCurveWires());

    // loft options
    lofter.setMakeSolid(true);
    lofter.setMakeSmooth(smooth);

    TopoDS_Shape shape = lofter.Shape();

    // apply SE3 transformation if present
    if (const auto& optTr = m.GetTransformation(); optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        shape                        = tr.Transform(shape);
    }

    return shape;
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
        throw CTiglError("Cannot open external element. Unknown file format: " + fileType);
    }

    std::string filePath = link.GetValue();
    if (!m_cpacsDocumentPath.empty()) {
        filePath = evaluatePathRelativeToApp(m_cpacsDocumentPath, filePath);
    }
    ListPNamedShape shapes = importer->Read(filePath);

    // ToDo: Transformation should be slightly different from the one used in relatively positioned components:
    //       as there is no parent, there should not be a refType attribute.
    //       This change in XSD would affect CCPACSTransformation and CTiglTransformation.
    PNamedShape shapeGroup = CGroupShapes(shapes);

    TopoDS_Shape shape = shapeGroup->Shape();

    // apply transformation if present
    const auto& optTr = e.GetTransformation();
    if (optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        shape                        = tr.Transform(shape);
    }

    return shape;
}

} // namespace tigl
