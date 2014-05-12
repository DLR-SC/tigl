/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "OsgMainApp.hpp"
#include "TiglViewerBackground.h"
#include "MaterialTemplate.h"
#include "PickHandler.h"
#include "CADImport.h"

#include <sstream>
#include <string>
#include <algorithm>
#include <tigl.h>
#include <CTiglLogging.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSConfiguration.h>
#include <CCPACSWing.h>
#include <CCPACSWingSegment.h>
#include <CCPACSFuselage.h>
#include <CCPACSFuselageSegment.h>
#include <CTiglTriangularizer.h>
#include <map>
#include <osg/ShapeDrawable>
#include <TopTools_HSequenceOfShape.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

#define HOME_POS 200

const double tesselationAccu = 0.01;

OsgMainApp& OsgMainApp::Instance()
{
    static OsgMainApp _instance;
    return _instance;
}

OsgMainApp::OsgMainApp()
{
    init();
}

void OsgMainApp::init()
{

    _initialized = false;
    _assetManager = NULL;
    soleViewer = NULL;

    // pipe osg and tigl message to android
    _notifyHandler = new OsgAndroidNotifyHandler();
    _notifyHandler->setTag("TiGL Viewer");
    osg::setNotifyHandler(_notifyHandler);
    _logAdapter = CSharedPtr < TiglAndroidLogger > (new TiglAndroidLogger);
    _logAdapter->SetTag("TiGL Viewer");
    tigl::CTiglLogging::Instance().SetLogger(_logAdapter);

}
OsgMainApp::~OsgMainApp()
{
}

osg::Node* OsgMainApp::addCross(osg::ref_ptr<osgViewer::View> view, int x, int y, int size)
{
    TiglViewerHUD * HUD = new TiglViewerHUD();
    HUD->setViewport(x, y, size, size);

    HUD->setCoordinateCrossEnabled(true);
    HUD->setMainCamera(view->getCamera());

    return HUD;
}

void OsgMainApp::setAssetManager(AAssetManager* mgr)
{
    _assetManager = mgr;
}

AAssetManager* OsgMainApp::getAssetManager()
{
    return _assetManager;
}

void OsgMainApp::initOsgWindow(int x, int y, int width, int height)
{

    float ar = (float)width/ (float)height;


    if (soleViewer) {
        // the window changed (resize)
        // get previous screen area
        double left, right, bottom, top, zNear, zFar;
        soleViewer->getCamera()->getProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar);
        if (screenWidth > screenHeight && width < height) {
            // screen rotated from landscape to portrait
            // map former width to height
            left = bottom;
            right = top;
            double _width = right - left;
            bottom = -_width/(2.*ar);
            top    =  _width/(2.*ar);
        }
        else if (screenWidth < screenHeight && width > height) {
            // screen rotated from portrait to landscape
            // map former height to width
            bottom = left;
            top = right;
            double _height = top - bottom;
            left   = -_height*ar/2.;
            right  =  _height*ar/2.;
        }
        else {
            // map the width
            double center = (top + bottom)/2.;
            double _width = right - left;
            bottom = -_width/(2.*ar);
            top    =  _width/(2.*ar);
        }

        soleViewer->setUpViewerAsEmbeddedInWindow(x, y, width, height);
        soleViewer->getEventQueue()->setMouseInputRange(x, y, x + width, y + height);
        soleViewer->getCamera()->setProjectionMatrixAsOrtho(left, right, bottom, top, zNear, zFar);

        screenHeight = (float) height;
        screenWidth = (float) width;
        return;
    }

    screenHeight = (float) height;
    screenWidth = (float) width;

    osg::notify(osg::ALWAYS) << "create viewer";

    soleViewer = new osgViewer::Viewer();
    osgViewer::GraphicsWindowEmbedded* window = soleViewer->setUpViewerAsEmbeddedInWindow(x, y, width, height);

    soleViewer->getEventQueue()->setMouseInputRange(x, y, x + width, y + height);
    soleViewer->getCamera()->setClearColor(osg::Vec4(0, 0, 0, 0));
    float zoomLevel = 0.1;
    soleViewer->getCamera()->setProjectionMatrixAsOrtho(-screenWidth/2.*zoomLevel, screenWidth/2. * zoomLevel, -screenHeight/2. * zoomLevel, screenHeight/2. * zoomLevel, 0, 1000);

    osg::ref_ptr<OrthoManipulator> tm = new OrthoManipulator(soleViewer->getCamera());
    tm->setHomePosition(osg::Vec3(0, 0, HOME_POS), osg::Vec3(0, 0, 0), osg::Vec3(0, 1 , 0));

    soleViewer->addEventHandler(new osgViewer::StatsHandler);
    soleViewer->addEventHandler(new osgGA::StateSetManipulator(soleViewer->getCamera()->getOrCreateStateSet()));
    soleViewer->addEventHandler(new osgViewer::ThreadingHandler);
    soleViewer->addEventHandler(new osgViewer::LODScaleHandler);
    soleViewer->addEventHandler(new PickHandler);

    soleViewer->setCameraManipulator(tm);
    soleViewer->realize();

    createScene();
    changeCamera(0);

    _initialized = true;
}

void OsgMainApp::createScene()
{
    osg::notify(osg::ALWAYS) << "createScene called";
    if (!soleViewer) {
        osg::notify(osg::FATAL) << "Cannot create scene, no viewer created!!!";
        return;
    }

    root_1 = new osg::Group();
    modeledObjects = new osg::Group();

    _state = root_1->getOrCreateStateSet();
    _state->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    _state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    _state->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

    osgViewer::Viewer::Views views;
    soleViewer->getViews(views);
    osgViewer::ViewerBase::Contexts contexts;
    soleViewer->getContexts(contexts);
    if (contexts.size() > 0) {
        osg::notify(osg::ALWAYS) << "Number of contexts: " << contexts.size();
        const osg::GraphicsContext::Traits* t = contexts[0]->getTraits();
        int size = t->width > t->height ? t->height : t->width;
        size /= 3;
        root_1->addChild(addCross(views[0], t->x, t->y, size));
    }
    else {
        root_1->addChild(addCross(views[0], 0, 0, 40));
    }

    _coordinateGrid = new VirtualVisObject();
    _coordinateGrid->setMainAxesEnabled(false);
    root_1->addChild(_coordinateGrid);


    // add background as gradient
    osg::ref_ptr<TiglViewerBackground> bg = new TiglViewerBackground;
    bg->makeGradient(osg::Vec4(0.6, 0.6, 1., 1.));
    root_1->addChild(bg);
    root_1->addChild(modeledObjects);

    soleViewer->setSceneData(root_1.get());
}
void OsgMainApp::draw()
{
    soleViewer->frame();
}

bool OsgMainApp::openFile( const std::string& filename )
{

    std::string::size_type idx;
    idx = filename.rfind('.');
    std::string fileType;
    if (idx != std::string::npos) {
        fileType = filename.substr(idx+1);
    }
    else {
        return false;
    }

    std::transform(fileType.begin(), fileType.end(), fileType.begin(), ::tolower);

    FileFormat format;
    bool isCAD = false;
    if (fileType == "xml") {
        format = FormatCPACS;
        isCAD = true;
    }
    if (fileType == "brep" || fileType == "rle") {
        format = FormatBREP;
        isCAD = true;
    }
    if (fileType == "step" || fileType == "stp") {
        format = FormatSTEP;
        isCAD = true;
    }
    if (fileType == "iges" || fileType == "igs") {
        format = FormatIGES;
        isCAD = true;
    }
    if (fileType == "stl") {
        format = FormatSTL;
    }
    if (fileType == "mesh") {
        format = FormatMESH;
    }
    if (fileType == "vtp") {
        format = FormatVTK;
    }

    if (isCAD) {
        openCADModel( format, filename );
        fitScreen();
    }
    else {
        openTriangulationModel( format, filename);
        fitScreen();
    }

    return true;
}

bool OsgMainApp::isFileSupported(const std::string& filename)
{
    std::string::size_type idx;
    idx = filename.rfind('.');
    std::string fileType;
    if (idx != std::string::npos) {
        fileType = filename.substr(idx+1);
    }
    else {
        return false;
    }

    std::transform(fileType.begin(), fileType.end(), fileType.begin(), ::tolower);

    if (fileType == "xml"  ||
        fileType == "brep" ||
        fileType == "igs"  || fileType == "iges" ||
        fileType == "stp"  || fileType == "step" ||
        fileType == "vtp"  ||
        fileType == "mesh") {

        return true;
    }
    else {
        return false;
    }
}

void OsgMainApp::openTriangulationModel( FileFormat format, std::string filename)
{
    switch(format) {
    case FormatVTK:
        addObjectFromVTK(filename);
        break;
    case FormatMESH:
        addObjectFromHOTSOSE(filename);
        break;
    default:
        return;
    }
}

//Events
void OsgMainApp::addObjectFromVTK(std::string filepath)
{
    osg::notify(osg::ALWAYS) << "Opening VTK file " << filepath << std::endl;

    osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
    geode->readVTK(filepath.c_str());
    geode->setName(filepath);

    modeledObjects->addChild(geode.get());
}

void OsgMainApp::addObjectFromHOTSOSE(std::string filepath)
{
    osg::notify(osg::ALWAYS) << "Opening Hotsose file " << filepath << std::endl;

    osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
    geode->readHotsoseMesh(filepath.c_str());
    geode->setName(filepath);

    modeledObjects->addChild(geode.get());
}

void OsgMainApp::removeObjects()
{
    std::vector<osg::Node*> selected;
    for (int iChild = 0; iChild < modeledObjects->getNumChildren(); ++iChild) {
        GeometricVisObject* obj = dynamic_cast<GeometricVisObject*>(modeledObjects->getChild(iChild));
        if (obj && obj->isPicked()) {
            selected.push_back(obj);
        }
    }
    if (selected.size() == 0) {
        modeledObjects->removeChildren(0, modeledObjects->getNumChildren());
    }
    else {
        std::vector<osg::Node*>::iterator it;
        for (it = selected.begin(); it != selected.end(); ++it) {
            modeledObjects->removeChild(*it);
        }
    }

}

void OsgMainApp::changeCamera(int view)
{
    if (!soleViewer) {
        return;
    }

    osgGA::StandardManipulator* m = dynamic_cast<osgGA::StandardManipulator*>(soleViewer->getCameraManipulator());
    if (!m) {
        return;
    }

    osg::Vec3d eye, center, up, dir;
    m->getTransformation(eye, center, up);
    double distance = (eye-center).length();

    /*pres*/
    if (view == 0) {
        // perspective
        dir = osg::Vec3d(-1, -1, 1);
        up = osg::Vec3d(1,1,1);
    }
    else if (view == 1) {
        // top
        dir = osg::Vec3d(0, 0, 1);
        up = osg::Vec3d(0,1,0);
    }
    else if (view == 2) {
        // side
        dir = osg::Vec3d(0, -1, 0);
        up = osg::Vec3d(0,0,1);
    }
    else if (view == 3) {
        // front
        dir = osg::Vec3d(-1, 0, 0);
        up = osg::Vec3d(0,0,1);
    }
    dir.normalize();
    eye = center + dir*distance;
    m->setTransformation(eye, center, up);
}

void OsgMainApp::fitScreen()
{
    // zoom in picked objects, if any
    osg::ref_ptr<osg::Group> selectedObjs = new osg::Group;
    for (int iChild = 0; iChild < modeledObjects->getNumChildren(); ++iChild) {
        GeometricVisObject* obj = dynamic_cast<GeometricVisObject*>(modeledObjects->getChild(iChild));
        if (obj && obj->isPicked()) {
            selectedObjs->addChild(obj);
        }
    }

    osg::BoundingSphere sphere;
    if (selectedObjs->getNumChildren() > 0) {
        sphere = selectedObjs->getBound();
    }
    else {
        sphere = modeledObjects->getBound();
    }

    // make radius somewhat smaller to zoom in
    double radius = sphere.radius() * 0.8;
    if (radius <= 0.) {
        return;
    }


    osg::Vec3d center = sphere.center();

    osg::Vec3d eye, centerold, up;
    osgGA::StandardManipulator* cm = dynamic_cast<osgGA::StandardManipulator*> (soleViewer->getCameraManipulator());
    if (!cm) {
        LOG(ERROR) << "Incompatible camera manipulator";
        return;
    }

    cm->getTransformation(eye, centerold, up);

    // center camera
    osg::Vec3d displacement = center-centerold;
    cm->setTransformation(eye + displacement, center, up);

    // take correct zoom level
    double ar = screenWidth/screenHeight;
    if (screenWidth > screenHeight) {
        soleViewer->getCamera()->setProjectionMatrixAsOrtho(-radius*ar, radius*ar, -radius, radius, 0, 1000);
    }
    else {
        soleViewer->getCamera()->setProjectionMatrixAsOrtho(-radius, radius, -radius/ar, radius/ar, 0, 1000);
    }
}

void OsgMainApp::mouseButtonPressEvent(float x, float y, int button, int view)
{
    soleViewer->getEventQueue()->mouseButtonPress(x, y, button);
}

void OsgMainApp::mouseButtonReleaseEvent(float x, float y, int button, int view)
{
    soleViewer->getEventQueue()->mouseButtonRelease(x, y, button);
}

void OsgMainApp::mouseMoveEvent(float x, float y, int view)
{
    soleViewer->getEventQueue()->mouseMotion(x, y);
}

void OsgMainApp::displayShape(TopoDS_Shape shape, std::string id)
{
    // The arrow calculation is based on the global min max
    Bnd_Box aBndBox;
    BRepBndLib::Add (shape, aBndBox);
    double aDeflection = tesselationAccu;
    if (!aBndBox.IsVoid()) {
      Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
      aBndBox.Get (aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
      aDeflection = Max (aXmax-aXmin, Max (aYmax-aYmin, aZmax-aZmin))
                       * tesselationAccu;
    }
    try {
        osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
        LOG(WARNING) << "LOAD SHAPE " << id;
        geode->fromShape(shape, aDeflection);
        geode->setName(id);
        modeledObjects->addChild(geode.get());
    }
    catch (...) {
        osg::notify(osg::ALWAYS) << "Error: could not triangularize " << id << std::endl;
    }
}
