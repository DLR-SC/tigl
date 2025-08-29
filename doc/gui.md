TiGLCreator GUI {#gui}
===========

@section gui_overview Overview

The TiGLCreator is a 3D viewer and editor for CPACS geometries. 
TiGLCreator is based on the TiGL library.

@image html images/tiglcreator-gui.png "The TiGLCreator showing a CPACS model"
@image latex images/tiglcreator-gui.png  "The TiGLCreator showing a CPACS model" width=8cm

__Features of TiGLCreator__

 * 3D Visualization CPACS, STEP, IGES, BREP, and STL files
 * Creation of screen shots
 * Conversion of geometries into standard CAD file formats
 * Conversion into mesh formats such as STL, VTK, and COLLADA (for Blender rendering)
 * A powerful scripting console that allows automated typical workflows, such as creating screen shots  
or making some debug plots.
 * CPACS specific:
  * Display single CPACS entities such as wings, fuselages, profiles and guide curves
  * Compute and export a trimmed aircraft configuration
  * Compute discrete points on wings/fuselages using TiGL functions
  * Edit wing high level parameters @ref sec_wing_param  "see available parameters here"
  * Edit fuselage high level parameters  @ref sec_fuselage_param  "see available parameters here"
  * Edit wing section 
  * Edit fuselage section  
  * Edit positionings
  * Standardization of positionings
  * Create a wing
  * Create a fuselage 
  * Undo/Redo operation
  
  
@section widgets Widgets presentation

Here we will briefly present the different widgets that are available in TiGLCreator. Note that each widget can 
be move around or closed. To close or activate the widgets use the shortcut or go in the display menu (View->Display).

@image html images/creator-gui-explained.png "Main widow and widgets of TiGLCreator"

@subsection scene The scene
The scene displays the 3D tigl object. You can choose which object you want to draw in Draw menu. 
Remark: No modification on the object can be performed from the scene. The scene is only a display interface.

@subsection cpcacstree_view The CPACS tree view
The CPACS tree view shows the CPACS tree structure. When you click on an element in the tree, the correct editor 
widget is activated. By default the tree view widget filters the objects to display only the most import objects. 
You can see all the CPACS objects by clicking on "Expert view" mode at the top of the tree view widget.
You can also filter the tree by some UID or CPACS type be typing in the line next to the "Expert view" checkbox.


@subsection edtior_widget The Editor widget
The editor widget displays the parameters that can be modified for the object selected in the tree view. 


@subsection console The console
A page is dedicated to the console: Please visit @subpage gui_console "this page" for further information



@section navigation Navigation in the 3D view

The most often used navigation functions are included in the tool bar 

@image html images/tiglcreator-navigation.png
@image latex images/tiglcreator-navigation.pdf  "The navigation panel" width=11cm

Here, one can use

 * "Select shapes" to select some parts ("shapes") of the geometry with the
    mouse.
 * "Pan view" to translate the whole geometry with the mouse
 * "Rotate view" to rotate the view point with the mouse
 * "Zoom" to zoom in or out with the mouse
 * "Fit to view" to fit all objects into the current view

 * "Top" to see the top view of the geometry
 * "Side" to see the side view of the geometry
 * "Front" to see the front view of the geometry
 * "Axonometric" to see the axonometric view of the geometry
 * "Reflection plot" to visualize C1 and C2 surface discontinuities

More functions can be found in the "View" menu. 

@section shortcuts Keyboard shortcuts

All actions in the TiGL Creator can be accessed using the application menu or the context menu inside the 3D view. To improve usability, some the actions can also be executed with keyboard shortcuts. The most practical ones are:

__Basic actions__
 | Keys   |  Action                     |
 |:------:|:----------------------------|
 | Ctrl+O | open file                   |
 | Ctrl+S | save file                   |
 | Ctrl+Shift+S | save file as                  |
 | Ctrl+Q | close program               |
 | Ctrl+N | create a new file from template    |

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
  
  
@section gui_settings GUI Settings

The settings dialog allows some customization of the visualization of objects and the export of meshes. It is opened by clicking on File -> Settings.

__Display Settings__

 * Tesselation accuracy: the accuracy how the mathematical geometries are converted to triangles. The higher
   this setting is, the more triangles are created. High values typical require more computation time. __Default: 5__.
 * Triangulation accuracy: similar to tesselation accuracy, but only used for export of triangular meshes (VTK, COLLADA, STL).
   __Default: 5__.
 * Background color: base color of the 3D viewer's background gradient.

@image html images/tiglcreator_displaysettings.jpg
@image latex images/tiglcreator_displaysettings.jpg  "The display settings dialog" width=7cm

__Debugging__

 * Enumerate faces: If enabled, a number will be displayed next to each face in the viewer. This helps to understand
   the order of face creation. Mostly useful for TiGL developers. __Default: off__
 * Debug boolean operations: Boolean operations tend to be quite unstable due to the problems in the OpenCASCADE kernel.
   To improve the debugging of such operations, TiGL Creator can export intermediate geometries as BREP files to disk.
   These files can again be displayed in the TiGL Creator. In case of an error, these files should be send to the TiGL
   developers. The files are placed inside the current working directory. __Default: off__

@image html images/tiglcreator_debugging.jpg
@image latex images/tiglcreator_debugging.jpg  "The debugging dialog" width=7cm


  
@section custom_graphics Graphics Customization

The rendering off all geometrical objects can be be customized to some extends. In order modify some rendered objects, 
select the objects of interest and __press the right mouse button__ inside the 3D view. The following actions are available:
 * Setting the material (which affects the shading behavior)
 * Setting the object color and transparency
 * Toggle between wireframe and shading rendering
 
@image html images/tiglcreator-shading-web.jpg "Different shading settings (textured, plastic blue, wireframe)"
@image latex images/tiglcreator-shading.png  "Different shading settings (textured, plastic blue, wireframe)" width=12cm

@section mobile Mobile platforms

A simplified version of the TiGL Creator is also available for mobile devices based on Android. It can be downloaded from the 
[Google play store](https://play.google.com/store/apps/details?id=de.dlr.sc.tiglcreator.android&hl=de).
  
@image html images/tiglcreator-android-web.jpg "The TiGL Creator App for Android"
@image latex images/tiglcreator-android.png  "The TiGL Creator App for Android" width=9cm

