Installation {#tigl_installation}
===========

@section binary Binary Download

The standard way of using pre-packaged binaries is to download them from TiGL's release page https://github.com/DLR-SC/tigl/releases.
Here, we offer packages for Windows and macOS (Darwin).

@section python Python

The easiest way to install TiGL and all its dependencies for Python is using Conda. Conda is a package manager
for Python packages and allows the distribution of pre-compiled packages.

To install TiGL into a separate environement, enter the following from the conda command prompt

    conda create -n tigl_env tigl3 -c dlr-sc

All TiGL related packages are now found inside the tigl_env environment. To enter this environment, type

    conda activate tigl_env

Have a look to our examples at [https://github.com/DLR-SC/tigl-examples](https://github.com/DLR-SC/tigl-examples) on how to use the Python bindings.

@section matlab Matlab

The TiGL binary distribution includes interfaces to the MATLAB language. On Windows systems, we ship
with a precompiled MEX file and script files that can be found under share/tigl3/matlab.

On Linux, we cannot provide precompiled binaries of the interface. Instead you can find all
script files and the MEX input file tiglmatlab.c under share/tigl3/matlab. To compile the
MATLAB bindings, the tool "mex" is required, which is typically part of each MATLAB installation.
To compile, use our Makefile by typing in the command "make".

Our small Matlab demo at [https://github.com/DLR-SC/tigl/tree/main/examples/matlab_demo](https://github.com/DLR-SC/tigl/tree/main/examples/matlab_demo) demonstrates, how to use TiGL's Matlab bindings.

@section building Building from source

TiGL is a CMake project, so in simple terms, TiGL can be connfigured and built via

    mkdir build && cd build
    cmake .. 
    cmake --build .

The minimum requirements to build TiGL are a C++17 compliant compiler and cmake,  TiXI and OpenCascade Technology (OCCT). Qt5 is needed if you want to build the TiGLCreator.

All build dependencies of TiGL are available as conda packages. Most dependencies are supplied in a decicated channel at [https://anaconda.org/dlr-sc/](https://anaconda.org/dlr-sc/). 
Specifically, this channel contains a recommended variant of opencascade, which includes a patch for G2-continuous Coons patches.
The recipes for the conda packages in the dlr-sc channel can be found at [https://github.com/DLR-SC/tigl-conda](https://github.com/DLR-SC/tigl-conda).

We recommend installing TiGL's dependencies and configuring TiGL using the [pixi](https://pixi.sh/latest/) package manager.

    pixi run configure

will install TiGL's dependencies, create a build directory and run cmake with a default configuration using ninja as a make program. Now you can navigate to the build directory and modify the initial cmake configuration if you like.

For convenience, our pixi project provides some custom tasks for building, testing and running TiGL:

    pixi run install

Will build and install TiGL using cmake and ninja.

    pixi run tests

will invoke unit tests and integration tests

    pixi run tiglcreator

will start the TiGLCreator from the install directory.

For our CI we provide several pixi environments for common build configurations. Currently, these are `default`, `coverage`, `occt-static` and `python-internal`.
Every pixi environment as its specific set of dependencies and modified tasks. As an example,

    pixi run -e python-internal configure
    pixi run install
    pixi run unittests

 configures TiGL for a Release build with internal python bindings, install TiGL in a subdirectory of the build directory and run the unit tests, but not the integration tests.

The `configure` task has additional arguments. For instance

    pixi run -e occt-static configure Debug

Will configure a Debug build of TiGL that links in OCCT statically.