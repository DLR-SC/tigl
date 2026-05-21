#include "tiglexternalfilehelpers.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

#include <string>
#include <cstddef>

std::string ResolveFilePath(const std::string& baseFilePath, const std::string& filePath)
{
    if (IsPathRelative(filePath)) {
        size_t pos = baseFilePath.find_last_of("/\\");
        if (pos == std::string::npos) {
            return filePath;
        }
        std::string dirPath = baseFilePath.substr(0, pos);
        return dirPath + "/" + filePath;
    }
    else {
        return filePath;
    }
}

std::string ResolveReadableFilePath(const std::string& baseFilePath, const std::string& filePath)
{
    std::string resolved = ResolveFilePath(baseFilePath, filePath);
    if (!IsFileReadable(resolved)) {
        throw tigl::CTiglError("File " + resolved + " can not be read!", TIGL_OPEN_FAILED);
    }
    return resolved;
}