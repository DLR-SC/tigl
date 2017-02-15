/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* Created: 2015-05-08 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CTiglImporterFactory.h"

#include <algorithm>

namespace tigl
{

CTiglImporterFactory& CTiglImporterFactory::Instance()
{
    static CTiglImporterFactory factory;
    return factory;
}

CTiglImporterFactory::CTiglImporterFactory()
{
}

void CTiglImporterFactory::RegisterImporter(ICADImporterCreator* importerCreator)
{
    if (importerCreator) {
        // build an importer to get supported file type
        PTiglCADImporter importer = importerCreator->create();
        if (importer) {
            std::string filetype = importer->SupportedFileType();
            _importerBuilders[filetype] = importerCreator;
        }
    }
}

PTiglCADImporter CTiglImporterFactory::Create(const std::string &filetype) const
{
    // make the requested filetype to lowercase
    std::string fileTypeToLower = filetype;
    std::transform(fileTypeToLower.begin(), fileTypeToLower.end(), fileTypeToLower.begin(), ::tolower);

    ImporterMap::const_iterator it = _importerBuilders.find(fileTypeToLower);
    if (it != _importerBuilders.end()) {
        ICADImporterCreator* creator = it->second;
        return creator->create();
    }
    else {
        return PTiglCADImporter();
    }
}

bool CTiglImporterFactory::ImporterSupported(const std::string& filetype) const
{
    // make the requested filetype to lowercase
    std::string fileTypeToLower = filetype;
    std::transform(fileTypeToLower.begin(), fileTypeToLower.end(), fileTypeToLower.begin(), ::tolower);

    ImporterMap::const_iterator it = _importerBuilders.find(fileTypeToLower);
    if (it != _importerBuilders.end()) {
        return true;
    }
    else {
        return false;
    }
}

}


