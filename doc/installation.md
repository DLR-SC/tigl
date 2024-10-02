Installation {#tigl_installation}
===========

##Binary Download##

The standard way of using pre-packaged binaries is to download them from TiGL's release page https://github.com/DLR-SC/tigl/releases.
Here, we offer packages for Windows and macOS (Darwin).

### Linux ###
There are two ways to install TiGL on Linux:
 - Use the package manager of your distribution. Just go to [https://software.opensuse.org/download.html?project=science:dlr&package=tigl3](https://software.opensuse.org/download.html?project=science:dlr&package=tigl3), choose 
   your linux distribution and follow the instructions.
 - If you can't install system wide packages, we provide a helper script [https://github.com/DLR-SC/tigl/blob/master/misc/get_tigl_tixi.sh](https://github.com/DLR-SC/tigl/blob/master/misc/get_tigl_tixi.sh) that downloads and decompresses the required packages.

##Python##

The easiest way to install TiGL and all its dependencies for Python is using Conda. Conda is a package manager
for Python packages and allows the distribution of pre-compiled packages.

To install TiGL into a separate environement, enter the following from the conda command prompt

    conda create -n cpacscreator_env python=3.6 cpacscreator -c dlr-sc

All TiGL related packages are now found inside the tigl_env environment. To enter this environment, type

    conda activate cpacscreator_env

Have a look to our examples at [https://github.com/DLR-SC/tigl-examples](https://github.com/DLR-SC/tigl-examples) on how to use the Python bindings.

##Matlab##

The TiGL binary distribution includes interfaces to the MATLAB language. On Windows systems, we ship
with a precompiled MEX file and script files that can be found under share/tigl3/matlab.

On Linux, we cannot provide precompiled binaries of the interface. Instead you can find all
script files and the MEX input file tiglmatlab.c under share/tigl3/matlab. To compile the
MATLAB bindings, the tool "mex" is required, which is typically part of each MATLAB installation.
To compile, use our Makefile by typing in the command "make".

Our small Matlab demo at [https://github.com/DLR-SC/tigl/tree/master/examples/matlab_demo](https://github.com/DLR-SC/tigl/tree/master/examples/matlab_demo) demonstrates, how to use TiGL's Matlab bindings.

## Building from source ##

The minimum requirements to build TiGL are TiXI and OpenCascade Technology (OCCT). Qt5 is needed if you want to build the tiglviewer.
All build dependencies of TiGL are available as conda packages. Most dependencies are supplied in a decicated channel at [https://anaconda.org/dlr-sc/](https://anaconda.org/dlr-sc/). 
Specifically, this channel contains a recommended variant of opencascade, which includes a patch for G2-continuous Coons patches.
The recipes for the conda packages in the dlr-sc channel can be found at [https://github.com/DLR-SC/tigl-conda](https://github.com/DLR-SC/tigl-conda).

We recommend installing TiGL's dependencies via conda in a seperate environment. You can find our recommended environment file for the latest development version at [https://github.com/DLR-SC/tigl/tree/master/environment.yml](https://github.com/DLR-SC/tigl/tree/master/environment.yml). To create this environment locally, enter the following command from the conda command prompt:

    conda env create -f environment.yml

This command will install a new environment called tigl-bld with all dependencies pre-installed. Next activate the environment:

    conda activate tigl-bld

Now tigl can be configured and build using cmake:

    mkdir build
    cd build
    cmake ..
    cmake --build .

This will configure TiGL with default options. An example of a customized cmake configuration could look like this:

    cmake .. -GNinja \
             -DCMAKE_BUILD_TYPE=Release \
             -DCMAKE_INSTALL_PREFIX=install \
             -DOCE_STATIC_LIBS=ON \
             -DTIGL_BUILD_TESTS=ON \
             -DTIGL_ENABLE_COVERAGE=ON \
             -DTIGL_CONCAT_GENERATED_FILES=ON \
             -DTIGL_BINDINGS_PYTHON_INTERNAL=ON \
             -DTIGL_BINDINGS_MATLAB=ON \
             -DTIGL_BINDINGS_JAVA=ON \
             -DTIGL_NIGHTLY=ON

You can also take inspiration from [here](https://github.com/DLR-SC/tigl/tree/master/.github/actions) to see how TiGL releases are build in our continuous integration pipeline.
