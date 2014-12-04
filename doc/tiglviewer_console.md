The TiGL Viewer Console {#tiglviewer_console}
------------------

TiGL Viewer comes with a powerful JavaScript console that allows automatizing small workflows or helps 
debugging of geometries. To show and hide the console, press __ALT+C__.

The console has two purposes

 1. Show all kinds of error messages, warnings etc. from the TiGL and OpenCASCADE engine
 2. The user can enter code to steer the TiGL Viewer application

In many cases, the user wants to open a CPACS file and execute TiGL function to compute and draw some points
on the aircraft surface. Another use case would be to automatize the creation of screenshots of a changing aircraft
geometry.

@image html images/tiglviewer_console.png "The scripting console"
@image latex images/tiglviewer_console.png  "The scripting console" width=9cm

We copied some practical features from the famous bash terminal to improve the usability of the console. This includes 
a history of the recently typed commands. Just type 

    history 

To navigate through the recent commands from the history, use the up and down arrow keys.

To clear the console, enter

    clear

For a general help, type in

    help

### Global functions ###

 * drawPoint
 * drawVector
 * drawShape
 * Point3d


### The main application objects ###

The TiGL Viewer scripting engine consists of four main objects. The whole application can be controlled with only
these four objects. These are app, app.viewer, app.scene, and tigl. 

The scripting engine contains a _help()_ function that shows the available methods and properties for each object. 
To show the help for e.g. the tigl object, enter

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
help(tigl);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~  

#### The app object ####

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

#### The app.viewer object ####

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

The most important methods of the app.viewer object are:

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

#### The app.scene object ####

#### The tigl object ####


Make Screenshots

### Examples ###

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
app.saveFile("part.igs");
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
app.scene.toggleGrid(false);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

_Show polar grid in y-z plane_
~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.js}
app.scene.gridCirc();
app.scene.gridYZ();
app.scene.toggleGrid(true);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~