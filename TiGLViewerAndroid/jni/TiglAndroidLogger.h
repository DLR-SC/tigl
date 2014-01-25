/*
 * TiglAndroidLogger.h
 *
 *  Created on: 25.01.2014
 *      Author: martin
 */

#ifndef TIGLTOANDROIDLOGADAPTER_H_
#define TIGLTOANDROIDLOGADAPTER_H_

#include "ITiglLogger.h"

#include <string>

class TiglAndroidLogger : public tigl::ITiglLogger
{
public:
    virtual void LogMessage(TiglLogLevel, const char * message) ;
    virtual void SetVerbosity(TiglLogLevel) ;

    void SetTag(const char* tag);

private:
    std::string tag;
};

#endif /* TIGLTOANDROIDLOGADAPTER_H_ */
