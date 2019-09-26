API Usage {#lib_usage}
===========


In order to use the TiGL library in a C program, you have to link against the TiGL as well as the TiXI library. 
The TiXI (TiXI Xml Interface) is used to read in the XML-based CPACS files. It can be obtained from the project site
http://github.com/DLR-SC/tixi.
In addition to the libraries itself, one needs to include the corresponding header files:

    #include "tigl.h" 
    #include "tixi.h"

For example, to open a CPACS configuration, one can use

    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    tixiOpenDocument("aFileName", &tixiHandle);
    tiglOpenCPACSConfiguration(tixiHandle, "aConfigurationUID", &tiglHandle);

For more details and for the usage from Python or Matlab, see the examples in the

    TIGL_INSTALL_DIR/share/doc/tigl3/examples

directory.

