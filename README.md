![Screenshot of the TiGL Viewer](doc/images/tiglviewer-web.jpg)

# TiGL

![Continuous Integration](https://github.com/dlr-sc/tigl/workflows/Continuous%20Integration/badge.svg)
[![codecov](https://codecov.io/gh/dlr-sc/tigl/branch/master/graph/badge.svg)](https://codecov.io/gh/dlr-sc/tigl)
[![Apache 2.0](https://img.shields.io/crates/l/k)](https://github.com/DLR-SC/tigl/blob/cpacs_3/LICENSE.txt)
[![Install with conda](https://anaconda.org/dlr-sc/tigl/badges/installer/conda.svg)](https://conda.anaconda.org/dlr-sc/tigl3)
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

TiGL is shipped with the Qt based _TiGL Viewer_ for visualizing aircraft
geometries or viewing CAD files.


 - Binary Downloads:  https://github.com/DLR-SC/tigl/wiki/Downloads
 - Nightly Builds:    https://github.com/DLR-SC/tigl/actions?query=workflow%3A%22Continuous+Integration%22+event%3Aschedule
 - Wiki:              https://github.com/DLR-SC/tigl/wiki

# News

Please head over to our TiGL website: https://dlr-sc.github.io/tigl/#news

# Cite us

TiGL is available as Open Source and we encourage anyone to make use of it. If you are applying TiGL in a scientific environment and publish any related work, please cite the following artice:

Siggel, M., Kleinert, J., Stollenwerk, T. et al.:  *TiGL: An Open Source Computational Geometry Library for Parametric Aircraft Design*, Math.Comput.Sci. (2019). https://doi.org/10.1007/s11786-019-00401-y

A free copy of the paper is offered here: https://rdcu.be/bIGUH 


