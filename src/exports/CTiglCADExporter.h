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
#include "COptionList.h"
#include "CCPACSImportExport.h"

namespace tigl
{

class CCPACSConfiguration;


class ExporterOptions : public COptionList
{
public:
    ExporterOptions()
    {
        AddOption("IsDefault", false);
        AddOption("ApplySymmetries", false);
        AddOption("IncludeFarfield", false);
        AddOption("ShapeGroupMode", WHOLE_SHAPE);
    }

    void SetApplySymmetries(bool apply)
    {
        Set("ApplySymmetries", apply);
    }

    void SetIncludeFarfield(bool include)
    {
        Set("IncludeFarfield", include);
    }

    void SetShapeGroupMode(tigl::ShapeGroupMode mode)
    {
        Set("ShapeGroupMode", mode);
    }

    ShapeGroupMode GroupMode() const
    {
        return Get<ShapeGroupMode>("ShapeGroupMode");
    }

    bool IsDefault() const
    {
        return Get<bool>("IsDefault");
    }
};

class DefaultExporterOption : public ExporterOptions
{
public:
    DefaultExporterOption()
    {
        Set("IsDefault", true);
    }
};

class ShapeExportOptions : public COptionList
{
public:
    ShapeExportOptions()
    {
        AddOption("IsDefault", false);
    }

    bool IsDefault() const
    {
        return Get<bool>("IsDefault");
    }
};

class DefaultShapeExportOptions : public ShapeExportOptions
{
public:
    DefaultShapeExportOptions()
    {
        Set("IsDefault", true);
    }
};

class TriangulatedExportOptions : public ShapeExportOptions
{
public:
    TriangulatedExportOptions()
    {
        AddOption("Deflection", 0.001);
    }

    TriangulatedExportOptions(double deflection)
    {
        AddOption("Deflection", deflection);
    }
};

/**
 * @brief Abstract base class for CAD exports
 */
class CTiglCADExporter
{
public:
    /// Constructor
    TIGL_EXPORT CTiglCADExporter(const ExporterOptions& options = DefaultExporterOption());

    // Empty destructor
    TIGL_EXPORT virtual ~CTiglCADExporter() { /* empty */}

    virtual ExporterOptions GetDefaultOptions() const = 0;
    virtual ShapeExportOptions GetDefaultShapeOptions() const = 0;

    /// Adds a shape
    TIGL_EXPORT void AddShape(PNamedShape shape, const ShapeExportOptions& options = DefaultShapeExportOptions());

    TIGL_EXPORT void AddShape(PNamedShape shape, const CCPACSConfiguration* config, const ShapeExportOptions& options = DefaultShapeExportOptions());
    

    ///  Adds the whole non-fused configuration, to the exporter
    TIGL_EXPORT void AddConfiguration(CCPACSConfiguration &config, const ShapeExportOptions& options = DefaultShapeExportOptions());

    /// Adds a whole geometry, boolean fused and meshed
    TIGL_EXPORT void AddFusedConfiguration(CCPACSConfiguration& config, const ShapeExportOptions& options = DefaultShapeExportOptions());

    TIGL_EXPORT bool Write(const std::string& filename) const;

    /// Number of shapes
    TIGL_EXPORT size_t NShapes() const;

    /// Returns all shapes added to the exporter
    TIGL_EXPORT PNamedShape GetShape(size_t iShape) const;

    TIGL_EXPORT const ShapeExportOptions& GetOptions(size_t iShape) const;
    TIGL_EXPORT const ExporterOptions& GlobalExportOptions() const;

    TIGL_EXPORT std::string SupportedFileType() const;

protected:
    /// Can also be NULL!
    const CCPACSConfiguration* GetConfiguration(size_t iShape) const;

private:
    /// must be overridden by the concrete implementation
    virtual bool WriteImpl(const std::string& filename) const = 0;
    
    /// must be overridden. If multiple types supported, separate with a ";"
    virtual std::string SupportedFileTypeImpl() const = 0;

    ListPNamedShape _shapes;
    mutable ExporterOptions _globalOptions;
    std::vector<ShapeExportOptions> _shapeOptions;
    std::vector<const CCPACSConfiguration*> _configs; //!< TIGL configurations */

};

typedef CSharedPtr<CTiglCADExporter> PTiglCADExporter;

class ICADExporterBuilder
{
public:
   virtual PTiglCADExporter create(const ExporterOptions& options = DefaultExporterOption()) const = 0;
};

template <class T>
class CCADExporterBuilder : public ICADExporterBuilder
{
public:
   PTiglCADExporter create(const ExporterOptions& options = DefaultExporterOption()) const OVERRIDE
   {
       return PTiglCADExporter(new T(options));
   }
};

} // namespace tigl

#endif // CTIGLCADEXPORTER_H
