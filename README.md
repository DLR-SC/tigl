<p><img src="doc/images/logo-readme.png" alt="TiGL Logo" title="TiGL Logo" width="167"/></p>

[![CI workflow](https://github.com/DLR-SC/tigl/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/DLR-SC/tigl/actions/workflows/ci-workflow.yml)
[![codecov](https://codecov.io/gh/dlr-sc/tigl/branch/main/graph/badge.svg)](https://codecov.io/gh/dlr-sc/tigl)
[![License: Apache 2.0](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](https://github.com/DLR-SC/tigl/blob/main/LICENSE.txt)
[![Install with conda](https://anaconda.org/dlr-sc/tigl3/badges/version.svg)](https://anaconda.org/dlr-sc/tigl3)
[![Cite-us](https://img.shields.io/badge/doi-10.1007%2Fs11786--019--00401--y-blue)](https://doi.org/10.1007/s11786-019-00401-y) 
[![Documentation](https://img.shields.io/badge/docs-online-green)](https://dlr-sc.github.io/tigl/doc/latest/) 

The **Ti**GL **G**eometry **L**ibrary can be used for the computation and processing of aircraft geometries 
stored inside [CPACS](https://github.com/DLR-LY/CPACS) files. TiGL offers many geometry related functions such as
 - Point retrieval functions to compute points on the aircraft surface
 - Intersection functions to compute the intersection of the aircraft with planes
 - Export functions for standard CAD file formats (STEP + IGES) or mesh formats, 
   including VTK, Collada, and STL.
   
The TiGL library uses the OpenCASCADE CAD kernel to represent the airplane geometry 
by NURBS surfaces. The library provides external interfaces for C, C++, Python, Java, MATLAB, and FORTRAN.

TiGL is shipped with the Qt based _TiGLCreator_ for visualizing and editing aircraft
geometries or viewing CAD files.

![Screenshot of the TiGLCreator](doc/images/tiglcreator.png)

# Installation

Install TiGL (library, Python bindings, TiGLCreator) with conda:

    conda install -c dlr-sc tigl3

Alternatively, download installers for Windows and macOS from the
[latest release](https://github.com/DLR-SC/tigl/releases/latest).

## Building from source

With [pixi](https://pixi.sh) (installs all dependencies):

    git clone --recursive https://github.com/DLR-SC/tigl.git
    cd tigl
    pixi run configure
    pixi run install

Or with plain CMake (requires a C++17 compiler, TiXI, OpenCASCADE and, for TiGLCreator, Qt5):

    cmake -S . -B build
    cmake --build build --target install

See the [installation guide](https://dlr-sc.github.io/tigl/doc/latest/tigl_installation.html)
for more options and details.

# News

Please head over to our TiGL website: https://dlr-sc.github.io/tigl/#news

# Cite us

TiGL is available as Open Source and we encourage anyone to make use of it. If you are applying TiGL in a scientific environment and publish any related work, please cite the following article:

Siggel, M., Kleinert, J., Stollenwerk, T. et al.:  *TiGL: An Open Source Computational Geometry Library for Parametric Aircraft Design*, Math.Comput.Sci. (2019). https://doi.org/10.1007/s11786-019-00401-y

A free copy of the paper is offered here: https://rdcu.be/bIGUH 

# License

TiGL is licensed under the [Apache License 2.0](LICENSE.txt).
