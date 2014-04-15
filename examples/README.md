Examples
=============

TiGL ships with three examples of how to use the API from different programming languages: C/C++, Python and Matlab. They can be found in the

    <TIGL_INSTALL_DIR>/share/doc/tigl/examples

directory

C-Demo
------

This program is given by the file 

    c_demo.c

and an be build using the makefiles shipped with the example.

On *Windows*, open a visual studio command prompt, go into the 
<TIGL_INSTALL_DIR>/share/doc/tigl/examples directory. Open makefile.msvc in a text editor and
change TIGL_HOME and TIXI_HOME to the appropriate directories. Then enter

    nmake /f makefile.msvc

to build the c-demo.exe.

On *Linux*, open a shell and go into the directory <TIGL_INSTALL_DIR>/share/doc/tigl/examples directory.
Open makefile.gnu in a text editor and change TIGL_HOME and TIXI_HOME to the appropriate directories.
Then type

    make -f makefile.gnu

to build the c-demo.


To build this example program by yourself without a makefile, one has to link against the following libaries:
    - (lib)TIGL
    - (lib)TIXI

The Opencascade libraries should be automatically linked by the TiGL shared library.

To run this program, call e.g.

    c-demo simpletest.cpacs.xml



Python-Demo
-----------

The Python demo is similar to the C demo. In order to make it work, one needs to add 

    <TIGL_INSTALL_DIR>/share/tigl/python
    <TIXI_INSTALL_DIR>/share/tixi/python

to the PYTHONPATH environment variable and

    <TIGL_INSTALL_DIR>/share/tigl/python

to LD_LIBRARY_PATH in *Linux* and PATH in *Windows*, respectively. Afterwards, test it with

    python python_demo.py simpletest.cpacs.xml

MATLAB-Demo
-----------
