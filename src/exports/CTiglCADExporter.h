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

#ifndef CTIGLCADEXPORTER_H
#define CTIGLCADEXPORTER_H

#include "tigl_internal.h"
#include "PNamedShape.h"
#include "ListPNamedShape.h"
#include "CTiglFusePlane.h"

namespace tigl
{

class CCPACSConfiguration;

struct ExportOptions
{
public:
    ExportOptions()
        : deflection(0.1)
        , applySymmetries(false)
        , includeFarField(true)
    {
    }

    double deflection;
    bool applySymmetries;
    bool includeFarField;
};

/**
 * @brief Abstract base class for CAD exports
 */
class CTiglCADExporter
{
public:
    /// Constructor
    TIGL_EXPORT CTiglCADExporter(){}

    // Empty destructor
    TIGL_EXPORT virtual ~CTiglCADExporter(void) { /* empty */}

    /// Adds a shape
    TIGL_EXPORT void AddShape(PNamedShape shape, ExportOptions options = ExportOptions());

    ///  Adds the whole non-fused configuration, to the exporter
    TIGL_EXPORT void AddConfiguration(CCPACSConfiguration &config, ExportOptions options = ExportOptions());

    /// Adds a whole geometry, boolean fused and meshed
    TIGL_EXPORT void AddFusedConfiguration(CCPACSConfiguration& config, ExportOptions options = ExportOptions());

    TIGL_EXPORT bool Write(const std::string& filename) const;

    /// Number of shapes
    TIGL_EXPORT size_t NShapes() const;

    /// Returns all shapes added to the exporter
    TIGL_EXPORT PNamedShape GetShape(size_t iShape) const;

    TIGL_EXPORT ExportOptions GetOptions(size_t iShape) const;

private:
    /// must be overridden by the concrete implementation
    virtual bool WriteImpl(const std::string& filename) const = 0;

    ListPNamedShape _shapes;
    std::vector<ExportOptions> _options;

};

} // namespace tigl

#endif // CTIGLCADEXPORTER_H
