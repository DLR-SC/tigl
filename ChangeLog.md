Changelog
=========

Changes since last release
-------------
10/01/2025

- Fixes
  - #936 A particular defined positioning (of the C++-type CCPACSPositioning) was not available via Python bindings since the std::vector<std::unique_ptr<**Type**>> is not exposed to swig. New getter functions have been implemented in CCPACSPositioning.h to make these elements accesible via index, similar to the implementation of for several other classes. For more information see https://github.com/RISCSoftware/cpacs_tigl_gen/issues/59.

  
- General changes:
    - Update the C++ standard to C++17 (#1045).
    - Added the possibility to switch between two algorithms in the `CCSTCurveBuilder`. The default algorithm based on a Chebychev approximation results in a small number of control points, but introduces small discontinuities in the curvature. The new option is to use OCCT's `GeomAPI_PointsToBSpline`, which results in a C3 continuous B-Spline.


13/11/2024

 - Fixes:
   - In #1026, the tolerance for creating cells via contourCoordinate was reduced for cells in spanwise direction. That fixed a bug that caused cells to overlap potentially. The same reduction was missed in chordwise direction and is added now to avoid the cell overlap in chordwise direction, as well. (#1034)
   - Added `CCPACSSkin` to python bindings (#1037).


Version 3.4.0
-------------
12/11/2024

 - General changes:

   - Support CPACS 3.4 (#1027)
   - Implemented standard fuselage profiles, i.e. rounded rectangle profiles (#1005) and superellipses (#1021).
   - CPACS allows prescribing kinks at (fuselage) profiles at given parameters. At the same time, guide curves must be prescribed at a relative circumference of the profile, which does not necessarily coincide with the parametrization of the profiles. In consequence, it was nearly impossible for a user to prescribe a guide curve exactly at a profile kink. This was fixed by allowing guide curves to be placed at a curve parameter that coincides with the prescribed parameter in the profile definition. Now guide curves can optionally be prescribed using `fromParameter ` and `toParameter` instead of `fromRelativeCircumference` and `toRelativeCircumference`. (#1015)
  
 - Fixes:
   - In Debug Mode, the Lofting algorithm now throws an exception, if the generation of the Side Caps fail. The intention behind having the check only in Debug-Mode is that the geometric validity check is very generic and may be too restrictive. In addition, it may be expensive. (#999)
   - Fixed an issue, where approximate reparametrization of a B-Spline Curve resulted in non-negligable geometric changes of the B-Spline, especially if parameters are prescribed for a profile in CPACS. This new reparametrization is incompatible with a previous reparametrization applied to fuselage profiles, that distributed the knots for an overall cleaner geometry. Now the knot distribution is turned off, if parameters are prescribed in CPACS. (#1007)
   - CPACS 3.5 introduces a new way of prescribing the version of the CPACS data set, which means that newer CPACS files could not be interpreted by TiGL anymore. Now, TiGL can open CPACS 3.5 files and raises a warning that the CPACS version is newer than the one supported by TiGL (#1010).
   - Fix hard crash in Wing Geometry generation with cutouts, if there are no cutouts defined in CPACS. (#1023) 

Version 3.3.1
-------------
14/03/2024

 - General changes:

   - Improve documentation and update installation instructions.
   - TiGL now supports Opencascade Technology 7.6.2 (#973).
   - Extension of the functionality of `::tiglwinginterpolatexsi` to work on a single segment that is not part of a component segment (#970). The function
   is now generalized, so that the start and end uid can be either a segment or component segment and the two do not have to be related to each other.
   - TiGL now supplies an environment.yml file that can be used with conda to install all build requirements of tigl into an Anaconda environment.
   - Renamed the function `::CCPACSConfiguration::GetAirplaneLenth` to `::CCPACSConfiguration::GetAirplaneLength` (#992).
   
 - Fixes:

   - Fix bug in guide curve transformation. The rX-Direction was wrongly interpreted with respect to global coordinates, not wing coordinates (#975).
   - Fix for hard crash if guiceCurves node is present but empty in wing segment (#962).
   - Fix bug related to the calculation of Euler angles from a given rotation matrix (#870).

 - TiGLViewer:

   - Improved visual quality of zebra stripes (#974).


Version 3.3.0
-------------
02/05/2023

 - General changes:

   - Support CPACS 3.3 (#883). The following CPACS 3.3 functionality is not yet supported: stringerFramePosition definition at fuselage section (#915), landing gears (#908), parametric fuselage profiles (#909). 
   - Implemented the upcoming CPACS 3.5 feature for modeling ducts. It allows the user to create duct cutouts in        wings and fuselages. TiGL was extended to support ducts and duct assemblies for aircraft configurations. It is      to mention, that currently wings and fuselages are the only components that enable duct cutouts (#881).
   - Generate doxygen documentation for all classes, not just the API functions (#943).
 
 - New API functions:

   - New functions `::tiglConfigurationSetWithDuctCutouts` and `::tiglConfigurationGetWithDuctCutouts` that set and      get the flag whether the ducts defined in CPACS shall be removed from the configuration using Boolean operations. 
   - New function #::tiglGetCrossSectionArea' that allows cutting either a whole configuration or single parts with a plane and calculating the cross section area. This functionality can be used for the application for Whitcomb's area rule (#763). 
 
 - Fixes:

   - Adapt get_tigl_tixi.sh to properly download the tixi3 package for debian (#884).
   - Export fuselages and wings as a whole and not segmentwise in the CAD export of the unfused configuration (#897).
   - Fix typos (#923, #924, #925).
   - Fix bug concerning the conversion of the internal parametrization of the faces of a wing to contour coordinates (#930).
   - Update CI (#918, #839).
   - Fix a compilation error with gcc 12.2 (#948).
   - Fix bug in `::tiglWingGetSegmentUpperSurfaceAreaTrimmed` (#961).

 - TiGLViewer:

   - Implement high dpi screen support (#953).

Version 3.2.3
-------------
09/03/2022

- Fixes:

  - Disabled check, if wing cells can be trimmed rather than cut, even if they are not defined
    by contour coordinates. This check was put in place for better performance and robustness, 
    because with trimmed curves expensive and error-prone boolean operations can be avoided.
    However, the current implementation of the check relies on some hard to tune tolerances and
    led to false positives and thus faulty wing cell geometries (issues #865, #869).
  - Fixed a bug, where the trailing edge of a wing cell could not be defined by referencing a
    spar (issue #864).
  - The class `::CCPACSEnginePosition` has been added to the swig interface (issue #858). Without 
    this it was previously impossible to access the transformed engine nacelle geometrie from Python.

Version 3.2.2
-------------
17/11/2021

- Highlights / General changes:

  - Added support for canard style flaps. This allows to position the hinge points
    outside of the wing by providing an additional translation vector (issue #816).

- Fixes:

  - Fixed CPACS versions header not correctly parsed, if the version number contained a patch version (issue #830).
  - Fixed multiple errors that occured during the computation of wing cells (issues #815, #829, #840).
  - Fixed computation of the wing aspect ratio (issue #827).
  - Fixed crashes in TiGL Viewer when displaying control surface devices (issue #851).
  - Improved robustness of export functions. The export now tries to export as much as possible,
    even if one component failed to built (issue #853).

- Bindings:
  - Added python 3.9 conda packages 
  - Fixed loading python bindings with python 3.8 due to a changed DLL loading policy (issue #842).
  - Fixed a crash in the python bindings due to wrong memory handling (issue #823).

Version 3.2.1
-------------
01/07/2021

- Fixes:

  - Fixed function `::tiglWingGetSegmentEtaXsi` ignoring the GetPointBehaviour that was introduced in TIGL 3.1.0: Since TiGL 3.0.0, the getPoint functions
    now use the parametrization of the wing surface geometries instead . This behaviour can be switched back to the old bevahiour using `::tiglWingSetGetPointBehavior`.
    Before fixing now `::tiglWingGetSegmentEtaXsi`, the functions always used the old behaviour such that the back and fort computation etaXsi -> point -> etaXsi
    did not result in the same values. 

Version 3.2.0
-------------
28/05/2021

 - Highlights / General changes:
 
   - Full support of CPACS 3.2
   - TiGL 3.2 is shipped with OpenCASCADE 7.4.0 and is now compatible with pythonOCC 7.4.0. This includes all TiGL releases and Conda packages.
   - Improved computation speed of wing cell geometries.
   - Implemented cell definition with contour coordinates. This feature will be part of CPACS 3.4.
   - A new curve / curve intersection algorithm. This is used e.g. for the computation of curve network interpolation and was necessary due to a regression in OpenCASCADE 7.4.0. The new algorithm is designed to find all intersections between two curves within a user defined tolerance.
   
   

 - New API functions:
  
   - New function `::tiglWingInterpolateXsi`, that generalizes the functions `::tiglWingComponentSegmentGetSegmentIntersection` and `::tiglWingComponentSegmentComputeEtaIntersection`. Both old functions are now deprecated.

 - Fixes:
  
  - Fixed fuselage walls transformation that did not respect the fuselage transformation before.
  - Fixed fuselage profiles, that had different positions than y=0 at start or end point.
  - Fixed crash, if first guide curve point is (0,0,0) or last point is (0,1,0).
  - Fixed wrong position of trailing edge device borders, if referenceUID is referring to a segment.
  - Fixed a bug in `::tiglWingGetUpperPoint` and `::tiglWingGetLowerPoint`, if the leading edge has zero size.

 
 - Python Bindings:
   
   - The tigl3 Conda packages depend not on pythonOCC-core 7.4.0. If you plan to migrate your code, please notice that pythonOCC and TiGL don't use OpenCASCADE handles anymore. For more insights, please see here: https://github.com/tpaviot/pythonocc-core/pull/583
 

Version 3.1.0
-------------
27/07/2020

 - General changes:

   - Full support of CPACS 3.1
   - Backport support for control surface devices from TiGL 2. It is now possible to manipulate trailing edge devices
     with TiGL 3.1 using the TiGL Viewer, the API or the language bindings.
   - Added support for fuselage walls
   - Support for fuselage profiles with kinks.
   - Added support for new symmetry flags `inherit` and `none`. The symmetry flag `inherit` causes a geometric
     component to inherit the symmetry flag from its parent geometry. This was and still is the default behavior in
     TiGL. The new symmetry type `none` allows the addition of components to mirrored geometries without respecting the
     symmetry of the parent.
   - Substantially improve performance of `::tiglCheckPointInside`.
   - Reduce complexity of the fuselage geometry: The profiles of the fuselage are reparametrized to create simpler a
     B-spline surface. This will improve robustness of boolean operations and increase the general performance at the
     cost of a small error in the geometry creation, that should be negligable in most cases.

 - New API functions:

   - Added new api function ``::tiglComponentGetType``
   - For control devices, the `deflection` value has been renamed to `control_parameter`. Therefore the functions
     - `::tiglControlSurfaceGetMinimumDeflection`
     - `::tiglControlSurfaceGetMaximumDeflection`
     - `::tiglControlSurfaceGetDeflection`
     - `::tiglControlSurfaceSetDeflection`
     
     are marked as deprecated. Please use the new functions

     - `::tiglControlSurfaceGetMinimumControlParameter`
     - `::tiglControlSurfaceGetMaximumControlParameter`
     - `::tiglControlSurfaceGetControlParameter`
     - `::tiglControlSurfaceSetControlParameter`
     
     instead.


 - Fixes:

   - Fixes compilation errors with MinGW and VS 2019
   - Fixes compilation errors with qt4
   - Fixes compilation errors with OpenCascade 7.4.0 and OCCT 7.2
   - Several fixes to internal python bindings
   - Several fixes to control devices code.
   - Fixes invalid segment meta data in VTK export
   - fixing problem in ReadCPACS when using rotors (invalidation during read of segments caused error in lazy creation of attached rotor blades)

 - TiGLViewer:

   - Back-ported control surfaces to TiGL Viewer and improved design of flap dialog.
   - Added option to draw face boundaries in TiGl Viewer


Version 3.0.0
-------------
24/09/2019

 - General Changes:

    - CPACS 3 compatibility, including the new component-segment coordinate-system definition.
   The new cpacs definition of the wing structure allows more precise modelling of ribs and spars.
    - Guide curve support for wings and fuselages for high-fidelity surface modeling according to the CPACS 3 defintion.
    - __Note: TiGL 3 does not yet contain the control surface device modeling. This will be included into TiGL 3.1.__
    - New modeling of nacelles.
    - New modeling of pylons.
    - New modeling of fuselage structure elements like doors, beams, frames and pressure bulkheads.
    - Automated creation of CPACS reading and writing routines. This allows much
   better vality checks of the CPACS document.
    - The "GetPoint" function of wings / fuselages operate now on the actual wing/fuselage shape (e.g. in case of guide curves). Before, these function operated
   on the linear loft, which is different from the actual shape. To be backwards compatible, ``::tiglWingGetUpperPoint``, ``::tiglWingGetLowerPoint``, and ``::tiglFuselageGetPoint``  still use the linear loft by default.
   If the actual smooth surface is desired, the new function ``::tiglWingSetGetPointBehavior`` and ``::tiglFuselageSetGetPointBehavior`` must be used to enable smooth lofts.
   __Note: Using the actual smooth wing shape, ``::tiglWingGetUpperPoint`` and ``::tiglWingGetLowerPoint`` do not use the wing chord surface anymore as a reference coordinate system, but relies on the wing surface parametrization.__
   __This change was required, since there would be no other way to retrieve points around the leading edge, that is on front of the trapezoidal chord surface.__
    - Improved speed of ``::tiglFuselageGetPoint`` function. Also, the paramter xsi is now interpreted as the relative
   curve parameter instead of the relative circumference.
    - Improved robustness of fuselage loft computation.
    - The TIGL library was renamed to tigl3. The TIGLViewer was renamed to tiglviewer-3.
    - The windows builds are now using the Visual C++ 2015 Toolchain.
    - TiGL now requires a C++ 11 capable compiler. Minimum required compilers are gcc 4.8 or Visual C++ 2015.


 - New API functions:

   - ``::tiglWingComponentSegmentPointGetEtaXsi`` computes the eta/xsi coordinates of a point on the component segment.
   - ``::tiglIntersectWithPlaneSegment`` computes the intersection of a CPACS shape (e.g. wing) with a plane of finite size.
   - ``::tiglGetCurveIntersection`` to compute the intersection of two curves.
   - ``::tiglGetCurveIntersectionPoint`` to query the intersection point(s) computed by ``::tiglGetCurveIntersection``.
   - ``::tiglGetCurveIntersectionCount`` returns the number of intersection points computed by ``::tiglGetCurveIntersection``.
   - ``::tiglGetCurveParameter`` projects a point onto a curve and returns the curve parameter of the point.
   - ``::tiglFuselageGetSectionCenter`` computes the center of a fuselage section defined by its eta coordinate.
   - ``::tiglFuselageGetCrossSectionArea`` computes the area of a fuselage section.
   - ``::tiglFuselageGetCenterLineLength`` computes the length of the centerline of the fuselage.
   - ``::tiglCheckPointInside`` checks, whether a point lies inside some object (defined by its uid).
   - ``::tiglExportFuselageBREPByUID`` and ``::tiglExportWingBREPByUID``
   - ``::tiglSetExportOptions`` sets exports-specific options
   - ``::tiglFuselageSetGetPointBehavior`` and  ``::tiglWingSetGetPointBehavior`` to adjust, whether the get point functions should return linear loft values or points on the actual shape.
	  - ``::tiglSetDebugDataDirectory`` specifies the directory of the crashdump data.
  
 - Changed API:

   - Removed deprectated intersection functions. These include
    - ``tiglComponentIntersectionLineCount``
    - ``tiglComponentIntersectionPoint``
    - ``tiglComponentIntersectionPoints``
   - Removed function ``tiglExportVTKSetOptions``. This is now replaced by ``::tiglSetExportOptions``
	  - ``tiglWingGetWettedArea`` now returns surface area of the wing, if the wing has no parent. Previously this caused a crash.

 - Fixes:

   - TiGL Viewer: Fixed missing fonts on macOS

 - Language bindings:

   - New python bindings allow to access the whole C++ API of TiGL. This bindings can be installed via conda. 
   - Python: the tiglwrapper.py module was renamed to tigl3wrapper.py. The Tigl object is renamed to Tigl3.
   - Java: the tigl package moved from de.dlr.sc.tigl to de.dlr.sc.tigl3

 - TiGL Viewer:

    - Display of reflection lines to inspect surface quality.
    - Display of textured surfaces.
    - Angle of perspective can be adjusted using the scripting API with ``setCameraPosition`` and ``setLookAtPosition``.
   This allows e.g. to create videos of the geometry.
    - Option to display face names.
    - Number of U and V iso-lines can be adjusted independently.
    - New design

Version 2.2.3
--------------
28/06/2018

 - New API functions:

   - Backport API function ``tiglCheckPointInside`` that checks wether a point lies inside a given geometric object
   - New API function ``tiglGetGuideCurvePoints`` to obtain the guide curve points on the fuselage/wing for a given guide curve.


Version 2.2.2
--------------
11/04/2018

 - General changes:

   - Modelling of control surfaces as trailing and leading edge devices
   - The wing is now written a one entity instead of several segments in the file exports 
 
 - New API functions:

   - ``tiglControlSurfaceGetDeflection`` to query the deflection of the given control surface device
   - ``tiglControlSurfaceSetDeflection`` to set the deflection of the given control surface device
   - ``tiglGetControlSurfaceCount`` to get the number of control surface devices
   - ``tiglGetControlSurfaceType`` to query the type of the control surface device (leading + trailing edge device)
   - ``tiglGetControlSurfaceUID`` to query the UID of the control surface device
   - ``tiglControlSurfaceGetMinimumDeflection`` and ``tiglControlSurfaceGetMaximumDeflection`` to
    get the minimum and maximum deflection of the control surface device

 - Fixes:

   - Fixed bug JAVA bindings getProfileSplines (issue #357)
  
 - TiGLViewer:

   - The control surface devices can be extended and retracted from inside the GUI
   - Added functions to set camera positions. The following functions where added:
     - setLookAtPosition(x, y, z)
     - setCameraPosition(elevationAngle, azimuthAngle)
They can be used from the console to e.g. create movies. 


Version 2.2.1
--------------
18/08/2017

 - General changes:

   - Improved calculation time of ``::tiglFuselageGetPointAngle`` by roughly a factor of 30. The results might be a different than in previous versions, but the function should be more robust now.
   - Improved calculation time of ``::tiglFuselageGetPoint`` by applying caching. This leads only to a benefit in case of a large number of GetPoint calls (~30) per fuselage segment. This will be even improved in TiGL 3.

 - New API functions:

   - New API function ``tiglExportVTKSetOptions``. This function can be used e.g to disable
    normal vector writing in the VTK export.

 - Changed API:

   - Ignore Symmetry face in ``::tiglFuselageGetSurfaceArea`` for half fuselages
   - In ``::tiglFuselageGetPointAngle`` the cross section center is used as starting point of the angle rather than the origin of the yz-plane

 - Fixes:

   - Fixed bug, where the VTK export showed no geometry in ParaView
   - Improved accuracy of the VTK export. The digits of points are not truncated anymore to avoid duplicate points
   - Triangles with zero surface are excluded from the VTK export
   - Fixed incorrect face name ordering in WingComponentSegment

Version 2.2.0
--------------
23/12/2016

 - Major changes:

   - Added modelling of the wing structure, including ribs and spars . This code was part of a large
    pull request by Airbus D&S. Currently, the structure is only accessible by the TiGL Viewer.
    In future releases, we plan to make the structure accessible from the API.
   - Improved Collada export: The export is now conforming with the collada schema
    and can be displayed with OS X preview.
   - External shapes are added to the exports.
   - Added writing of modified CPACS files. Still, we do not offer yet API functions
    for modifications. Using the internal API, modifications are already possible.

 - New API functions:

   - New API function ``::tiglSaveCPACSConfiguration`` for writing the CPACS configuration into a file.

 - TiGLViewer:

   - Visualization of the wing structure.
   - Improved linking and compilation with Qt.
   - Added STL export of the whole aircraft configuration.


Version 2.1.7
--------------
22/09/2016

 - General changes:

   - Support for generic aircraft systems (by Jonas Jepsen).
   - External components are now conforming with the CPACS 2.3 standard (by Jonas Jepsen).
   - Support for rotorcraft (by Philipp Kunze).
   - Improved IGES export: Added support for long names in IGES (more than 8 characters as before).
   - Removed support for RedHat 5 and Ubuntu 13.10.
   - Added support for RedHat 7.
   - Ported to OpenCASCADE 6.9.0 and 7.0.0.

 - New API functions:

   - ``::tiglWingComponentSegmentComputeEtaIntersection``: This function should be used to compute points on the wing
     that lie on a straight line between two given points.
   - ``::tiglFuselageGetIndex`` to compute the index based on the fuselage UID.
   - Added new API functions for rotors and rotor blades:
     - ``::tiglGetRotorCount``
     - ``::tiglRotorGetUID``
     - ``::tiglRotorGetIndex``
     - ``::tiglRotorGetRadius``
     - ``::tiglRotorGetReferenceArea``
     - ``::tiglRotorGetTotalBladePlanformArea``
     - ``::tiglRotorGetSolidity``
     - ``::tiglRotorGetSurfaceArea``
     - ``::tiglRotorGetVolume``
     - ``::tiglRotorGetTipSpeed``
     - ``::tiglRotorGetRotorBladeCount``
     - ``::tiglRotorBladeGetWingIndex``
     - ``::tiglRotorBladeGetWingUID``
     - ``::tiglRotorBladeGetAzimuthAngle``
     - ``::tiglRotorBladeGetRadius``
     - ``::tiglRotorBladeGetPlanformArea``
     - ``::tiglRotorBladeGetSurfaceArea``
     - ``::tiglRotorBladeGetVolume``
     - ``::tiglRotorBladeGetTipSpeed``
     - ``::tiglRotorBladeGetLocalRadius``
     - ``::tiglRotorBladeGetLocalChord``
     - ``::tiglRotorBladeGetLocalTwistAngle``

 - Fixes:

   - Fixed parent-child transformations in case of multiple root components.
   - Fixed an error in ``::tiglWingComponentSegmentGetPoint`` in case of multiple intersections of the eta plane with the wing. 
    This was the case for e.g. box wings. (issue #176).
   - Fixed bug ``::tiglWingGetSpan`` in wing span computation when no wing symmetry is given (e.g. for a VTP) (issue #185 and #195).
   - Fixed another bug in ``::tiglWingGetSpan`` when the symmetry plane was the Y-Z plane (issue #174).
   - Fixed incorrect result in ``::tiglWingGetSegmentEtaXsi`` near wing sections, returning the wrong section (issue#187).
   - Fixed an issue in ``::tiglWingGetSegmentEtaXsi`` in case the airfoil is completely above the chord surface.
   - Fixed point projection on the geometry for large scale data (eg a factor of 1000) by making the convergence criterium size dependent (issue #203).

 - TiGLViewer:

   - Visualization of rotorcraft and rotorcraft specific menus.
   - Visualization of generic aircraft systems.
   - Added ``wingGetSpan`` function to TiGLViewer scripting.
   - Added script function ``wingComponentSegmentGetPoint``.
   - Fixed tiglviewer.sh script loading wrong OpenCASCADE libraries.

 - Language bindings:

   - Started experimental python bindings for the internal API.
    This allows a direct manipulation of the geometry objects
    from python together with the OpenCASCADE python bindings (pythoncc).
   - The source code of the matlab bindings is now shipped on all systems. In addition,
    we distribute a Makefile which can be used to compile the Matlab bindings when needed.
   - Added function ``::tiglWingComponentSegmentComputeEtaIntersection`` to java bindings.


Version 2.1.6
-------------
15/07/2015

 - TiGL Viewer:

   - Fixed critical crash on Windows 64 bit systems that occured sporadically on some systems


Version 2.1.5
-------------
01/07/2015

 - Changed API:

   - Added an output argument in the function ``::tiglWingComponentSegmentPointGetSegmentEtaXsi`` that returns the error of the computation.
   - The function ``::tiglWingComponentSegmentGetSegmentIntersection`` uses a new algorithm that should ensure straight 
    flap leading edges and straight spars. Also, a new parameter ``hasWarning`` was added, to inform the user, that the
    returned segment xsi value is not in the valid range [0,1]. This might be the case, if a spar is partially located outside the wing.
   - Removed macros TIGL_VERSION and TIGL_VERSION_MAJOR from tigl.h. Please use tigl_version.h instead.

 - General changes:

   - External geometries can be included into the CPACS file using a link to a STEP file. The allows e.g. the use
    of engines and nacelles. (Note: this is not yet included in the CPACS standard)
   - Improved computation of half model fuselage profiles. Now, fuselages are c2 continuous at the symmetry plane.
   - Improved computation speed of ``::tiglWingComponentSegmentGetPoint`` by a factor of 30 to 600 (depending on the geometry).
   - Reduced execution time of ``::tiglOpenCPACSConfiguration``.
   - All TiXI messages (errors/warnings) are now printed to the TiGL log.
   - Ported to OpenCASCADE 6.8.0.

 - New API functions:

   - ``::tiglExportFusedBREP`` to export the fused configuration to the BRep file format.

 - Fixes:

   - Fixed bug, where guide curves on half model fuselages were not touching the symmetry plane.
   - Fixed a TIGL_MATH_ERROR bug in ``::tiglWingComponentSegmentGetSegmentIntersection``.

 - TiGL Viewer:

   - Ported to Qt 5.
   - OpenGL accelerated rubber band selection. This fixes the slow rubber band selection on Linux and 
    the invisible rubber band selection on Mac.
   - Fixed bug when loading a CPACS file with multiple models (thanks Jonas!).
   - Removed support for legacy VRML and CSDFB files.


Version 2.1.4
-------------
06/02/2015

 - Changed API:

   - Added an output argument in the functions``tiglWingGetUpperPointAtDirection`` and ``tiglWingGetLowerPointAtDirection`` that returns the error of the computation

 - New API functions:

   -  ``tiglFuselageGetIndex`` and ``tiglFuselageGetSegmentIndex`` to retrieve the index of a fuselage and fuselage segment given a CPACS UID

 - Fixes:

   - Fixed some warnings using CMake 3

 - TiGL Viewer:

   - Fixed a crash in case of a missing model UID 
   - Fixed a bug, where debugging BREP files where always created on Linux
   - Improved scripting interface for wingGetLower/UpperPointAtDirection to return the error

 - Language bindings:

   - Completed the new Java bindings for TiGL 
   - Removed Fortran bindings since nobody is using them


Version 2.1.3
-------------
08/12/2014

 - Changed API:

   - Changed functions ``tiglWingGetSegmentSurfaceArea`` to exclude side faces and trailing edges
   - Removed functions ``tiglWingGetUpperPointAtAngle`` and ``tiglWingGetLowerPointAtAngle``. These functions
    were replaced by ``tiglWingGetUpperPointAtDirection`` and ``tiglWingGetLowerPointAtDirection``.

 - General changes:

   - Support for global transformation (translation refType="absGlobal"). Notice: some (incorrect) CPACS models will now look differently.
   - Accurate B-Spline approximation of CST curves using Chebychev approximation.
   - Implemented recursive fusing trimming for the use of e.g. bellyfairings. Added small cpacs example how to model a bellyfairing.
   - Implemented trimming of intersection curves with parent bodies and far fields
   - Improved computation of fuselage positionings. This should improve loading times of configurations 
    with large number of sections, since the algorithmic complexity is reduced.
   - IGES + STEP export: All wing faces are now classified as Top-Wing, Bottom-Wing, or Trailing-Edge.
   - IGES export: Changed units to mm.
   - IGES export: Implemented layers/levels .
   - The OCAF framework is no longer required to build TiGL.
   - The build system now uses the cmake config-style TiXI and OCE search mechanisms.

 - New API functions:

   - Added functions to get the B-Spline paramterization of fuselage and wing profiles:
   - ``tiglProfileGetBSplineCount``,  returns the of B-Splines a profile is built of.
   - ``tiglProfileGetBSplineDataSizes``, returns the size of the knot vector and the number of control points of one profile B-Spline.
   - ``tiglProfileGetBSplineData``, returns the knot vector and the control points.
   - Added functions ``tiglWingGetSectionCount`` and ``tiglFuselageGetSectionCount``
   - Added functions ``tiglWingGetSegmentUpperSurfaceAreaTrimmed`` and ``tiglWingGetSegmentLowerSurfaceAreaTrimmed`` to e.g.
    compute the surface area of a control device.
   - Added functions ``tiglWingGetChordPoint`` and ``tiglWingGetChordNormal`` to query points on the wing chord surface.
   - Added functions ``tiglWingGetUpperPointAtDirection`` and ``tiglWingGetLowerPointAtDirection``.

 - Fixes:

   - Fixed ``tiglWingComponentSegmentGetPoint`` in case of global wing transformations.
   - Fixed ``tiglWingComponentSegmentGetPoint`` bug, returning eta values > 1 (issue 107).
   - Fixed numerical inaccuracy of ``tiglWingGetPointDirection``
   - Fixed incorrect CST curves at for N2 < 1.
   - Fixed null pointer bug in IGES export.


 - TiGLViewer:

   - Highly improved scripting console:
     - Script file can be given as command line argument (using option --script)
     - Added function to export all objects to file
     - The main application objects can be scripted (i.e. app, app.viewer, app.scene)
     - Draw shapes, points and vectors from script
     - Make screenshots by command
     - Context menu for copy-paste actions
     - History with recent commands
     - Mouse support
     - More wrapped tigl functions
     - Improved stability
     - Exception handling
     - TiXI errors and warnings are printed on the console now
    - Added dialog to draw points and vectors.
    - Added dialog for screenshot settings, including option for white background.
    - JPEG and PNG support for background images.
    - Collada export for the complete configuration (i.e. support for multiple objects).
   - Cleanup of menu entries.
   - Fixed multiple opened CPACS documents in TiGLViewer.
   - Fixed 3D view flickering on Mac OS X with Qt 4.8.6.


 - Language bindings:

   - Added python 3 support of ``tiglwrapper.py``
   - Improved TiGL library loading error messages in python wrapper 
   - New Java bindings. Hand-written high level API not yet complete (not all TiGL functions wrapped).
    Low level API (autogenerated) can be used instead for unimplemented functions.
   - Added Java example (see share/doc/tigl/examples/JavaDemo.java)

 - Documentation:

   - Added chapter for TiGL Viewer and the TiGL Viewer scripting console.



Version 2.1.2
-------------
17/04/2014

 - Changed API:

   - The returned UID strings of the following functions must not be freed by the user anymore:

     - ``tiglWingGetOuterSectionAndElementIndex``
     - ``tiglWingGetInnerSectionAndElementUID``
     - ``tiglFuselageGetStartSectionAndElementUID``
     - ``tiglFuselageGetEndSectionAndElementUID``
     - ``tiglWingComponentSegmentPointGetSegmentEtaXsi``
     - ``tiglWingComponentSegmentFindSegment`` 

   - Changed behavior of ``tiglWingComponentSegmentFindSegment``. In case the specified point does not 
    lie within 1 cm of any segment, ``TIGL_NOT_FOUND`` is returned.
   - Changed behavior of ``tiglWingComponentSegmentPointGetSegmentEtaXsi``. If the specified point lies outside
    any segment so that the transformation can not be executed, ``TIGL_MATH_ERROR`` is returned.
   - The following API functions ``tiglComponentIntersectionPoint``, ``tiglComponentIntersectionPoints`` 
    and ``tiglComponentIntersectionLineCount`` are deprecated and will
    be removed in future releases. These functions are replaced by new intersection routines.
   - Removed functions ``tiglExportStructuredIGES`` and ``tiglExportStructuredSTEP``

 - General changes:

   - Completely reworked boolean operations with the following effects:
     - Fusing the whole plane is faster in many cases and more reliable
     - We keep track of the origins of each trimmed face which helps for
     the IGES and STEP exports
     - The boolean operations can be debugged now by setting the environment variable TIGL_DEBUG_BOP
   - Completely rewritten STEP and IGES exports:
     - Each face has now an identifier name
     - Inclusion of intersection curves
     - Inclusion of far fields into exports
     - IGES export in non-BREP mode since it is not supported by CATIA
   - Added new demos for C, Python and MATLAB. Please look into the documentation how to run them.
   - Added function for the intersection computation of a geometrical shape with a plane 
   - Experimental implementation of wing and fuselage guide curves
   - The mathematical orientation of wing profiles is checked for correctness
   - TiGL requires now at least OpenCASCADE 6.6.0 
   - Ported to OpenCASCADE 6.7.0

 - New API functions:

   - Added new API functions for shape/shape and shape/plane intersections:

     - ``tiglIntersectComponents``
     - ``tiglIntersectWithPlane``
     - ``tiglIntersectGetLineCount``
     - ``tiglIntersectGetPoint``

 - Fixes:

   - Fixed a bug in ``tiglWingComponentSegmentFindSegment``
   - Fixed numerical inaccuracy in the projection of points to a wing segment
   - Fixed crash in case of too long UIDs
   - Fixed some memory leaks 

 - TiGLViewer:

   - Improved IGES import: 
     - Multiple shapes are now imported as separate shapes from IGES files.
     - Fixed wrong scaling when importing IGES files.
   - Improved STEP export:
     - Files are now exported in units of meters instead of millimeters.
   - Improved BREP export and import: compound objects are now decomposed to separate objects.
   - Added method to export fused aircraft to BREP
   - Added method to export wing and fuselage profiles and guide curves to BREP
   - Fixed incorrect units in IGES and STEP export from save-as dialog.
   - General save-as dialog: All visible objects are saved to the file, if no shape is selected. 
    Otherwise, only the selected objects are exported.
   - Added debugging parameters to settings dialog. This includes:
     - Adapting number of displayed iso lines per face
     - Display face numbers
     - Debug boolean operations (if enabled, debugging shapes in BRep format are stored 
     to the current working directory)
   - Added new dialog for shape/shape and shape/plane intersections.
   - Added drag and drop support to TiGLViewer. Files (CPACS, IGES, STEP, BREP) are opened when they are dragged into TiGLViewer.
   - TiGLViewer displays far field after calculating the trimmed aircraft
   - Iso U/V lines are no longer displayed by default. This can be changed in the settings dialog.
   - The fused/trimmed aircraft geometry is displayed using a different color for each component.
   - Added visualization of wing and fuselage guide curves.
   - Fix: Only the first intersection curve was displayed. This is fixed now.
   - New icons


Version 2.1.1
-------------
Released: 28/01/2014

 - Changed API:

   - In previous TiGL version, some strings had to be freed manually after calling some functions. 
    These strings must not be freed anymore. Following functions are affected:

     - ``tiglWingGetOuterSectionAndElementIndex``
     - ``tiglWingGetInnerSectionAndElementUID``
     - ``tiglFuselageGetStartSectionAndElementUID``
     - ``tiglFuselageGetEndSectionAndElementUID``
     - ``tiglWingComponentSegmentPointGetSegmentEtaXsi`` 
     - ``tiglWingComponentSegmentFindSegment``

   - Changed return value of ``tiglWingComponentSegmentFindSegment``: In case the given point is 
     located more than 1 cm away from any segment, TIGL_NOT_FOUND is returned.
    - Changed return value of ``tiglWingComponentSegmentPointGetSegmentEtaXsi``: If the given point 
     lies outside any segment so that the transformation can not be executed, TIGL_MATH_ERROR is returned.


 - General changes:

   - Changed console logging to include errors and warnings by default
   - Wing profiles are automatically trimmed at their trailing edge to ensure, that the trailing edge is 
    always perpendicular to the chord line. This is required by the ``wingGetUpperPoint`` and ``wingGetLowerPoint`` functions.

 - Fixes:

   - Fixed a bug in ``tiglWingGetUpperPoint`` and ``tiglWingGetLowerPoint`` in which some points could not be calculated
   - Fixed two memory leaks


Version 2.1.0
-------------
Released: 17/01/2014

 - Changed API:

   - Added argument for ``tiglWingGetReferenceArea`` to define the projection plane for reference area calculations
 
 - General Changes:

   - Support for parametric CST wing profiles
   - Logging improvements. The console verbosity can now be set independent of file logging.
 
 - New API functions:

   - ``tiglWingGetMAC``, computes the mead aerodynamic chord length and position (thanks to Arda!)
 
 - Fixes:

   - Fixed crash in case of missing wing and fuselage profiles
   - Fixed accuracy errors in ``tiglWingSegmentPointGetComponentSegmentEtaXsi`` and ``tiglWingComponentSegmentPointGetSegmentEtaXsi``
   - Fixed a warning when including ``tigl.h``
   - Fixed numerical bug ``tiglWingComponentSegmentGetSegmentIntersection``
 
 - TiGLViewer:

   - Improved dialog for displaying wing component segment points
   - Added BRep export
   - Fixed crash on some Linux systems with strange LANG settings
   - Added dialog showing log history in case of an error


Version 2.0.7
-------------
Released: 21/11/2013

 - Changed API:

   - replaced ``tiglWingComponentSegmentGetMaterialUIDs`` with 
    ``tiglWingComponentSegmentGetMaterialUID`` and ``tiglWingComponentSegmentGetMaterialCount``
 
 - General Changes:

   - Implementation of far fields
   - More work on STEP and IGES export. The geometry is now exported as faces instead of solids in the STEP export.
   - Verification of airfoils during CPACS loading, this fixes twisted wing segments for some CPACS files
   - Improved modelling of leading and trailing edge
   - New Logging Framework
 
 - New API functions:

   - ``tiglWingGetSegmentEtaXsi``, to transform global x,y,z coordinates into wing segment coordinates
   - ``tiglExportFusedSTEP``, exports the trimmed/fused geometry as a step file
   - ``tiglWingComponentSegmentGetMaterialUID``, to get the material UID at a point of the component segment
   - ``tiglWingComponentSegmentGetMaterialThickness``, to get the material thickness at a point of the component segment
   - ``tiglWingComponentSegmentGetMaterialCount``, to get the number of materials defined at a point of the component segment
   - ``tiglLogSetFileEnding``, ``tiglLogSetTimeInFilenameEnabled``, ``tiglLogToFileDisabled``,
    ``tiglLogToFileEnabled``, ``tiglLogToFileStreamEnabled``, to modify logging settings
 
 - Fixes:

   - fixed a bug in ``tiglWingComponentSegmentGetSegmentIntersection``
 
 - TiGLViewer:

   - Loading of HOTSOSE mesh files
   - Auto-reload of non-CPACS files
   - Improved display of airfoils
   - Improved error dialogs


Version 2.0.6
-------------
Released: 27/08/2013


 - General Changes:

   - Improved loading times
   - Improved speed of intersection calculations by caching and reusing results
   - Switched to faces instead of solids in STEP Export 
   - Switched to const ``char * API``
   - Prepared TiGL for Android (currently working but still experimental)
 
 - New API functions:

   - ``tiglExportMeshedWingSTLByUID`` and ``tiglExportMeshedFuselageSTLByUID`` for STL export (Hello 3D printing!)
   - ``tiglWingComponentSegmentGetSegmentIntersection``
 
 - Fixes:

   - Fixed ``tiglFuselageGetPointAngle`` and ``tiglFuselageGetPointAngleTranslated`` giving wrong results (issues 89 and 92)
   - TiGL required positionings for all fuselage segments, this is now fixed (issue 57)
   - Fixed error when opening CPACS files with composite materials
   - Fixed wrong units in iges export (issue 78)
   - Silenced Error messages that weren't errors
   - Fixed different errors in ``tiglWingComponentSegmentPointGetSegmentEtaXsi``
   - Fixed duplicate log files
  
 
Version 2.0.5
-------------
Released: 11/06/2013

 - Changed API:

   - Switched to const ``char * strings``
   - ``tiglWingGetSegmentIndex`` returns now also the wing index instead of asking for it
   - Removed TiGL wire algorithms switching functions. Now a Bspline wire is used for each profile.
  
 - General Changes:

   - Symmetry modeling
   - Explicit modeling of the wing leading edge to improve tesselated output (no more leading edge bumps)
   - Explicit modeling of the wing upper and lower shape
   - The fuselage lofting now creates a smooth surface without hard edges (due to fixed OpenCASCADE bug)
   - Completely rewritten VTK export:
     - Full body export including wing segment metadata
     - Fixed lots of errors. 
     - Large improvement of calculation times. 
     - Provides wing segment metadata in a proper VTK-way, to allow visualization of these data in VTK viewers.
     Calculation and export of the proper normal vectors.
   - IGES and STEP export with CPACS metadata
   - Collada export for use in 3D rendering programs like Blender
   - MATLAB bindings for TiGL!
   - Completeley rewritten python bindings (e.g. better support for arrays)
   - Added logging framework to write outputs into log files (google-glog)
   - Ported to OpenCASCADE 6.6.0
  
 - New API functions:

   - ``tiglWingComponentSegmentGetMaterialUIDs``, to query materials on component segment
   - ``tiglWingComponentSegmentGetPoint``, to query cartesian point on the wing component segment (on chord surface)
   - ``tiglWingSegmentPointGetComponentSegmentEtaXsi``, to compute segment to component segment coordinates
   - ``tiglWingComponentSegmentGetSegmentUID``, queries the uids of the ith segment of the component segment
   - ``tiglWingComponentSegmentGetNumberOfSegments``, queries the number of segments belonging to a component segment
   - ``tiglExportFuselageColladaByUID``
   - ``tiglExportWingColladaByUID``
   - ``tiglExportStructuredIGES``
   - ``tiglExportStructuredSTEP``
   - ``tiglConfigurationGetLength``, returns the length of the airplane
   - ``tiglWingGetSpan``, returns the wing span
   - ``tiglComponentIntersectionPoints`` (convenience function, vectorizes ``tiglComponentIntersectionPoint`` to improve speed)
   - ``tiglExportMeshedGeometryVTKSimple`` and ``tiglExportMeshedGeometryVTKSimple`` (replaced dummy implementation)
  
 - Fixes:

   - Intersection calculation used by ``tiglComponentIntersectionPoint``
   - Fixed incorrectly placed fuselage positionings
   - Fixed ``tiglWingComponentSegmentFindSegment`` returning segments that don't belong to the component segment
   - Workaround to buggy OpenCASCADE boolean fuse algorithms (which seems to be non-commutative). As a result, the fusing of the whole plane
    can be slower than before.
   - Removed warning about missing component segments
  
 - TiGLViewer:

   - Display user defined component segment point
   - Improved speed of intersection calculation
   - Improved speed of shape triangulation
   - Display upper and lower shape of the wing
   - Display of the full model incorporating the symmetry properties of the CPACS components
   - Fixed detection, if CPACS file is changed while displayed in TiGLViewer
   - An optional control file can steer some basic settings like tesselation/triangulation accuracy (e.g. to tune speed of cpacs file opening)
   - STEP export
   - Settings Dialog (tesslation accuracy settings, background...)
   - New icon and color scheme


Version 2.0.4
-------------
Released: 17/01/2013

 - New API Functions: 

   - Added function ``tiglGetErrorString``

 - Fixes:

   - Fixed bug in ``tiglWingComponentSegmentPointGetSegmentEtaXsi``
   - Fixed camberline / chordline bug
   - Fixed exception, when no airfoils are available


 - TiGLViewer:

   - Added scripting interface to TIGLViewer. It is now possible to call TIGL/TIXI functions from within the TIGLViewer. The output is displayed in the console-view. Find more information here.
   - Added context-popup menu on right click. Available actions:
     - Removing of geometric shapes
     - Set transparency level of geometric shapes
     - Set color of selected shapes
     - Set material of selected shapes
     - Set wireframe/shading of selected shapes
   - Works now also in Mac OSX (since opencascade commit 4fe5661 )
   - Added toolbars
   - Added option to load a background image
   - Added autosave of user settings
   - Menus are disabled/enabled depending on dataset
   - Script could be loaded from file via File->open Script
   - Fixed opening from command line
   - Fixed bad font rendering for OpenCascade? > - 6.4.0


Version 2.0.3
-------------
Released: 17/01/2013

 - General Changes:

   - Added ``CCPACSWingSegment::GetChordPoint`` to internal API
   - Added a simple cpacs data set for accuracy testing
   - Added some accuracy tests for getPoint functions
   - Added support for visual leak detector
   - Changed UnitTesting? Framework to google-test
   - Changed TIGL linking to static for unit tests
   - Added coverage with gcov (gcc only) to project

 - New API Functions:

   - ``tiglWingGetIndex`` Returns the wing index given a wing UID
   - ``tiglWingGetSegmentIndex`` Returns the segment index given a wing segment UID
   - ``tiglWingGetComponentSegmentCount`` Returns the number of component segments for a specific wing (selected by wing index)
   - ``tiglWingGetComponentSegmentIndex`` Translates component segment UID into component segment index
   - ``tiglWingGetComponentSegmentUID`` Translates component segment index into component segment UID

 - Fixes:

   - Fixed ``tiglWingComponentSegmentPointGetSegmentEtaXsi``
   - Fixed accuracy of ``CCPACSWingComponentSegment::getPoint``
   - Fixed accuracy of ``CCPACSWingSegment::getEta``
   - Fixed memory management handling of ``tiglWingGetProfileName``
   - Fixed memory leaks in VTK export
   - Fixed memory leak in ``CCPACSFuselages``

 - TIGLViewer:

   - Menus are enabled/disabled depending on number of wings/fuselages
   - Added close configuration menu entry
   - improved view rotation with middle mouse button


Version 2.0.2
-------------
Released: 16/10/2012

 - General Changes:

   - Fixed wing translation bug in TIGLViewer and export Functions
   - Implemented STL import
   - The result of the fused plane calculation is now chached 
 
 - TiGLViewer:

   - Fixed triangulation algorithm (should not crash anymore)
   - Added full plane triangulation
   - Checks in all Selection-Dialogs, if cancel was pressed
   - Added calculation of intersection line of Wing and Fuselage
   - The recently opened folder is now saved
 

Version 2.0.1
-------------
Released: 02/10/2012

 - General Changes:

   - Fixed geometry transformations like sweep angle and dihedral angle rotating sections
   - Fixed scalings that lead to a translation of sections
   - Datasets with CPACS Version < 2.0 are now rejected
   - Calculation of fused fuselage, wing, and airplane should not crash anymore
   - Removed memory leaks
   - Cmake based project files
   - Prepared for OpenCascade? 6.5.3
   - Windows 64 Bit builds available
 
 - TiGLViewer:

   - Fixed inconsistent hot keys and menu entries for views
   - Console window showing outputs of TIGL and OpenCascade?
   - Wireframe mode
   - Recent Documents menu entry


Version 2.0
-----------

 - General Changes:

   - Compatible with CPACS 2.0
   - Corrected the implicit rotation of fuselage and wing profiles. They shouldn't turn from x-y to x-z by TIGL
   - TIGL now uses the coordinate system of parent components for child components. This is only done when parent --> child 
    relations could be figured out via UIDs.
   - Profile points could now also be stored as the vector (x ,y, and z) containing all profile points.
   - Some XPath have changed in CPACS 2.0, for example "sweepangle" is now "sweepAngle". TIGL is taking care of these changes.

 - New API Functions:

   - ``tiglFuselageGetSymmetry`` and ``tiglWingGetSymmetry`` for querying symmetry information
   - ``tiglWingGetReferenceArea`` gives the reference area of a wing
   - ``tiglWingComponentSegmentFindSegment`` returns the segmentUID and wingUID for a given point on a componentSegment
   - ``tiglWingGetWettedArea`` a new function for caluclating wetted area of a wing
   - ``tiglWingComponentSegmentPointGetSegmentEtaXsireturns`` eta, xsi, segmentUID and wingUID for a given eta and xsi on 
     a componentSegment
 
 - TiGLViewer:

   - Complete rebuild of TIGLViewer. It is now QT based and platform independent
   - TIGLViewer now updates the view when the cpacs file is changed.


Version 1.0
-----------
Released: 31/08/2011

 - Changed API:

   - ``tiglOpenCpacsConfiguration`` now opens a configuration without specification if the uid if it is the only one 
    in the data set. Simply take NULL or en empty string as uid argument.
   - Added a Python wrapper for the C-code and DLL handling (no need to manually convert cpython variables to python)

 - New API Functions

   - ``tiglComponentIntersectionPoint`` Returns a point on the intersection line of two geometric components. Often 
    there are more one intersection line, therefore you need to specify the line.
   - ``tiglComponentIntersectionLineCount`` Returns the number if intersection lines of two geometric components.
   - ``tiglComponentGetHashCode`` Computes a hash value to represent a specific shape. The value 
    is computed from the value of the underlying shape reference and the Orientation is not taken into account.
   - ``tiglFuselageGetMinumumDistanceToGround`` Returns the point where the distance between the selected fuselage 
    and the ground is at minimum. Fuselage could be turned with a given angle at at given axis, specified by a 
    point and a direction.

 - TIGLViewer

   - now could draw the componentSegment of a selected wing.


Version 0.9
-----------
Released: 29/04/2011

 - General Changes:

   - Fixed a bug that leads to strange errors when the first positioning of a Wing is not in the origin and has not 
    innerSectionUID-element.
   - Removed annoying strErr message when a point miss one parameter.
   - TIGL 64-Bit libs are available for linux.
   - TIGL is now able to open rotocraft configurations as well as aircraft
