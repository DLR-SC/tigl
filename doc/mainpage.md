Overview{#mainpage}
==================

The TiGL Geometry Library can be used for easy processing of geometric data stored inside CPACS data sets. 
TiGL offers query functions for the geometry structure. 
These functions can be used for example to detect how many segments are attached to a certain segment, 
which indices these segments have, or how many wings and fuselages the current airplane configuration contains. 
This functionality is necessary because not only the modeling of simple wings or fuselages but also the 
description of quite complicated structures with branches or flaps is targeted. 
The developed library uses the Open Source software OpenCASCADE to represent the airplane geometry by 
B-spline surfaces in order to compute surface points and also to export the geometry in the IGES/STEP/STL/VTK format. 
The library provides external interfaces for C/C++, Python, MATLAB and FORTRAN. 

@section sec1 What is TIGL

In order to perform the modeling of wings and fuselages as well as the computation of surface points effectively, a
geometry library was developed in C++. The library provides external interfaces
for C and FORTRAN. Some of the requirements of the library were:

 - Ability to read and process the information stored in a CPACS file for
 wings and fuselages,
 - Possibility to extend to engine pods, landing gear and other
 geometrical characteristics, 
 - Ability to build up the three-dimensional airplane geometry for further
 processing,
 - Ability to compute surface points in Cartesian coordinates by using
   common aircraft parameters,
 - Possibility to be expanded by additional functions such as area or volume
 computations,
 - Possibility to export the airplane geometry in the IGES format.

The developed library uses the Open Source software OpenCASCADE to represent the airplane geometry by B-spline surfaces
in order to compute surface points and also to export the geometry in the IGES format.
OpenCASCADE is a development platform written in C++ for CAD, CAM, and CAE
applications which has been continuously developed for more than ten years. 
The functionality covers geometrical primitives (for example points,
vectors, matrix operations), the computation of B-spline surfaces and boolean operations on volume models.

Apart from the already specified requirements above, the geometry library 
offers query functions for the geometry structure. These functions can be used
for example to detect how many segments are attached to a certain segment,
which indices these segments have, or how many wings and fuselages the current
airplane configuration contains. This functionality is necessary because not
only the modeling of simple wings or fuselages but also the description of quite complicated
structures with branches or flaps is targeted.


@section sec2 TiGLViewer

In order to review the geometry information of the central data set a visualization
tool, TIGLViewer, was developed. The TIGLViewer allows the visualization of the used airfoils and
fuselage profiles as well as of the surfaces and the entire airplane model.
Furthermore, the TIGLViewer can be used to validate and test the implemented
functions of the geometry library, for example the calculation of points on the
surface or other functions to check data that belong to the geometry structure.

@page tigl_usage Usage
@page tigl_examples Examples
