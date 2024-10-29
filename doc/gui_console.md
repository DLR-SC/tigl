The CPACSCreator Console {#gui_console}
========================

TiGL Viewer comes with a powerful JavaScript console that allows automatizing small workflows or helps 
debugging of geometries. To show and hide the console, press __ALT+C__.

The console has two purposes

 1. to show all kinds of error messages, warnings etc. from the TiGL and OpenCASCADE engine
 2. to enter user code to steer the TiGL Viewer application

In many cases, the user wants to open a CPACS file and execute TiGL function to compute and draw some points
on the aircraft surface. Another use case would be to automate the creation of screenshots of a changing aircraft
geometry.

@image html images/tiglviewer_console.png "The scripting console"
@image latex images/tiglviewer_console.png  "The scripting console" width=9cm

We copied some practical features from the famous bash terminal to improve the usability of the console. This includes 
a history of the recently typed commands. Just type 

@code{.js}
history 
@endcode

To navigate through the recent commands from the history, use the up and down arrow keys.

To clear the console, enter

@code{.js}
clear 
@endcode

For a general help, type in

@code{.js}
help 
@endcode

@section console_basic Basic use

The console understands all kinds of JavaScript elements. Thus, typical constructs like loops, functions, and even classes can be defined.
Instead of typing the code into the console, TiGL Viewer can also load and execute a script file. If you want to load a script file during
the startup of TiGL Viewer, use the `--script` option:

     TIGLViewer --script myscript.js [--filename aircraft.cpacs.xml]

__The `ans` object__: The result of the last command will be stored in a variable `ans`. This can be sometimes quite practical:

@code{.js}
>> 2+3
5
>> ans*ans
25
>> ans - 5
20
@endcode

In addition to pure JavaScript, TiGL Viewer offers some function to draw points, vectors and TiGL shapes.

__Draw a point at (0,0,0)__

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
drawPoint(new Point3d(0,0,0));
~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 

__Draw an arrow in z direction__

@code{.js}
pnt = new Point3d(0,0,0);
dir = new Point3d(0,0,5);
drawVector(pnt, dir);
@endcode

__Draw the first fuselage of a cpacs file__

@code{.js}
uid = tigl.fuselageGetUID(1);
shape = tigl.getShape(uid);
drawShape(shape);
@endcode

In case you want to do some vector arithmetic, we defined the `Point3d` class, which offers some basic
vector functionality:

@code{.js}
>> p = new Point3d(10,0,0)
Point3d(10,0,0)
>> p.length()
10
>> p.add(p)
Point3d(20,0,0)
>> p.normalized()
Point3d(1,0,0)
>> p.dot(p)
100
>> p.mult(0.5)
Point3d(5,0,0)
>> q = new Point3d(0,10,0)
Point3d(0,10,0)
>> p.cross(q)
Point3d(0,0,100)
@endcode

To get a list of all `Point3d` methods, enter

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
help(new Point3d);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~  


@section console_mainobj The main application objects


The TiGL Viewer scripting engine consists of four main objects. The whole application can be controlled with only
these four objects. These are app, app.viewer, app.scene, and tigl. 

The scripting engine contains a _help()_ function that shows the available methods and properties for each object. 
To show the help for e.g. the tigl object, enter

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
help(tigl);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

@subsection app The app object

This is the main object of the application. It is used to load and save files, load scripts, control the application's window
size, and also close the application. Some methods of the app object are

|    method          |         description                                          |
---------------------|--------------------------------------------------------------|
| openFile           | opens a file (CPACS, STEP, IGES, BREP)                       |
| saveFile           | saves all visible objects to a file (STEP, IGES, BREP, STL)  |
| openScript         | executes a TiGL Viewer script file                           |
| closeConfiguration | closes the current CPACS configuration and clears the scene  |
| close              | terminates TiGL Viewer                                       |

To get a list of all methods and properties, enter help(app) in the TiGL Viewer console.

@subsection appviewer The app.viewer object

This object controls the rendering of the 3D view. It can be used to

 * change the camera position,
 * make screen shots,
 * set the background color or a background image,
 * and fit all objects into the view.

There are different options, to make a screen shot of the viewer. The simplest is

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.viewer.makeScreenshot("myfile-white.jpg");
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This will create a screenshot with a white background color and the size of the image will be the
size of the viewer's window. To disable the white background, type

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.viewer.makeScreenshot("myfile.jpg", false);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition, you can also control the size of the resulting image file. However, __this feature does not work
in Linux or Max OS X__. The syntax is the following:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.viewer.makeScreenshot("myfile-large.jpg", true, 1500, 1000);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The most important methods of the _app.viewer_ object are:

|    method             |         description                                                     |
------------------------|-------------------------------------------------------------------------|
| setBackgroundColor    | sets a solid background color (RGB triple, 0..255)                      |
| setBackgroundGradient | sets a background with gradually darkening color(RGB triple, 0..255)    |
| viewTop               | moves camera to the top view                                            |
| viewFront             | moves camera to the front view                                          |
| viewLeft              | moves camera to the left view                                           |
| viewAxo               | changes camera to axonometrix view (top, left, front)                   |
| fitAll                | fits all objects into the viewer window                                 |
| zoomIn                | zooms in the camera                                                     |
| zoomOut               | zooms out the camera                                                    |

To get a list of all methods and properties, enter 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
help(app.viewer);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~  
 
in the TiGL Viewer console.


@subsection appscene The app.scene object


The application scene controls, which objects are shown in the 3D view. Thus, it offers methods to display
objects and points, to clear the whole scene and modifying the grid display.

The most important _app.scene_ methods are:

|    method             |         description                                                     |
------------------------|-------------------------------------------------------------------------|
| wireFrame             | if argument is true, all objetcs will be displayed in wireframe mode    |
| displayShape          | displays a TiGL geometry (e.g. got from tigl.getShape("wingUID"))       |
| drawPoint             | draws a point (e.g. app.scene.drawPoint(0,0,10))                        |
| drawVector            | draws a vector/arrow (e.g. app.scene.drawVector(x,y,z,dx,dy,dz))        |
| deleteAllObjects      | clears the scene                                                        |
| gridOn                | displays the grid                                                       |
| gridOff               | turns of the grid display                                               |
| gridXY                | Sets the grid into the X-Y plane                                        |
| gridXZ                | Sets the grid into the X-Z plane                                        |
| gridYZ                | Sets the grid into the Y-Z plane                                        |
| gridCirc              | Switches the grid into polar form                                       |
| gridRect              | Swictehs the grid into cartesian form                                   |

To get a list of all app.scene methods and properties, enter:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
help(app.scene);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

@subsection console_tigl The tigl object

The tigl object is used to access the TiGL library functions. Currently, not all functions are wrapped. 

These are the functions currently wrapped by the scripting interface

 * getWingCount()
 * getVersion()
 * componentGetHashCode(componentUID)
 * componentIntersectionLineCount(componentUidOne, componentUidTwo)
 * exportFusedWingFuselageIGES(filename)
 * exportIGES(filename)
 * exportSTEP(filename)
 * exportMeshedFuselageSTL(fuselageIndex, filename, deflection)
 * exportMeshedFuselageVTKByIndex(fuselageIndex, filename, deflection)
 * exportMeshedFuselageVTKByUID(fuselageUID, filename, deflection)
 * fuselageGetUID(fuselageIndex)
 * fuselageGetCircumference(fuselageIndex, segmentIndex, eta)
 * fuselageGetPoint(fuselageIndex, segmentIndex, eta, zeta)
 * fuselageGetSegmentUID(fuselageIndex, segmentIndex)
 * fuselageGetSegmentVolume(fuselageIndex, segmentIndex)
 * getFuselageCount()
 * fuselageGetSegmentCount(fuselageIndex)
 * wingGetUpperPoint(wingIndex, segmentIndex, eta, xsi)
 * wingGetUID(wingIndex)
 * wingGetLowerPoint(wingIndex, segmentIndex, eta, xsi)
 * wingGetUpperPointAtDirection(wingIndex, segmentIndex, eta, xsi, dirx, diry, dirz)
 * wingGetLowerPointAtDirection(wingIndex, segmentIndex, eta, xsi, dirx, diry, dirz)
 * wingGetChordPoint(wingIndex, segmentIndex, eta, xsi)
 * wingGetChordNormal(wingIndex, segmentIndex, eta, xsi)
 * wingGetSegmentCount(wingIndex)
 * wingGetSegmentUID(wingIndex, segmentIndex)
 * getErrorString(errorCode)
 * getShape(uid)

In comparison to the C/C++ API, the scripting functions don't return error codes. Instead, an exception is thrown in case of an error, e.g. if 
no CPACS file is currently open.

The use of these functions is analog to the TiGL functions of the C API. The only exception is the `tigl.getShape(uid)` method. It can be used
to return the CAD representation of a CPACS entity. Currently, only wings, fuselages, wing segments, and fuselage segments can be
returned by `getShape`. The returned shape can then be rendered using the `drawShape` command.

The point sampling functions (e.g. `fuselageGetPoint`, `wingGetChordPoint`, ...) return a Point3d object.

__Example__

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.openFile("aircraft.cpacs.xml");
p = tigl.wingGetUpperPoint(1, 1, 0.5, 0.2);
drawPoint(p);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 


@section examples Examples

Here are some real life examples how to use the scripting engine:

_Open a file_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.openFile("airplane.cpacs.xml");
~~~~~~~~~~~~~~~~~~~~~~~~~~~~    



_Change to top view and save screenshot to image.png_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.viewer.viewTop();
app.viewer.fitAll();
app.viewer.zoomOut();
app.viewer.makeScreenshot("image.png");
~~~~~~~~~~~~~~~~~~~~~~~~~~~~    


_Convert a __STEP__ file into __IGES__ format and close TiGL Viewer_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.openFile("part.stp");
app.exportFile("part.igs");
app.close();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 


_Display all wings of an aircraft (which is already opened)_

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
for (iwing = 1; iwing <= tigl.getWingCount(); ++iwing) {
  uid = tigl.wingGetUID(iwing);
  shape = tigl.getShape(uid);
  drawShape(shape);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

_Switch to fullscreen view_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.showFullScreen();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

_Delete all visible objects_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.scene.deleteAllObjects();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

_Disable the coordinate grid_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.scene.gridOff();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

_Show polar grid in y-z plane_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.scene.gridCirc();
app.scene.gridYZ();
app.scene.gridOn();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~