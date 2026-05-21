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

#ifndef TIGLEXTERNALFILEHELPERS_H
#define TIGLEXTERNALFILEHELPERS_H

#include <string>
#include "tigl_internal.h"

/**
 * @brief Resolves a file path relative to a base file.
 *
 * Relative paths are interpreted relative to the directory of @p baseFilePath.
 * Absolute paths are returned unchanged.
 *
 * @param baseFilePath Path to the file whose directory is used as base.
 * @param filePath Absolute or relative file path to resolve.
 * @return The resolved file path.
 */
TIGL_EXPORT std::string ResolveFilePath(const std::string& baseFilePath, const std::string& filePath);

/**
 * @brief Resolves a file path and verifies that it is readable.
 *
 * @param baseFilePath Path to the file whose directory is used as base.
 * @param filePath Absolute or relative file path to resolve.
 * @return The resolved readable file path.
 * @throws tigl::CTiglError with TIGL_OPEN_FAILED if the resolved file is not readable.
 */
TIGL_EXPORT std::string ResolveReadableFilePath(const std::string& baseFilePath, const std::string& filePath);

#endif // TIGLEXTERNALFILEHELPERS_H