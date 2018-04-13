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

#include "CTiglExporterFactory.h"
#include "CGlobalExporterConfigs.h"
#include "stringtools.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace tigl
{

CTiglExporterFactory& CTiglExporterFactory::Instance()
{
    static CTiglExporterFactory factory;
    return factory;
}

void CTiglExporterFactory::RegisterExporter(ICADExporterBuilder *creator, const ExporterOptions& exporterConfig)
{
    if (creator) {
        // build an importer to get supported file type
        PTiglCADExporter exporter = creator->create();
        if (exporter) {
            std::string filetype = exporter->SupportedFileType();
            CGlobalExporterConfigs::Instance().Add(filetype, exporterConfig);
            std::vector<std::string> fileTypes = split_string(filetype, ';');
            for (std::vector<std::string>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it) {
                std::string type = *it;
                _exporterBuilders[to_lower(type)] = creator;
            }
        }
    }
}

PTiglCADExporter CTiglExporterFactory::Create(const std::string &filetype, const ExporterOptions& theOptions) const
{
    // make the requested filetype to lowercase
    std::string fileTypeToLower = to_lower(filetype);

    const ExporterOptions& options = theOptions.IsDefault() ? getExportConfig(fileTypeToLower) : theOptions;

    ExporterMap::const_iterator it = _exporterBuilders.find(fileTypeToLower);
    if (it != _exporterBuilders.end()) {
        ICADExporterBuilder* creator = it->second;
        return creator->create(options);
    }
    else {
        throw CTiglError("No exporter for type '" + filetype + "'", TIGL_NOT_FOUND);
    }
}

bool CTiglExporterFactory::ExporterSupported(const std::string &filetype) const
{
    // make the requested filetype to lowercase
    std::string fileTypeToLower = to_lower(filetype);

    ExporterMap::const_iterator it = _exporterBuilders.find(fileTypeToLower);
    if (it != _exporterBuilders.end()) {
        return true;
    }
    else {
        return false;
    }
}

CTiglExporterFactory::CTiglExporterFactory()
{
}

} // namespace tigl
