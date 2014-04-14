TiGL example files
==================

== C-Demo ==

This program can be build using the makefiles shipped with the example.

On Windows, open a visual studio command prompt, go into the 
<TIGL_INSTALL_DIR>/share/doc/tigl/examples directory. Open makefile.msvc in a text editor and
change TIGL_HOME and TIXI_HOME to the appropriate directories. Then enter

  nmake /f makefile.msvc

to build the c-demo.exe.

On Linux, open a shell and go into the directory <TIGL_INSTALL_DIR>/share/doc/tigl/examples directory.
Open makefile.gnu in a text editor and change TIGL_HOME and TIXI_HOME to the appropriate directories.
Then type

  make -f makefile.gnu

to build the c-demo.


To build this example program by yourself without a makefile, one has to link against the following libaries:
 * (lib)TIGL
 * (lib)TIXI

The opencascade libraries should be automatically linked by the TiGL shared library.

To run this program, call e.g.
  c-demo simpletest.cpacs.xml



== Python-Demo==

== MATLAB-Demo==
