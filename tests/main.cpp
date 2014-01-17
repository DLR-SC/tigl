/*
 * 
 */
//#include "this/package/foo.h"
#include "gtest/gtest.h"
#include "CTiglLogging.h"

#ifdef HAVE_VLD
#include <vld.h>
#endif

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  tigl::CTiglLogging::Instance().SetTimeIdInFilenameEnabled(false);
  // disable any console logging
  tigl::CTiglLogging::Instance().SetConsoleVerbosity(TILOG_SILENT);
  tigl::CTiglLogging::Instance().LogToFile("tigltest");
  return RUN_ALL_TESTS();
}
