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

All build dependencies of TiGL are available as conda packages, mostly from the [conda-forge](https://conda-forge.org/) channel. 
TiGL no longer requires a patched variant of OpenCASCADE; the stock conda-forge `occt` package is used directly.

@subsection pixi Using the Pixi package manager

We recommend installing TiGL's dependencies and configuring TiGL using the [pixi](https://pixi.sh/latest/) package manager. In the root directory of our repository, there is a `pixi.toml` file that defines several environments and tasks. 

A pixi *environment* corresponds to a typical build configuration frequently used by us, the TiGL developers, or as part of our continuous integration pipeline. Each environment and platform comes with a specific set of dependencies.

A *task* corresponds to a typical step, such as configuring the build with CMake, building or installing TiGL, executing the tests etc.

Enter `pixi info` for a complete list of environments and tasks.

For example,    

    pixi run -e default configure

will install TiGL's dependencies, create a build directory and run cmake with a default configuration using ninja as a generator. Now you can navigate to the build directory and modify the initial cmake configuration if you like.

    pixi run -e default install

Will build and install TiGL using cmake and ninja. This will be done using the task definition of our `default` environment using the default dependencies.

    pixi run tests

will invoke unit tests and integration tests

    pixi run -e default tiglcreator

will start the TiGLCreator from the install directory.

To build and run tests, use the `default` environment:

    pixi run -e default configure
    pixi run -e default install
    pixi run -e default unittests

The `configure` task has additional arguments. For instance

    pixi run -e default configure Debug

will configure a Debug build of TiGL.

@subsection internalpython Internal Python bindings

To build and install the internal Python bindings, run

    pixi run -e python-internal configure
    pixi run install

These commands will configure TiGL to be built with internal python bindings enabled and it will install tigl and its python bindings directly into the pixi environment in `.pixi/envs/python-internal/`. You can activate this environment using `pixi shell -e python-internal`

     pixi shell -e python-internal
     python my_test_script.py
     exit

or you could run python directly as a task in one command

    pixi r -e python-internal python my_test_script.py

@subsection codegenerator Code Generation

TiGL uses a code generator to automatically generate C++ classes from the xml schema definition of CPACS in `cpacs_gen_input/cpacs_schema.xsd`. These generated classes are stored in `src/generated` and can be customized by hand-written code. Whenever a new CPACS node is added or the underlying CPACS schema of TIGL changes, the code generator must be triggered for the new schema. 

The code generator is included as a git submodule to this repository. For convenience, there is a pixi task to generate the code

    pixi run generate

will update the git submodule, build the code generator and invoke the code generator on the input files in the directory `cpacs_gen_input/`.

@subsection cpacsstylechecker Check the CPACS schema on style and syntax

After a change on the `cpacs_schema.xsd`, users can execute the external `cpacs-schema-tool` to check the schema on correctness and style. Since it is also used by the CPACS maintainers, this can avoid overhead when merging the changes coming from TiGL into the main CPACS repository. There are a few commands to solve different tasks:

    pixi run test-schema

verifies canonical formatting and XSD compilation.

    pixi run lint-schema

checks CPACS conventions, references, reachability, prefixes, and XSD validity.

    pixi run format-schema

creates a new file based on the current schema that normalizes ordering, attributes, whitespace, and redundant occurrence defaults. Before application, a copy of the current schema called `cpacs_schema.xsd.backup` is made.

    pixi run check-schema

combines the both calls of `pixi run test-schema` and `pixi run lint-schema`.

@subsection thirdpartysources Vendored third-party sources

Two dependencies that aren't (fully) available as conda-forge packages are vendored directly in the repository:

- `thirdparty/pythonocc-core` (git submodule): the pythonocc-core SWIG interface files used by TiGL's
  internal Python bindings (`TIGL_BINDINGS_PYTHON_INTERNAL`) to reuse OCCT type wrappers. The
  conda-forge `pythonocc-core` package only ships the compiled `OCC` Python module, not these sources.
  The pixi `generate` and `python-internal configure` tasks initialize this submodule automatically.
  When building without pixi, run `git submodule update --init --recursive` before configuring.
  **Important**: keep the submodule's pinned tag in sync with the `pythonocc-core` and `occt` versions
  in `pixi.toml` (see the pinned version comments there). SWIG's cross-module runtime type sharing
  breaks silently across large version gaps.
- `thirdparty/matlab-sdk/{win-64,osx-64}`: MATLAB's `extern/include` headers and `mex`/`mx`/`mat`
  import-stub libraries needed to build the MATLAB (MEX) bindings (`TIGL_BINDINGS_MATLAB`) without a
  full MATLAB installation. No conda-forge equivalent exists. Not needed on Linux, where MATLAB itself
  provides `mex`/`make` for building the bindings against a real local installation (see @ref matlab).
  `cmake/FindMATLAB.cmake` uses these automatically as a fallback when `MATLAB_DIR`/`MATLABDIR` aren't
  set to a real MATLAB installation. Only an Intel (`osx-64`) SDK is vendored — on Apple Silicon
  (`osx-arm64`) the MATLAB bindings are skipped with a CMake warning unless you point `MATLAB_DIR`
  at your own Apple Silicon MATLAB installation.

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
