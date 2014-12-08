The TiGL Viewer {#tiglviewer}
===============

The TiGL Viewer is a 3D viewer for CPACS geometries. In addition, it also allows opening standard CAD file formats like IGES, STEP and BREP.
Historically, TiGL Viewer was created as a TiGL debugging tool to visualize the modeled geometries.

@image html images/tiglviewer-web.jpg "The TiGL Viewer showing a CPACS model of the Enterprise NCC 1701"
@image latex images/tiglviewer.png  "The TiGL Viewer showing a CPACS model of the Enterprise NCC 1701" width=8cm

__Features of TiGL Viewer__

 * 3D Visualization CPACS, STEP, IGES, BREP, and STL files
 * Creation of screen shots
 * Conversion of geometries into standard CAD file formats
 * Conversion into mesh formats such as STL, VTK, and COLLADA (for Blender rendering)
 * A powerful @subpage tiglviewer_console "scripting console" that allows automatizing typical workflows, such as creating screen shots 
or making some debug plots.
 * CPACS specific:
  * Display single CPACS entities such as wings, fuselages, profiles and guide curves
  * Compute and export a trimmed aircraft configuration
  * Compute discrete points on wings/fuselages using TiGL functions

@section navigation Navigation in the 3D view

The most often used navigation functions are included in the tool bar 

@image html images/tiglviewer-navigation.png
@image latex images/tiglviewer-navigation.pdf  "The navigation panel" width=9cm

Here, one can use

 * "Select shapes" to select some parts ("shapes") of the geometry with the
    mouse.
 * "Pan view" to translate the whole geometry with the mouse
 * "Rotate view" to rotate the view point with the mouse
 * "Zoom" to zoom in or out with the mouse

 * "Top" to see the top view of the geometry
 * "Side" to see the side view of the geometry
 * "Front" to see the front view of the geometry
 * "Axonometric" to see the axonometric view of the geometry

More functions can be found in the "View" menu. 

@section shortcuts Keyboard shortcuts

All actions in the TiGL Viewer can be accessed using the application menu or the context menu inside the 3D view. To improve usability, some the actions can also be executed with keyboard shortcuts. The most practical ones are:

__Basic actions__
 | Keys   |  Action                     |
 |:------:|:----------------------------|
 | Ctrl+O | open file                   |
 | Ctrl+S | save file                   |
 | Ctrl+Q | close program               |

__Change view__

 | Keys   |  Action                     |
 |:------:|:----------------------------|
 | 1      | Front view                  |
 | 2      | Back view                   |
 | 3      | Top view                    |
 | 4      | Bottom view                 |
 | 5      | Left view                   |
 | 6      | Right view                  |
 | Ctrl-D | Axonometric view            |
 | +      | Zoom in                     |
 | -      | Zoom out                    |
 | Ctrl-W | Toggle display of wireframe |
 | Alt-C  | Toggle display of console   |
 | Ctrl-E | Enable zoom mode            |
 | Ctrl-T | Enable pan view mode        |
 | Ctrl-R | Enable rotate view mode     |
 | Ctrl-A | Fits all  objects into view | 
 | Ctrl-G | Toggle display of grid      |
  
@section settings The Settings

The settings dialog allows some customization of the visualization of objects and the export of meshes. It is opened by clicking on File -> Settings.

__Display Settings__

 * Tesselation accuracy: the accuracy how the mathematical geometries are converted to triangles. The higher 
   this setting is, the more triangles are created. High values typical require more computation time. __Default: 5__.
 * Triangulation accuracy: similar to tesselation accuracy, but only used for export of triangular meshes (VTK, COLLADA, STL).
   __Default: 5__.
 * Background color: base color of the 3D viewer's background gradient.

@image html images/tiglviewer_displaysettings.jpg
@image latex images/tiglviewer_displaysettings.jpg  "The display settings dialog" width=7cm
 
__Debugging__

 * Enumerate faces: If enabled, a number will be displayed next to each face in the viewer. This helps to understand
   the order of face creation. Mostly useful for TiGL developers. __Default: off__
 * Debug boolean operations: Boolean operations tend to be quite unstable due to the problems in the OpenCASCADE kernel. 
   To improve the debugging of such operations, TiGL Viewer can export intermediate geometries as BREP files to disk. 
   These files can again be displayed in the TiGL Viewer. In case of an error, these files should be send to the TiGL
   developers. The files are placed inside the current working directory. __Default: off__
 
@image html images/tiglviewer_debugging.jpg
@image latex images/tiglviewer_debugging.jpg  "The debugging dialog" width=7cm


@section custom Graphics Customization

The rendering off all geometrical objects can be be customized to some extends. In order modify some rendered objects, 
select the objects of interest and __press the right mouse button__ inside the 3D view. The following actions are available:
 * Setting the material (which affects the shading behavior)
 * Setting the object color and transparency
 * Toggle between wireframe and shading rendering
 
@image html images/tiglviewer-shading-web.jpg "Different shading settings (glossy red, plastic blue, wireframe)"
@image latex images/tiglviewer-shading.png  "Different shading settings (glossy red, plastic blue, wireframe)" width=12cm

@section mobile Mobile platforms

A simplified version of the TiGL Viewer is also available for mobile devices based on Android. It can be downloaded from the 
[Google play store](https://play.google.com/store/apps/details?id=de.dlr.sc.tiglviewer.android&hl=de).
  
@image html images/tiglviewer-android-web.jpg "The TiGL Viewer App for Android"
@image latex images/tiglviewer-android.png  "The TiGL Viewer App for Android" width=9cm
