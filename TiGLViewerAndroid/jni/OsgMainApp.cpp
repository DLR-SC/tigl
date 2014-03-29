#include "OsgMainApp.hpp"
#include "AuxiliaryViewUpdater.h"
#include "TiglViewerBackground.h"

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

#define HOME_POS 80

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

    _lodScale = 1.0f;
    _prevFrame = 0;

    _initialized = false;
    _clean_scene = false;
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
    osg::notify(osg::ALWAYS) << "initIsgWindow  called";

    if (soleViewer) {
        // the window changed (resize)
        soleViewer->setUpViewerAsEmbeddedInWindow(x, y, width, height);
        soleViewer->getEventQueue()->setMouseInputRange(x, y, x + width, y + height);
        osg::notify(osg::ALWAYS) << "just resizsing ";
        return;
    }

    osg::notify(osg::ALWAYS) << "create viewer";

    soleViewer = new osgViewer::Viewer();
    soleViewer->setUpViewerAsEmbeddedInWindow(x, y, width, height);

    soleViewer->getEventQueue()->setMouseInputRange(x, y, x + width, y + height);
    //soleViewer->getCamera()->setClearColor( osg::Vec4(98/255. * 1.1 , 166/255. * 1.1 , 1.0 , 0.0) );
    soleViewer->getCamera()->setClearColor(osg::Vec4(0, 0, 0, 0));
    soleViewer->getCamera()->setProjectionMatrixAsPerspective(20.0, width / (double) height, 10, 1000);

    tm = new osgGA::TrackballManipulator();
    tm->setHomePosition(osg::Vec3(-HOME_POS, -HOME_POS, HOME_POS), osg::Vec3(20, 0, 0), osg::Vec3(0, 0, 1));

    // cviewer = new osgViewer::CompositeViewer();
    //osgViewer::GraphicsWindowEmbedded* _gwe = new osgViewer::GraphicsWindowEmbedded(x,y,width,height);

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
    // root_2 = new osg::Group();
    // root_3 = new osg::Group();
    // root_4 = new osg::Group();

    // cviewer->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);

    _state = root_1->getOrCreateStateSet();
    _state->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    _state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    _state->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

    // cviewer->addView(createView(x,height/2,width/2,height/2,_gwe,1));
    // cviewer->addView(createView(x,y,width/2,height/2,_gwe,2));
    // cviewer->addView(createView(width/2,y,width/2,height/2,_gwe,4));
    // cviewer->addView(createView(width/2,height/2,width/2,height/2,_gwe,3));

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
    // root_2->addChild(vvo->main.get());
    // root_3->addChild(vvo->main.get());
    // root_4->addChild(vvo->main.get());

    // add background as gradient
    osg::ref_ptr<TiglViewerBackground> bg = new TiglViewerBackground;
    bg->makeGradient(osg::Vec4(0.6, 0.6, 1., 1.));
    root_1->addChild(bg);

    soleViewer->setSceneData(root_1.get());
}

/*
 osgViewer::View* OsgMainApp::createView(int x, int y, int width, int height
 ,osgViewer::GraphicsWindowEmbedded* _gwe, int id)
 {
 osgViewer::View* _viewer = new osgViewer::View;

 osgGA::TrackballManipulator* tm = new osgGA::TrackballManipulator();
 osgGA::TrackballManipulator* tm1 = new osgGA::TrackballManipulator();
 osgGA::TrackballManipulator* tm2 = new osgGA::TrackballManipulator();
 osgGA::TrackballManipulator* tm3 = new osgGA::TrackballManipulator();
 tm->setHomePosition(osg::Vec3(300,300,300), osg::Vec3(0,0,0), osg::Vec3(0,0,1));
 tm1->setHomePosition(osg::Vec3(0,300,0), osg::Vec3(0,0,0), osg::Vec3(0,0,1));
 tm2->setHomePosition(osg::Vec3(-300,0,0), osg::Vec3(0,0,0), osg::Vec3(0,0,1));
 tm3->setHomePosition(osg::Vec3(0,0,300), osg::Vec3(0,0,0), osg::Vec3(0,0,1));

 osg::ref_ptr<osg::Camera> _camera = _viewer->getCamera();
 _camera->setViewport(new osg::Viewport(x,y,width,height));
 _camera->setGraphicsContext(_gwe);

 switch(id)
 {
 case(1):
 {
 addCross(crossnode1,_viewer, root_1.get() , x , y , width/4, height/4);
 _viewer->setSceneData(root_1.get());
 _state = root_1->getOrCreateStateSet();
 //_camera->setViewMatrixAsLookAt(osg::Vec3(300,300,300), osg::Vec3(0,0,0), osg::Vec3(0,0,1));
 _viewer->setCameraManipulator( tm );
 break;
 }
 case(2):
 {
 addCross(crossnode2,_viewer, root_2.get(), x , y , width/4, height/4);
 _viewer->setSceneData(root_2.get());
 _state = root_2->getOrCreateStateSet();
 _camera->setViewMatrixAsLookAt(osg::Vec3(-300,0,0), osg::Vec3(0,0,0),osg::Vec3(0,0,1));
 _viewer->addEventHandler(new AuxiliaryViewUpdater());
 break;
 }
 case(3):
 {
 addCross(crossnode3,_viewer, root_3.get(), x , y , width/4, height/4);
 _viewer->setSceneData(root_3.get());
 _state = root_3->getOrCreateStateSet();
 _camera->setViewMatrixAsLookAt(osg::Vec3(0,300,0), osg::Vec3(0,0,0),osg::Vec3(0,0,1));
 _viewer->addEventHandler(new AuxiliaryViewUpdater());
 break;
 }
 case(4):
 {
 addCross(crossnode4,_viewer, root_4.get(), x , y , width/4, height/4);
 _viewer->setSceneData(root_4.get());
 _state = root_4->getOrCreateStateSet();
 _camera->setViewMatrixAsLookAt(osg::Vec3(0,0,300), osg::Vec3(0,0,0),osg::Vec3(0,1,0));
 _viewer->addEventHandler(new AuxiliaryViewUpdater());
 break;
 }

 }

 _state->setMode(GL_LIGHTING, osg::StateAttribute::ON);
 _state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
 _state->setMode(GL_CULL_FACE, osg::StateAttribute::ON);


 _viewer->addEventHandler(new osgViewer::StatsHandler);
 _viewer->addEventHandler(new osgGA::StateSetManipulator(_viewer->getCamera()->getOrCreateStateSet()));
 _viewer->addEventHandler(new osgViewer::ThreadingHandler);
 _viewer->addEventHandler(new osgViewer::LODScaleHandler);

 return _viewer;

 }
 */
//Draw
void OsgMainApp::draw()
{
    currentCamera();
    soleViewer->frame();
}
//Events
void OsgMainApp::addObjectFromVTK(std::string filepath)
{
    osg::notify(osg::ALWAYS) << "Opening VTK file " << filepath << std::endl;

    osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
    geode->readVTK(filepath.c_str());
    geode->setName(filepath);

    root_1->addChild(geode.get());
}

void OsgMainApp::addObjectFromHOTSOSE(std::string filepath)
{
    osg::notify(osg::ALWAYS) << "Opening Hotsose file " << filepath << std::endl;

    osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject;
    geode->readHotsoseMesh(filepath.c_str());
    geode->setName(filepath);

    root_1->addChild(geode.get());
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
            root_1->addChild(geode.get());
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
            root_1->addChild(geode.get());
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
                root_1->addChild(geode.get());
            }
            catch (...) {
                osg::notify(osg::ALWAYS) << "Error: could not triangularize fuselage segment " << i << std::endl;
            }
        }
    }
}

void OsgMainApp::removeObjects()
{
    std::map<std::string, GeometricVisObject*>::iterator soi = selectedObjects.begin();
    if (selectedObjects.size() == 0) {
        root_1->removeChildren(3, root_1->getNumChildren());
    }
    else {
        for (soi = selectedObjects.begin(); soi != selectedObjects.end(); ++soi) {

            root_1->removeChild(soi->second);
            osg::notify(osg::ALWAYS) << soi->first << std::endl;
        }
        selectedObjects.clear();
    }

}

void OsgMainApp::currentCamera()
{

    if (viewSelected == 0) {
        osgGA::TrackballManipulator* tm1 = new osgGA::TrackballManipulator();
        tm1->setHomePosition(osg::Vec3(-HOME_POS, -HOME_POS, HOME_POS), osg::Vec3(20, 0, 0), osg::Vec3(0, 0, 1));
        soleViewer->setCameraManipulator(tm1);

        viewSelected = -1;
    }
    else if (viewSelected == 1) {
        osgGA::TrackballManipulator* tm1 = new osgGA::TrackballManipulator();
        tm1->setHomePosition(osg::Vec3(20, 0, HOME_POS), osg::Vec3(20, 0, 0), osg::Vec3(0, 0, 1));
        soleViewer->setCameraManipulator(tm1);

        viewSelected = -1;
    }
    else if (viewSelected == 2) {
        osgGA::TrackballManipulator* tm1 = new osgGA::TrackballManipulator();
        tm1->setHomePosition(osg::Vec3(20, -HOME_POS, 0), osg::Vec3(20, 0, 0), osg::Vec3(0, 0, 1));
        soleViewer->setCameraManipulator(tm1);

        viewSelected = -1;
    }
    else if (viewSelected == 3) {
        osgGA::TrackballManipulator* tm1 = new osgGA::TrackballManipulator();
        tm1->setHomePosition(osg::Vec3(-HOME_POS, 0, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));
        soleViewer->setCameraManipulator(tm1);

        viewSelected = -1;
    }
    else if (viewSelected == 4) {
        osgGA::TrackballManipulator* tm1 = new osgGA::TrackballManipulator();
        tm1->setHomePosition(osg::Vec3(-HOME_POS / 2, -HOME_POS / 2, HOME_POS / 2), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));
        soleViewer->setCameraManipulator(tm1);

        viewSelected = -1;
    }
}

void OsgMainApp::changeCamera(int view)
{
    viewSelected = view;

}

void OsgMainApp::fitScreen()
{
    viewSelected = 4;
}

void OsgMainApp::mouseButtonPressEvent(float x, float y, int button, int view)
{

    soleViewer->getEventQueue()->mouseButtonPress(x, y, button);
//    if(view == 1)
//  cviewer->getView(0)->getEventQueue()->mouseButtonPress(x, y, button);
//  if(view == 2)
//  cviewer->getView(1)->getEventQueue()->mouseButtonPress(x, y, button);
//  if(view == 3)
//  cviewer->getView(2)->getEventQueue()->mouseButtonPress(x, y, button);
//  if(view == 4)
//  cviewer->getView(3)->getEventQueue()->mouseButtonPress(x, y, button);

}
void OsgMainApp::mouseButtonReleaseEvent(float x, float y, int button, int view)
{

    soleViewer->getEventQueue()->mouseButtonRelease(x, y, button);
//  if(view == 1)
//  cviewer->getView(0)->getEventQueue()->mouseButtonRelease(x, y, button);
//  if(view == 2)
//  cviewer->getView(1)->getEventQueue()->mouseButtonRelease(x, y, button);
//  if(view == 3)
//  cviewer->getView(2)->getEventQueue()->mouseButtonRelease(x, y, button);
//  if(view == 4)
//  cviewer->getView(3)->getEventQueue()->mouseButtonRelease(x, y, button);

}
void OsgMainApp::mouseMoveEvent(float x, float y, int view)
{

    soleViewer->getEventQueue()->mouseMotion(x, y);
//  if(view == 1)
//  cviewer->getView(0)->getEventQueue()->mouseMotion(x, y);
//  if(view == 2)
//  cviewer->getView(1)->getEventQueue()->mouseMotion(x, y);
//  if(view == 3)
//  cviewer->getView(2)->getEventQueue()->mouseMotion(x, y);
//  if(view == 4)
//  cviewer->getView(3)->getEventQueue()->mouseMotion(x, y);

}
void OsgMainApp::pickEvent(float x, float y, int density, int view)
{
    int horPixels = 50; //0.11811*density;
    int verPixels = 50; //0.0393701*density;
    osg::notify(osg::ALWAYS) << "Action area " << density << " " << horPixels << " " << verPixels << std::endl;
    osg::notify(osg::ALWAYS) << "Position of Action " << x << " , " << y << std::endl;

    bool rayHit = false;

    osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);
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
            osg::notify(osg::ALWAYS) << "Object \"" << intersectionsIterator->nodePath.back()->getName() << "\"" << std::endl;

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

//	bool intersect = soleViewer.get()->computeIntersections(x,y,intersections);
//	osgUtil::LineSegmentIntersector::Intersections::iterator hit = intersections.begin();
//	osg::notify(osg::ALWAYS)<<"Number of Intersections "<< hit->nodePath.size() << std::endl;

//	if(intersect)
//			{
//				for(int i=0 ; i<hit->nodePath.size() ; i++)
//				{
//					//osg::notify(osg::ALWAYS)<<"Object \""<< hit->nodePath.back()->getName()<<"\""<<std::endl;
//					if(!(hit->nodePath.at(i)->getName().empty()))
//					{
//						//osg::notify(osg::ALWAYS)<<"Object \""<<hit->nodePath.back()->getName()<<"\""<<std::endl;
//
//						//GeometricVisObject* intersectedGeode = (GeometricVisObject*) hit->nodePath.at(i);
//						if(!((GeometricVisObject*) hit->nodePath.at(i))->isPicked())
//						{
//							((GeometricVisObject*) hit->nodePath.at(i))->pick();
//							rayHit = true;
//							//osg::notify(osg::ALWAYS)<<"Object \""<<hit->nodePath.at(i)->getName()<<"\" is Picked"<<std::endl;
//						}
//						else
//						{
//							((GeometricVisObject*) hit->nodePath.at(i))->unpick();
//							rayHit = true;
//							//osg::notify(osg::ALWAYS)<<"Object \""<<hit->nodePath.at(i)->getName()<<"\" is UnPicked"<<std::endl
//						}
//						//rayHit = true;
//						//delete &intersections;
//						//delete &hit;
//						//break;
//					}
//				}
//
//			}
//			if(rayHit){
//				break;
//			}
//		}
//		if(rayHit){
//			rayHit = false;
//			break;
//		}
//	}

}
