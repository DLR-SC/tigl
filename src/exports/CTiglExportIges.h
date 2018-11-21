/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
* @brief  Export routines for CPACS configurations.
*/

#ifndef CTIGLEXPORTIGES_H
#define CTIGLEXPORTIGES_H

#include "tigl_config.h"
#include "tigl_internal.h"
#include "CCPACSImportExport.h"
#include "CTiglCADExporter.h"


class IGESControl_Writer;

namespace tigl 
{

class CCPACSConfiguration;

enum FaceNameSettings
{
    UIDOnly,
    FaceNameOnly,
    UIDandFaceName,
    None
};

class IgesOptions : public ExporterOptions
{
public:
    IgesOptions()
    {
        Set("ApplySymmetries", false);
        Set("IncludeFarfield", true);
        Set("ShapeGroupMode", NAMED_COMPOUNDS);
        AddOption("IGES5.3", true);
        AddOption("FaceNames", FaceNameOnly);
    }
};

class IgesShapeOptions : public ShapeExportOptions
{
public:
    IgesShapeOptions()
    {
        AddOption("Layer", -1);
    }

    IgesShapeOptions(int layer)
    {
        AddOption("Layer", layer);
    }

    void SetLayer(int layer)
    {
        Set("Layer", layer);
    }
};

class CTiglExportIges : public CTiglCADExporter
{

public:
    // Constructor
    TIGL_EXPORT CTiglExportIges(const ExporterOptions& = DefaultExporterOption());

    TIGL_EXPORT ExporterOptions GetDefaultOptions() const OVERRIDE;
    TIGL_EXPORT ShapeExportOptions GetDefaultShapeOptions() const OVERRIDE;

private:
    // Actual implementation of the IGES file writing
    TIGL_EXPORT bool WriteImpl(const std::string& filename) const OVERRIDE;

    std::string SupportedFileTypeImpl() const OVERRIDE
    {
        return "igs;iges";
    }

    // Assignment operator
    void operator=(const CTiglExportIges& );
    void AddToIges(PNamedShape shape, IGESControl_Writer& writer, int level = 0) const;

    void SetTranslationParameters() const;
};

template <>
inline void from_string<FaceNameSettings>(const std::string &s, FaceNameSettings &t)
{
    std::string value = tigl::to_upper(s);
    if (value == "UIDONLY") {
        t = UIDOnly;
    }
    else if (value == "FACENAMEONLY") {
        t = FaceNameOnly;
    }
    else if (value == "UIDANDFACENAME") {
        t = UIDandFaceName;
    }
    else if (value == "NONE") {
        t = None;
    }
    else {
        throw CTiglError("Cannot convert string to FaceNameSettings");
    }
}

} // end namespace tigl

#endif // CTIGLEXPORTIGES_H
