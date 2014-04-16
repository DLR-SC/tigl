#include "OsgMainApp.hpp"
#include "AuxiliaryViewUpdater.h"
#include "TiglViewerBackground.h"
#include "MaterialTemplate.h"

#include <sstream>
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

#define HOME_POS 200

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
    screenHeight = (float) height;
    screenWidth = (float) width;
    float ar = screenWidth/ screenHeight;


    if (soleViewer) {
        // the window changed (resize)
        soleViewer->setUpViewerAsEmbeddedInWindow(x, y, width, height);
        soleViewer->getEventQueue()->setMouseInputRange(x, y, x + width, y + height);
        float viewArea = 50;
        soleViewer->getCamera()->setProjectionMatrixAsOrtho(-viewArea/2., viewArea/2., -viewArea/2. / ar, viewArea/2. / ar, -100, 200);
        return;
    }

    osg::notify(osg::ALWAYS) << "create viewer";

    soleViewer = new osgViewer::Viewer();
    osgViewer::GraphicsWindowEmbedded* window = soleViewer->setUpViewerAsEmbeddedInWindow(x, y, width, height);

    soleViewer->getEventQueue()->setMouseInputRange(x, y, x + width, y + height);
    soleViewer->getCamera()->setClearColor(osg::Vec4(0, 0, 0, 0));
    //soleViewer->getCamera()->setProjectionMatrixAsPerspective(20.0, width / (double) height, 10, 1000);
    float zoomLevel = 0.3;
    soleViewer->getCamera()->setProjectionMatrixAsOrtho(-screenWidth/2.*zoomLevel, screenWidth/2. * zoomLevel, -screenHeight/2. * zoomLevel, screenHeight/2. * zoomLevel, 0, 1000);

    //tm = new osgGA::TrackballManipulator();
    osg::ref_ptr<OrthoManipulator> tm = new OrthoManipulator(soleViewer->getCamera());
    tm->setHomePosition(osg::Vec3(-HOME_POS, -HOME_POS, HOME_POS), osg::Vec3(0, 0, 0), osg::Vec3(1, 1 , 1));

    soleViewer->addEventHandler(new osgViewer::StatsHandler);
    soleViewer->addEventHandler(new osgGA::StateSetManipulator(soleViewer->getCamera()->getOrCreateStateSet()));
    soleViewer->addEventHandler(new osgViewer::ThreadingHandler);
    soleViewer->addEventHandler(new osgViewer::LODScaleHandler);

    soleViewer->setCameraManipulator(tm);
    soleViewer->realize();

    createScene();

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
//Events
void OsgMainApp::addObjectFromVTK(std::string filepath)
{
    osg::notify(osg::ALWAYS) << "Opening VTK file " << filepath << std::endl;

    osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
    geode->readVTK(filepath.c_str());
    geode->setName(filepath);

    modeledObjects->addChild(geode.get());
    fitScreen();
}

void OsgMainApp::addObjectFromHOTSOSE(std::string filepath)
{
    osg::notify(osg::ALWAYS) << "Opening Hotsose file " << filepath << std::endl;

    osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
    geode->readHotsoseMesh(filepath.c_str());
    geode->setName(filepath);

    modeledObjects->addChild(geode.get());
    fitScreen();
}

void OsgMainApp::addObjectFromCPACS(std::string filepath)
{
    osg::notify(osg::ALWAYS) << "Opening CPACS file " << filepath << std::endl;
    TixiDocumentHandle handle = -1;
    TiglCPACSConfigurationHandle tiglHandle = -1;

    if (tixiOpenDocument(filepath.c_str(), &handle) != SUCCESS) {
        return;
    }

    if (tiglOpenCPACSConfiguration(handle, "", &tiglHandle) != TIGL_SUCCESS) {
        osg::notify(osg::ALWAYS) << "Error opening cpacs file " << filepath << std::endl;
    }

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);

    double tesselationAccu = 0.01;

    for (int iWing = 1; iWing <= config.GetWingCount(); ++iWing) {
        tigl::CCPACSWing& wing = config.GetWing(iWing);

        for (int iSegment = 1; iSegment <= wing.GetSegmentCount(); ++iSegment) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(iSegment);
            osg::notify(osg::ALWAYS) << "Computing " << segment.GetUID() << std::endl;

            osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
            geode->fromShape(segment.GetLoft(), tesselationAccu);
            geode->setName(segment.GetUID());
            modeledObjects->addChild(geode.get());
        }

        if (wing.GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
            continue;
        }

        for (int i = 1; i <= wing.GetSegmentCount(); i++) {
            tigl::CCPACSWingSegment& segment = (tigl::CCPACSWingSegment &) wing.GetSegment(i);
            TopoDS_Shape loft = segment.GetMirroredLoft();

            osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
            geode->fromShape(loft, tesselationAccu);
            geode->setName(segment.GetUID() + "_mirrored");
            modeledObjects->addChild(geode.get());
        }

    }

    for (int f = 1; f <= config.GetFuselageCount(); f++) {
        tigl::CCPACSFuselage& fuselage = config.GetFuselage(f);

        for (int i = 1; i <= fuselage.GetSegmentCount(); i++) {
            tigl::CCPACSFuselageSegment& segment = (tigl::CCPACSFuselageSegment &) fuselage.GetSegment(i);
            TopoDS_Shape loft = segment.GetLoft();

            // Transform by fuselage transformation
            loft = fuselage.GetFuselageTransformation().Transform(loft);
            osg::notify(osg::ALWAYS) << "Computing " << segment.GetUID() << std::endl;
            try {
                tigl::CTiglTriangularizer t(loft, tesselationAccu);

                osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
                geode->fromShape(loft, tesselationAccu);
                geode->setName(segment.GetUID());
                modeledObjects->addChild(geode.get());
            }
            catch (...) {
                osg::notify(osg::ALWAYS) << "Error: could not triangularize fuselage segment " << i << std::endl;
            }
        }
    }
    fitScreen();
}

void OsgMainApp::removeObjects()
{
    std::map<std::string, GeometricVisObject*>::iterator soi = selectedObjects.begin();
    if (selectedObjects.size() == 0) {
        modeledObjects->removeChildren(0, root_1->getNumChildren());
    }
    else {
        for (soi = selectedObjects.begin(); soi != selectedObjects.end(); ++soi) {

            modeledObjects->removeChild(soi->second);
            osg::notify(osg::ALWAYS) << soi->first << std::endl;
        }
        selectedObjects.clear();
    }

}

void OsgMainApp::changeCamera(int view)
{
    if(!soleViewer) {
        return;
    }

    osgGA::CameraManipulator* m = new OrthoManipulator(soleViewer->getCamera());
    if(!m) {
        return;
    }
    /*pres*/
    if (view == 0) {
        // perspective
        m->setHomePosition(osg::Vec3(-HOME_POS, -HOME_POS, HOME_POS), osg::Vec3(20, 0, 0), osg::Vec3(1, 1, 1));
    }
    else if (view == 1) {
        // top
        m->setHomePosition(osg::Vec3(20, 0, HOME_POS), osg::Vec3(20, 0, 0), osg::Vec3(0, 1, 0));
    }
    else if (view == 2) {
        // side
        m->setHomePosition(osg::Vec3(20, -HOME_POS, 0), osg::Vec3(20, 0, 0), osg::Vec3(0, 0, 1));
    }
    else if (view == 3) {
        // front
        m->setHomePosition(osg::Vec3(-HOME_POS, 0, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));
    }
    soleViewer->setCameraManipulator(m);

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


    double radius = sphere.radius() * 1.4;
    if (radius <= 0.) {
        return;
    }

    osg::Vec3d center = sphere.center();

    osg::Vec3d eye, centerold, up;
    osgGA::StandardManipulator* cm = dynamic_cast<osgGA::StandardManipulator*> (soleViewer->getCameraManipulator());
    if(!cm) {
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
        soleViewer->getCamera()->setProjectionMatrixAsOrtho(-radius*ar/2., radius*ar/2., -radius/2., radius/2., 0, 1000);
    }
    else {
        soleViewer->getCamera()->setProjectionMatrixAsOrtho(-radius/2, radius/2., -radius/(2.*ar), radius/(2.*ar), 0, 1000);
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


void OsgMainApp::pickEvent(float x, float y, int /* view */)
{
    // map pixel coordinates to [-1,1] (OpenGL Screen Coordinates)
    float xwindow =  x/screenWidth  * 2. - 1;
    float ywindow = -y/screenHeight * 2. + 1;

    if (!soleViewer || !soleViewer->getCamera()) {
        return;
    }
    osg::Camera* cam = soleViewer->getCamera();

    osg::Matrixd m;
    m.preMult(cam->getProjectionMatrix());
    m.preMult(cam->getViewMatrix());

    // define intersection ray
    osg::Vec3d startPoint (xwindow, ywindow, -1000);
    osg::Vec3d endPoint(xwindow, ywindow, 1000);

    osg::Matrixd i;
    i.invert(m);

    osg::Vec3d wStart =  startPoint * i;
    osg::Vec3d wEnd   =  endPoint   * i;

    bool rayHit = false;

    osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = new osgUtil::LineSegmentIntersector(wStart, wEnd);
    osgUtil::IntersectionVisitor iv(picker.get());
    soleViewer.get()->getCamera()->accept(iv);
    iv.setTraversalMask(~0x1);

    if (picker->containsIntersections()) {

        osgUtil::LineSegmentIntersector::Intersections allIntersections = picker->getIntersections();
        osgUtil::LineSegmentIntersector::Intersections::iterator intersectionsIterator = allIntersections.begin();
        osg::notify(osg::ALWAYS) << "Number of Intersections " << intersectionsIterator->nodePath.size() << std::endl;

        GeometricVisObject* pickedObject;
        std::string nameOfpickedObject;

        for (int i = 0; i < intersectionsIterator->nodePath.size(); i++) {

            pickedObject = (GeometricVisObject*) intersectionsIterator->nodePath.at(i);
            nameOfpickedObject = intersectionsIterator->nodePath.at(i)->getName();

            if (!(intersectionsIterator->nodePath.at(i)->getName().empty())) {
                if (!pickedObject->isPicked()) {
                    pickedObject->pick();
                    selectedObjects.insert(std::pair<std::string, GeometricVisObject*>(nameOfpickedObject, pickedObject));
                    rayHit = true;
                }
                else {
                    pickedObject->unpick();
                    selectedObjects.erase(nameOfpickedObject);
                    rayHit = true;
                }
            }

        }
    }

}
