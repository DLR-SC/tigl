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

TiGL is a CMake project, so in simple terms, TiGL can be configured and built via

    mkdir build && cd build
    cmake .. 
    cmake --build .

The minimum requirements to build TiGL are a C++17 compliant compiler and CMake, TiXI and OpenCascade Technology (OCCT). Qt5 is needed if you want to build the TiGLCreator.

All build dependencies of TiGL are available as conda packages. Most dependencies are supplied in a decicated channel at [https://anaconda.org/dlr-sc/](https://anaconda.org/dlr-sc/). 
Specifically, this channel contains a recommended variant of opencascade, which includes a patch for G2-continuous Coons patches.
The recipes for the conda packages in the dlr-sc channel can be found at [https://github.com/DLR-SC/tigl-conda](https://github.com/DLR-SC/tigl-conda).

@subsection pixi Using the Pixi package manager

We recommend installing TiGL's dependencies and configuring TiGL using the [pixi](https://pixi.sh/latest/) package manager. In the root directory of our repository is a `pixi.toml` file that defines several environments and tasks. 

A pixi *environment* corresponds to a typical build configuration frequently used by us, the TiGL developers or as part of our continuous integration pipeline. Each environment and platform comes with a specific set of dependencies.

A *task* corresponds to a typical step, such as configuring the build with CMake, building or installing TiGL, executing the tests etc.

Enter `pixi info` for a complete list of environments and tasks.

For example,    

    pixi run configure

will install TiGL's dependencies, create a build directory and run cmake with a default configuration using ninja as a generator. Now you can navigate to the build directory and modify the initial cmake configuration if you like.

    pixi run -e default install

Will build and install TiGL using cmake and ninja. This will be done using the task definition of our `default` environment using the default dependencies.

    pixi run tests

will invoke unit tests and integration tests

    pixi run tiglcreator

will start the TiGLCreator from the install directory.

To use another environment than `default`, we need to invoke the command like this:

    pixi run -e python-internal configure
    pixi run install
    pixi run unittests

 This command configures TiGL for a Release build with internal python bindings, install TiGL in a subdirectory of the build directory and run the unit tests, but not the integration tests.

The `configure` task has additional arguments. For instance

    pixi run -e occt-static configure Debug

will configure a Debug build of TiGL that links in OCCT statically.

@subsection codegenerator Code Generation

TiGL uses a code generator to automatically generate C++ classes from the xml schema definition of CPACS in `cpacs_gen_input/cpacs_schema.xsd`. These generated classes are stored in `src/generated` and can be customized by hand-written code. Whenever a new CPACS node is added or the underlying CPACS schema of TIGL changes, the code generator must be triggered for the new schema. 

The code generator is includes as a git submodule to this repository. For convenience, there is a pixi task to generate the code

    pixi run generate

will update the git submodule, build the code generator and invoke the code generator on the input files in the directory `cpacs_gen_input/`.

@subsection cmakeoptions CMake Options

Here is a complete list of TiGL's CMake options.

| Option | Description | Default Value |
| -- | -- | -- |
| TIGL_CREATOR | Builds the TiGLCreator program (requires OpenGL and Qt) | ON |
| TIGL_BUILD_TESTS | Build TiGL Test suite | OFF |
| TIGL_BINDINGS_PYTHON | Builds the python bindings of TiGL's C API (requires python > 2.5) | ON |
| TIGL_BINDINGS_PYTHON_INTERNAL | Build the python interface to the internal API (requires swig) | OFF |
| TIGL_BINDINGS_JAVA | Build the java bindings of TiGL (requires Java) | OFF |
| TIGL_BINDINGS_MATLAB | Build the Matlab bindings of TiGL (requires matlab and python) | OFF |
| TIGL_BINDINGS_INSTALL_CPP | Install TiGL's C++ bindings | OFF |
| TIGL_NIGHTLY | Create a nightly build of TIGL (includes git sha into tigl version) | OFF |
| TIGL_CONCAT_GENERERATED_FILES | Concatenate all generated files into one. This speeds up compilation, but gives undesirable line numbers in error messages in releases | ON |
| TIGL_USE_GLOG | Enables advanced logging (requires google glog) | OFF |
| TIGL_DOC_PDF | Build TiGLs documentation using Lates | OFF |
| TIGL_ENABLE_COVERAGE | Enable GCov coverage analysis (defines a 'coverage' target and enforces static build of tigl) | OFF |
| TIGL_COVERAGE_GENHTML | Use Genhtml to generate htmls fromg gcov output | OFF |
| DEBUG_EXTENSIVE | Swith on extensive debug output | OFF |
| TIGL_USE_VLD | Enable Visual Leak Detector | OFF |
| OpenCASCADE_DONT_SEARCH_OCE | Disabled searching for OCE | OFF |
| OCE_STATIC_LIBS | Should be checked, if static OCE libs are linked | OFF |
| OpenCASCADE_STATIC_LIBS | Should be checked, if static OpenCASCADE libs are linked | OFF |