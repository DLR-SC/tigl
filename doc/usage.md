Usage
=====

In order to use the TIGL library in a C program, you have to link against the TIGL as well as the TIXI library. Morover one needs to include the corresponding header files:

    #include "tigl.h" 
    #include "tixi.h"

For example, to open a CPACS configuration, one can use

    TixiDocumentHandle tixiHandle;
    TiglCPACSConfigurationHandle tiglHandle;
    tixiOpenDocument("aFileName", &tixiHandle);
    tiglOpenCPACSConfiguration(tixiHandle, "aConfigurationUID", &tiglHandle);

For more details and for the usage from Python or Matlab, see the examples in the

    <TIGL_INSTALL_DIR>/share/doc/tigl/examples

directory.

