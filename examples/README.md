Examples {#tigl_examples}
=========================

TiGL ships with three examples of how to use the API from different programming languages: C/C++, Python and MATLAB. 
All examples can be found in the

    TIGL_INSTALL_DIR/share/doc/tigl3/examples

directory.

In order to run a TiGL based project, make sure to add
  - TIGL_INSTALL_DIR/bin and TIXI_INSTALL_DIR/bin to *PATH on Windows*
  - TIGL_INSTALL_DIR/lib(64) and TIXI_INSTALL_DIR/lib(64) to *LD_LIBRARY_PATH on Linux*
  - TIGL_INSTALL_DIR/lib and TIXI_INSTALL_DIR/lib to *DYLD_LIBRARY_PATH on Mac*
	
	
C-Demo
------

This program is given by the file 

    c_demo.c

and an be build using the makefiles shipped with the example.

On *Windows*, open a visual studio command prompt, go into the 

    TIGL_INSTALL_DIR/share/doc/tigl3/examples 

directory. Open makefile.msvc in a text editor and
change TIGL_HOME and TIXI_HOME to the appropriate directories. Then enter

    nmake /f makefile.msvc

to build the c-demo.exe.

On *Linux*, open a shell and go into the directory 

    TIGL_INSTALL_DIR/share/doc/tigl3/examples 

Open makefile.gnu in a text editor and change TIGL_HOME and TIXI_HOME to the appropriate directories.
Then type

    make -f makefile.gnu

to build the c-demo.


To build this example program by yourself without a makefile, one has to link against the following libaries:
  - (lib)tigl3
  - (lib)tixi3

The OpenCASCADE libraries should be automatically linked by the TiGL shared library.

To run this program, call it with a CPACS file, e.g.

    c-demo simpletest.cpacs.xml



Python-Demo
-----------

The Python demo is similar to the C demo. In order to make it work, one needs to add 

    TIGL_INSTALL_DIR/share/tigl3/python
    TIXI_INSTALL_DIR/share/tixi3/python

to the PYTHONPATH environment variable. Afterwards, test it with

    python python_demo.py simpletest.cpacs.xml


MATLAB-Demo
-----------
To run the MATLAB demo, one has to make sure, that the TiGL and TiXI libraries
can be found by setting the path variables as described above. 

The MATLAB bindings can be found in the following directories:

    TIGL_INSTALL_DIR/share/tigl3/matlab
    TIXI_INSTALL_DIR/share/tixi3/matlab

Please add both directories to the MATLAB path using the 'path' command of MATLAB.

The demo can be called as a function by entering

    matlab_demo('simpletest.cpacs.xml')

	
JAVA-Demo
---------
To compile the Java Demo (JavaDemo.java), you have to link against the following libraries
  - commons-logging (Apache Commons logging)
  - jna and jna-platform (Sun Java Native Access)
  - tigl (the JAR file can be found in TIGL_INSTALL_DIR/share/tigl3/java)
	
Under Windows it is important to chose the right TiGL library matching to the Java Virtual Machine. 
That means if you use a 64 Bit JVM, use the TiGL 64 bit library and vice versa.
	
