#include "CTiglLogSplitter.h"

namespace tigl {

CTiglLogSplitter::CTiglLogSplitter()
{
}

CTiglLogSplitter::~CTiglLogSplitter()
{
    Loggerlist::iterator it;
    for(it = _loggers.begin(); it != _loggers.end(); ++it) {
        ITiglLogger* logger = it->logger;
        if(logger){
            delete logger;
        }
    }
}


void CTiglLogSplitter::AddLogger(ITiglLogger* logger, TiglLogLevel maxlevel) {
    if(logger) {
        _loggers.push_back(LoggerEntry(logger, maxlevel));
    }
}

// override from ITIglLogger
void CTiglLogSplitter::LogMessage(TiglLogLevel level, const char * message) {
    Loggerlist::iterator it;
    for(it = _loggers.begin(); it != _loggers.end(); ++it) {
        if(!it->logger)
            continue;

        if(level <= it->level) {
            it->logger->LogMessage(level, message);
        }
    }
}

} // namespace tigl

