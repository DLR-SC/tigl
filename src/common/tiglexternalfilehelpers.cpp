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

#include "tiglexternalfilehelpers.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

#include <string>
#include <cstddef>

std::string ResolveFilePath(const std::string& baseFilePath, const std::string& filePath)
{
    if (IsPathRelative(filePath)) {
        const size_t pos = baseFilePath.find_last_of("/\\");
        if (pos == std::string::npos) {
            return filePath;
        }

        const std::string dirPath = baseFilePath.substr(0, pos);
        return dirPath + "/" + filePath;
    }

    return filePath;
}

void CheckFileIsReadable(const std::string& filePath)
{
    if (!IsFileReadable(filePath)) {
        throw tigl::CTiglError("File " + filePath + " can not be read!", TIGL_OPEN_FAILED);
    }
}