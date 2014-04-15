Description {#mainpage}
===========

The TiGL Geometry Library can be used for easy processing of geometric data stored inside CPACS data sets. 
TiGL offers query functions for the geometry structure. 
These functions can be used for example to detect how many segments are attached to a certain segment, 
which indices these segments have, or how many wings and fuselages the current airplane configuration contains. 
This functionality is necessary because not only the modeling of simple wings or fuselages but also the 
description of quite complicated structures with branches or flaps is targeted. 
The developed library uses the Open Source software OpenCASCADE to represent the airplane geometry by 
B-spline surfaces in order to compute surface points and also to export the geometry in the IGES/STEP/STL/VTK format. 
The library provides external interfaces for C/C++, Python, MATLAB and FORTRAN. 