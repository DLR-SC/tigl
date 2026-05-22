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