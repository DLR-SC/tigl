#include "tiglexternalfilehelpers.h"
#include "tiglcommonfunctions.h"
#include "CTiglError.h"

#include <filesystem>
#include <string>
#include <cstddef>

namespace fs = std::filesystem;

std::string getPathRelativeToApp(const std::string& cpacsDocumentPath,
                                 const std::string& linkedFilePath)
{
    if (IsPathRelative(linkedFilePath)) {
        size_t pos = cpacsDocumentPath.find_last_of("/\\");
        if (pos == std::string::npos) {
            // kein Verzeichnis verfügbar -> Pfad bleibt relativ
            return linkedFilePath;
        }
        std::string dirPath = cpacsDocumentPath.substr(0, pos);
        return dirPath + "/" + linkedFilePath;
    }
    else {
        return linkedFilePath;
    }
}

std::string evaluatePathRelativeToApp(const std::string& cpacsDocumentPath, const std::string& linkedFilePath)
{
    std::string resolved = getPathRelativeToApp(cpacsDocumentPath, linkedFilePath);
    if (!IsFileReadable(resolved)) {
        throw tigl::CTiglError("File " + resolved + " can not be read!", TIGL_OPEN_FAILED);
    }
    return resolved;
}