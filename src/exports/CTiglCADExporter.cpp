/*
* Copyright (C) 2016 German Aerospace Center (DLR/SC)
*
* Created: 2016-12-21 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglCADExporter.h"

#include "CTiglFusePlane.h"
#include "CCPACSConfiguration.h"
#include "CCPACSWingSegment.h"
#include "CCPACSFuselageSegment.h"
#include "CCPACSExternalObject.h"

#include <string>
#include <algorithm>
#include <cassert>


namespace tigl
{

bool CTiglCADExporter::Write(const std::string &filename) const
{
    return WriteImpl(filename);
}

CTiglCADExporter::CTiglCADExporter(const ExporterOptions &options)
    : _globalOptions(options)
{
}

void CTiglCADExporter::AddShape(PNamedShape shape, const ShapeExportOptions& options)
{
    AddShape(shape, NULL, options);
}

void CTiglCADExporter::AddShape(PNamedShape shape, const CCPACSConfiguration* config, const ShapeExportOptions& options)
{
    if (shape) {
        _shapes.push_back(shape);
        _shapeOptions.push_back(options.Get<bool>("IsDefault") ? GetDefaultShapeOptions() : options);
        _configs.push_back(config);
    }
}

void CTiglCADExporter::AddConfiguration(CCPACSConfiguration& config, const ShapeExportOptions& options)
{
    // Export all wings of the configuration
    for (int w = 1; w <= config.GetWingCount(); w++) {
        CCPACSWing& wing = config.GetWing(w);

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            AddShape(loft, &config, options);

            if (GlobalExportOptions().Get<bool>("ApplySymmetries") && segment.GetSymmetryAxis() != TIGL_NO_SYMMETRY) {
                AddShape(segment.GetMirroredLoft(), &config, options);
            }
        }
    }

    // Export all fuselages of the configuration
    for (int f = 1; f <= config.GetFuselageCount(); f++) {
        CCPACSFuselage& fuselage = config.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            PNamedShape loft = segment.GetLoft();
            AddShape(loft, &config, options);

            if (GlobalExportOptions().Get<bool>("ApplySymmetries") && segment.GetSymmetryAxis() != TIGL_NO_SYMMETRY) {
                AddShape(segment.GetMirroredLoft(), &config, options);
            }
        }
    }

    // Export external objects
    for (int e = 1; e <= config.GetExternalObjectCount(); e++) {
        CCPACSExternalObject& obj = config.GetExternalObject(e);
        PNamedShape loft = obj.GetLoft();
        AddShape(loft, &config, options);

        if (GlobalExportOptions().Get<bool>("ApplySymmetries") && obj.GetSymmetryAxis() != TIGL_NO_SYMMETRY) {
            AddShape(obj.GetMirroredLoft(), &config, options);
        }
    }

    if (GlobalExportOptions().Get<bool>("IncludeFarfield")) {
        CCPACSFarField& farfield = config.GetFarField();
        if (farfield.GetType() != NONE) {
            AddShape(farfield.GetLoft(), &config, options);
        }
    }
}

void CTiglCADExporter::AddFusedConfiguration(CCPACSConfiguration &config, const ShapeExportOptions& options)
{
    PTiglFusePlane fuser = config.AircraftFusingAlgo();

    TiglFuseResultMode mode = HALF_PLANE_TRIMMED_FF;
    if (GlobalExportOptions().Get<bool>("ApplySymmetries")) {
        mode = GlobalExportOptions().Get<bool>("IncludeFarfield") ? FULL_PLANE_TRIMMED_FF : FULL_PLANE;
    }
    else {
        mode = GlobalExportOptions().Get<bool>("IncludeFarfield") ? HALF_PLANE_TRIMMED_FF : HALF_PLANE;
    }

    fuser->SetResultMode(mode);
    assert(fuser);

    PNamedShape fusedAirplane = fuser->FusedPlane();
    PNamedShape farField      = fuser->FarField();
    if (!fusedAirplane) {
        throw CTiglError("Error computing fused airplane.", TIGL_NULL_POINTER);
    }

    AddShape(fusedAirplane, &config, options);
    AddShape(farField, &config, options);

    // add intersections
    const ListPNamedShape& ints = fuser->Intersections();
    ListPNamedShape::const_iterator it;
    for (it = ints.begin(); it != ints.end(); ++it) {
        AddShape(*it, &config, options);
    }
}

size_t CTiglCADExporter::NShapes() const
{
    return _shapes.size();
}

PNamedShape CTiglCADExporter::GetShape(size_t iShape) const
{
    return _shapes.at(iShape);
}

const ShapeExportOptions& CTiglCADExporter::GetOptions(size_t iShape) const
{
    return _shapeOptions.at(iShape);
}

const ExporterOptions &CTiglCADExporter::GlobalExportOptions() const
{
    if (_globalOptions.IsDefault()) {
        _globalOptions = GetDefaultOptions();
    }
    return _globalOptions;
}

const CCPACSConfiguration *CTiglCADExporter::GetConfiguration(size_t iShape) const
{
    return _configs.at(iShape);
}

std::string CTiglCADExporter::SupportedFileType() const
{
    return SupportedFileTypeImpl();
}

} // namespace tigl
