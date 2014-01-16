#include "CTiglLogSplitter.h"

namespace tigl {

CTiglLogSplitter::CTiglLogSplitter() : verbosity(TILOG_DEBUG4)
{
}

CTiglLogSplitter::~CTiglLogSplitter()
{
}


void CTiglLogSplitter::AddLogger(PTiglLogger logger) 
{
    if(logger) 
    {
        _loggers.push_back(logger);
    }
}

// override from ITIglLogger
void CTiglLogSplitter::LogMessage(TiglLogLevel level, const char * message) 
{
    if (level<=verbosity)
    {
        for(std::vector<PTiglLogger>::iterator it = _loggers.begin(); it != _loggers.end(); ++it) 
        {
            if(!*it)
            {
                continue;
            }
            (*it)->LogMessage(level, message);
        }
    }
}

void CTiglLogSplitter::SetVerbosity(TiglLogLevel vlevel)
{
    verbosity=vlevel;
}

} // namespace tigl

