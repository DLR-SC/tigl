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

#ifndef CTIGLIMPORTERFACTORY_H
#define CTIGLIMPORTERFACTORY_H

#include "tigl_internal.h"

#include "ITiglCADImporter.h"
#include "ICADImporterCreator.h"

#include <map>

namespace tigl
{

class CTiglImporterFactory
{
public:
    TIGL_EXPORT static CTiglImporterFactory& Instance();

    /// Registers a CAD Importer at the factory
    TIGL_EXPORT void RegisterImporter(ICADImporterCreator* creator);

    /// Creates an importer for the matching file format
    /// Returns NULL, if the format is unknown
    TIGL_EXPORT ITiglCADImporter* Create(const std::string& filetype) const;
   
    /// Returns true, if an importer was registered for the specified file type
    TIGL_EXPORT bool ImporterSupported(const std::string& filetype) const;

private:
    CTiglImporterFactory();

    typedef std::map<std::string, ICADImporterCreator*>  ImporterMap;
    ImporterMap _importerBuilders;

};

}

#endif // CTIGLIMPORTERFACTORY_H
