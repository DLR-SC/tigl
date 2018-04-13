/* 
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-04-04 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CGlobalExporterConfigs.h"
#include "stringtools.h"

namespace tigl
{

CGlobalExporterConfigs &CGlobalExporterConfigs::Instance()
{
    static CGlobalExporterConfigs globalConfig;
    return globalConfig;
}

void CGlobalExporterConfigs::Add(const std::string &supportedFileTypes, const ExporterOptions &config)
{
    m_configs.push_back(config);
    size_t idx = m_configs.size() - 1;
    
    std::vector<std::string> fileTypes = split_string(supportedFileTypes, ';');
    for (std::vector<std::string>::const_iterator it = fileTypes.begin(); it != fileTypes.end(); ++it) {
        std::string type = *it;
        m_index_map[to_lower(type)] = idx;
    }
}

const ExporterOptions& CGlobalExporterConfigs::Get(const std::string &fileType) const
{
    IndexMap::const_iterator it = m_index_map.find(to_lower(fileType));
    if (it == m_index_map.end()) {
        throw CTiglError("No Exporter for type '" + fileType + "'", TIGL_NOT_FOUND);
    }
    else {
        size_t idx = it->second;
        return m_configs[idx];
    }
}

ExporterOptions& CGlobalExporterConfigs::Get(const std::string &fileType)
{
     return const_cast<ExporterOptions&>(static_cast<const CGlobalExporterConfigs*>(this)->Get(fileType));
}

CGlobalExporterConfigs::CGlobalExporterConfigs()
{
}

} // namespace tigl
