/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
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

#ifndef CTIGLEXPORTERFACTORY_H
#define CTIGLEXPORTERFACTORY_H

#include "tigl_internal.h"
#include "CTiglCADExporter.h"

#include <map>

namespace tigl
{

class CTiglExporterFactory
{
public:
    TIGL_EXPORT static CTiglExporterFactory& Instance();

    /// Registers a CAD Exporter at the factory
    TIGL_EXPORT void RegisterExporter(ICADExporterBuilder* creator, const ExporterOptions& exporterConfig);

    /// Creates an exporter for the matching file format
    /// Returns NULL, if the format is unknown
    TIGL_EXPORT PTiglCADExporter Create(const std::string& filetype, const ExporterOptions& options = DefaultExporterOption()) const;

    /// Returns true, if an exporter was registered for the specified file type
    TIGL_EXPORT bool ExporterSupported(const std::string& filetype) const;

private:
    CTiglExporterFactory();
    
    typedef std::map<std::string, ICADExporterBuilder*>  ExporterMap;
    ExporterMap _exporterBuilders;
};

inline PTiglCADExporter createExporter(const std::string& filetype, const ExporterOptions& options = DefaultExporterOption())
{
    return CTiglExporterFactory::Instance().Create(filetype, options);
}

} // namespace tigl

#endif // CTIGLEXPORTERFACTORY_H
