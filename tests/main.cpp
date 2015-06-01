/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2012-10-17 Markus Litz <Markus.Litz@dlr.de>
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

#include "gtest/gtest.h"
#include "CTiglLogging.h"
#include <tixi.h>

#ifdef HAVE_VLD
#include <vld.h>
#endif

// make tixi quiet
void tixiSilentMessage(MessageType , const char *, ...){}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    tigl::CTiglLogging::Instance().SetTimeIdInFilenameEnabled(false);
    // disable any console logging
    tigl::CTiglLogging::Instance().SetConsoleVerbosity(TILOG_SILENT);
    tigl::CTiglLogging::Instance().LogToFile("tigltest");
    // disable tixi output
    tixiSetPrintMsgFunc(tixiSilentMessage);
    int retval = RUN_ALL_TESTS();
    tixiCleanup();
    return retval;
}
