Installation {#tigl_installation}
===========

##Binary Download##

The standard way of using pre-packaged binaries is to download them from TiGL's release page https://github.com/DLR-SC/tigl/releases.
Here, we offer packages for Windows and macOS (Darwin).

### Linux ###
There are two ways to install TiGL on Linux:
 - Use the package manager of your distribution. Just go to https://software.opensuse.org/download.html?project=science:dlr&package=tigl3, choose 
   your linux distribution and follow the instructions.
 - If you can't install system wide packages, we provide a helper script https://raw.githubusercontent.com/DLR-SC/tigl/cpacs_3/misc/get_tigl_tixi.sh that downloads and decompresses the required packages.

##Python##

The easiest way to install TiGL and all its dependencies for Python is using Conda. Conda is a package manager
for Python packages and allows the distribution of pre-compiled packages.

To install TiGL into a separate environement, enter the following from the conda command prompt

    conda create -n cpacscreator_env python=3.6 cpacscreator -c dlr-sc

All TiGL related packages are now found inside the tigl_env environment. To enter this environment, type

    activate cpacscreator_env

Have a look to our examples at https://github.com/DLR-SC/tigl/tree/cpacs_3/examples/python on how to use the Python bindings.

##Matlab##

The TiGL binary distribution includes interfaces to the MATLAB language. On Windows systems, we ship
with a precompiled MEX file and script files that can be found under share/tigl3/matlab.

On Linux, we cannot provide precompiled binaries of the interface. Instead you can find all
script files and the MEX input file tiglmatlab.c under share/tigl3/matlab. To compile the
MATLAB bindings, the tool "mex" is required, which is typically part of each MATLAB installation.
To compile, use our Makefile by typing in the command "make".

Our small Matlab demo at https://github.com/DLR-SC/tigl/tree/cpacs_3/examples/matlab_demo demonstrates, how to use TiGL's Matlab bindings.