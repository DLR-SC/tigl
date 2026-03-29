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

#include "CTiglElementGeometryBuilder.h"
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

#include <CCPACSFuselageSegment.h>
#include <CCPACSFuselageSegments.h>
#include "CTiglMakeLoft.h"

namespace tigl
{

CTiglElementGeometryBuilder::CTiglElementGeometryBuilder(const CTiglRelativelyPositionedComponent& refComponent,
                                                         const CCPACSConfiguration& refConfig,
                                                         const CCPACSElementGeometry& geometry,
                                                         const std::string& shapeName,
                                                         const std::string& cpacsDocumentPath)
    : m_refComponent(&refComponent)
    , m_refConfig(&refConfig)
    , m_geometry(&geometry)
    , m_shapeName(shapeName)
    , m_cpacsDocumentPath(cpacsDocumentPath)
{
}

PNamedShape CTiglElementGeometryBuilder::BuildShape() const
{
    const auto& geom = *m_geometry;

    // Set shape name
    std::string shapeName = "unnamed";
    if (!m_shapeName.empty()) {
        shapeName = m_shapeName;
    }
    else if (const auto* parent = geom.GetNextUIDParent()) {
        shapeName = parent->GetObjectUID().get_value_or(shapeName);
    }

    // Build shapes
    ListPNamedShape shapes;

    auto addPart = [&](const std::string& type, const TopoDS_Shape& s, size_t idx) {
        const std::string n = shapeName + "_" + type + "_" + std::to_string(idx);
        shapes.push_back(PNamedShape(new CNamedShape(s, n)));
    };

    if (const auto& cuboidsOpt = geom.GetCuboids(); cuboidsOpt) {
        const auto& cuboids = *cuboidsOpt;
        for (size_t i = 1; i <= cuboids.GetCuboidCount(); ++i) {
            const auto& c = cuboids.GetCuboid(i);
            addPart("cuboid", BuildCuboidShape(c), i);
        }
    }

    if (const auto& cylindersOpt = geom.GetCylinders(); cylindersOpt) {
        const auto& cylinders = *cylindersOpt;
        for (size_t i = 1; i <= cylinders.GetCylinderCount(); ++i) {
            const auto& c = cylinders.GetCylinder(i);
            addPart("cylinder", BuildCylinderShape(c), i);
        }
    }

    if (const auto& conesOpt = geom.GetCones(); conesOpt) {
        const auto& cones = *conesOpt;
        for (size_t i = 1; i <= cones.GetConeCount(); ++i) {
            const auto& c = cones.GetCone(i);
            addPart("cone", BuildConeShape(c), i);
        }
    }

    if (const auto& ellipsoidsOpt = geom.GetEllipsoids(); ellipsoidsOpt) {
        const auto& ellipsoids = *ellipsoidsOpt;
        for (size_t i = 1; i <= ellipsoids.GetEllipsoidCount(); ++i) {
            const auto& e = ellipsoids.GetEllipsoid(i);
            addPart("ellipsoid", BuildEllipsoidShape(e), i);
        }
    }

    if (const auto& mssOpt = geom.GetMultiSegmentShapes(); mssOpt) {
        const auto& mss = *mssOpt;
        for (size_t i = 1; i <= mss.GetMultiSegmentShapeCount(); ++i) {
            const auto& m = mss.GetMultiSegmentShape(i);
            addPart("multiSegmentShape", BuildMultiSegmentShape(m), i);
        }
    }

    if (const auto& externalsOpt = geom.GetExternals(); externalsOpt) {
        const auto& exts = *externalsOpt;
        for (size_t i = 1; i <= exts.GetExternalCount(); ++i) {
            const auto& e = exts.GetExternal(i);
            addPart("external", BuildExternalShape(e), i);
        }
    }

    if (shapes.empty()) {
        std::string uid = "unknown";
        if (const auto* parent = geom.GetNextUIDParent()) {
            uid = parent->GetObjectUID().get_value_or(uid);
        }
        throw CTiglError("No geometry primitives defined for uID=\"" + uid + "\"");
    }

    // build a base/compound shape from all parts
    PNamedShape groupedShape = CGroupShapes(shapes);

    // apply transformation from geom
    if (const auto& optTr = geom.GetTransformation(); optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        groupedShape->SetShape(tr.Transform(groupedShape->Shape()));
    }

    return groupedShape;
}

TopoDS_Shape CTiglElementGeometryBuilder::BuildCuboidShape(const CCPACSCuboid& c) const
{
    const double lengthX = c.GetLengthX();
    const double depthY  = c.GetDepthY();
    const double heightZ = c.GetHeightZ();

    if (lengthX <= 0.0 || depthY <= 0.0 || heightZ <= 0.0) {
        const auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        const std::string errorMsg =
            "Invalid cuboid parameters for uID=\"" + uID + "\": lengthX, depthY and heightZ must be positive.";
        throw tigl::CTiglError(errorMsg, TIGL_INVALID_VALUE);
    }

    const double xmin = c.GetUpperFaceXmin().get_value_or(0);
    const double xmax = c.GetUpperFaceXmax().get_value_or(lengthX);
    const double ymin = c.GetUpperFaceYmin().get_value_or(0);
    const double ymax = c.GetUpperFaceYmax().get_value_or(depthY);

    const TopoDS_Shape wedge =
        BRepPrimAPI_MakeWedge(lengthX, heightZ, depthY, xmin, depthY - ymax, xmax, depthY - ymin).Shape();

    // Rotate and translate from OCC to CPACS convention:
    const gp_Ax1 xAxis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
    gp_Trsf rot;
    gp_Trsf trl;
    rot.SetRotation(xAxis, M_PI / 2.0);
    trl.SetTranslation(gp_Vec(0, depthY, 0));

    gp_Trsf comb       = trl * rot;
    TopoDS_Shape shape = BRepBuilderAPI_Transform(wedge, comb, /*copy=*/true).Shape();

    // apply SE3 transformation if present
    const auto& optTr = c.GetTransformation();
    if (optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        shape                        = tr.Transform(shape);
    }

    return shape;
}

TopoDS_Shape CTiglElementGeometryBuilder::BuildCylinderShape(const CCPACSCylinder& c) const
{
    const double radius = c.GetRadius();
    const double height = c.GetHeight();

    if (radius <= 0.0 || height <= 0.0) {
        const auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        const std::string errorMsg =
            "Invalid cylinder parameters for uID=\"" + uID + "\": Radius and height must be positive.";
        throw tigl::CTiglError(errorMsg, TIGL_INVALID_VALUE);
    }

    TopoDS_Shape shape = BRepPrimAPI_MakeCylinder(radius, height).Shape();

    // apply transformation if present
    const auto& optTr = c.GetTransformation();
    if (optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        shape                        = tr.Transform(shape);
    }

    return shape;
}

TopoDS_Shape CTiglElementGeometryBuilder::BuildConeShape(const CCPACSCone& c) const
{
    const double lowerRadius = c.GetLowerRadius();
    const double upperRadius = c.GetUpperRadius().get_value_or(0);
    const double height      = c.GetHeight();

    if (lowerRadius < 0.0 || upperRadius < 0.0 || height <= 0.0 || (lowerRadius == 0.0 && upperRadius == 0.0)) {
        const auto uID             = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        const std::string errorMsg = "Invalid cone parameters for uID=\"" + uID +
                                     "\": At least one radius must be positive and height must be positive.";
        throw tigl::CTiglError(errorMsg, TIGL_INVALID_VALUE);
    }

    TopoDS_Shape shape;

    if (std::abs(lowerRadius - upperRadius) < 1e-8) {
        shape          = BRepPrimAPI_MakeCylinder(lowerRadius, height).Shape();
        const auto uID = c.GetNextUIDParent()->GetObjectUID().get_value_or("unknown");
        LOG(WARNING) << "Element with uID=\"" << uID
                     << "\" defines a cylinder via the cone definition! It is strongly recommended to use the cylinder "
                        "definition instead.";
    }
    else {
        shape = BRepPrimAPI_MakeCone(lowerRadius, upperRadius, height).Shape();
    }

    // apply transformation if present (robust against dangling-pointer bug)
    const auto& optTr = c.GetTransformation();
    if (optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        shape                        = tr.Transform(shape);
    }

    return shape;
}

TopoDS_Shape CTiglElementGeometryBuilder::BuildEllipsoidShape(const CCPACSEllipsoid& e) const
{
    const double radiusX = e.GetRadiusX();
    const double radiusY = e.GetRadiusY().get_value_or(radiusX);
    const double radiusZ = e.GetRadiusZ().get_value_or(radiusX);
    const double angle   = e.GetDiskAngle().get_value_or(2.0 * M_PI);

    if (radiusX <= 0.0 || radiusY <= 0.0 || radiusZ <= 0.0) {
        throw tigl::CTiglError("Invalid ellipsoid parameters: All radii must be positive.", TIGL_INVALID_VALUE);
    }

    if (angle <= 0.0 || angle > 2.0 * M_PI) {
        throw tigl::CTiglError("Invalid ellipsoid diskAngle: must be in range (0, 2*pi].", TIGL_INVALID_VALUE);
    }

    const TopoDS_Shape sphere = BRepPrimAPI_MakeSphere(1.0, angle).Shape();

    gp_Mat M(radiusX, 0.0, 0.0, 0.0, radiusY, 0.0, 0.0, 0.0, radiusZ);
    gp_GTrsf gtrsf(M, gp_XYZ(0.0, 0.0, 0.0));
    BRepBuilderAPI_GTransform transformer(sphere, gtrsf, true);

    TopoDS_Shape shape = transformer.Shape();

    // apply transformation if present
    const auto& optTr = e.GetTransformation();
    if (optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        shape                        = tr.Transform(shape);
    }

    return shape;
}

TopoDS_Shape CTiglElementGeometryBuilder::BuildMultiSegmentShape(const CCPACSMultiSegmentShape& m) const
{
    auto const& segments = m.GetSegments();
    segments.SetReferenceParent(m_refComponent);
    segments.SetConfiguration(m_refConfig);

    const int nSeg = segments.GetSegmentCount();

    if (nSeg < 1) {
        throw CTiglError("Cannot build multi-segment shape: no segments defined.", TIGL_INVALID_VALUE);
    }

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
    lofter.setMakeSmooth(m.GetSmooth().get_value_or(true));

    TopoDS_Shape shape = lofter.Shape();

    // apply SE3 transformation if present
    if (const auto& optTr = m.GetTransformation(); optTr) {
        const CTiglTransformation tr = optTr->getTransformationMatrix();
        shape                        = tr.Transform(shape);
    }

    return shape;
}

TopoDS_Shape CTiglElementGeometryBuilder::BuildExternalShape(const CCPACSExternalGeometry& e) const
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
    const ListPNamedShape shapes = importer->Read(filePath);

    // ToDo: Transformation should be slightly different from the one used in relatively positioned components:
    //       as there is no parent, there should not be a refType attribute.
    //       This change in XSD would affect CCPACSTransformation and CTiglTransformation.
    const PNamedShape shapeGroup = CGroupShapes(shapes);

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
