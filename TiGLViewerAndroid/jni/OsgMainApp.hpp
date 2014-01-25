/*
 * OsgMainApp.hpp
 *
 *  Created on: 29/05/2011
 *      Author: Jorge Izquierdo Ciges
 */

#ifndef OSGMAINAPP_HPP_
#define OSGMAINAPP_HPP_

//Android log
#include <android/log.h>
#include <android/asset_manager.h>
#include <iostream>
#include <cstdlib>
#include <math.h>

//Standard libraries
#include <string>

#include "CSharedPtr.h"
#include "TiglAndroidLogger.h"

//osg
#include <osg/GL>
#include <osg/GLExtensions>
#include <osg/Depth>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Node>
#include <osg/Notify>

//osgDB
#include <osgDB/DatabasePager>
#include <osgDB/Registry>

//osg_viewer
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osgViewer/ViewerBase>
#include <osgViewer/CompositeViewer>
#include <osg/Camera>

#include <osgViewer/Renderer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/GraphicsWindow>
#include <osg/GraphicsContext>

//osgGA
#include <osgGA/GUIEventAdapter>
#include <osgGA/MultiTouchTrackballManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

//Self headers
#include "OsgAndroidNotifyHandler.hpp"
#include "VirtualVisObject.hpp"
#include "TiglViewerHUD.h"


USE_OSGPLUGIN(osg)
USE_OSGPLUGIN(osg2)
USE_OSGPLUGIN(freetype)

//Static DOTOSG
USE_DOTOSGWRAPPER_LIBRARY(osg)
USE_DOTOSGWRAPPER_LIBRARY(osgViewer)

#define  LOG_TAG    "osgNativeLib"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

struct Model{
    std::string filename;
    std::string name;
};

class OsgMainApp
{

public:
    static OsgMainApp& Instance();

    ~OsgMainApp();

    void init();

    void setAssetManager(AAssetManager* mgr);
    AAssetManager * getAssetManager();

    //Initialization function
    void initOsgWindow(int x,int y, int width, int height);
    //Draw
    void createScene();

    void draw();
    //Events
    void currentCamera();
    void addObjectFromVTK(std::string filepath);
    void addObjectFromCPACS(std::string filepath);
    void removeObjects();
    void changeCamera(int view);
    void mouseButtonPressEvent(float x,float y,int button , int view);
    void mouseButtonReleaseEvent(float x,float y,int button , int view);
    void mouseMoveEvent(float x,float y, int view);

private:
	/*
	 * * Commented code is used for adding multiple views in the same viewer
	 */
	osg::ref_ptr<osgViewer::Viewer> soleViewer;
	//osg::ref_ptr<osgViewer::CompositeViewer> cviewer;
	//osgViewer::View* _viewer;
	osg::ref_ptr<osg::Group> root_1;
	//osg::ref_ptr<osg::Group> root_2;
	//osg::ref_ptr<osg::Group> root_3;
	//osg::ref_ptr<osg::Group> root_4;
	osg::ref_ptr<TiglViewerHUD> crossnode1;
	//osg::ref_ptr<CrossNode> crossnode2;
	//osg::ref_ptr<CrossNode> crossnode3;
	//osg::ref_ptr<CrossNode> crossnode4;
	osg::ref_ptr<VirtualVisObject> _coordinateGrid;
	osg::ref_ptr<osg::StateSet> _state;
	osg::ref_ptr<osgGA::TrackballManipulator> tm;

    float _lodScale;
    unsigned int _prevFrame;

    bool _initialized;
    bool _clean_scene;
    int counter;
    int viewSelected;
    float screenHeight;
    float screenWidth;

    OsgAndroidNotifyHandler *_notifyHandler;
    AAssetManager*           _assetManager;
    CSharedPtr<TiglAndroidLogger> _logAdapter;

    std::vector<Model> _vModels;
    std::vector<Model> _vModelsToLoad;
    std::vector<Model> _vModelsToDelete;

   // osgViewer::View* createView(int x, int y, int height, int width , osgViewer::GraphicsWindowEmbedded* _gwe , int id);
    osg::Node* addCross(osg::ref_ptr<osgViewer::View> view, int x, int y, int size);
    OsgMainApp();

};


#endif /* OSGMAINAPP_HPP_ */
