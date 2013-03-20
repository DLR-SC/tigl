#include "VisualizerWidget.h"
#include "PickHandler.h"
#include "SelectionBoxHandler.h"
#include <osgGA/TrackballManipulator>
#include "MaterialTemplate.h"
#include <iostream>

//VisualizerWidget is responsible for the visualization. GeometricVisObject and VirtualVisObjects are added here.

VisualizerWidget::VisualizerWidget(QWidget* widget) : QWidget(widget)
{
		setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);

		root = new osg::Group();
		vvo = new VirtualVisObject();

		root->addChild(vvo);
		root->addChild(vvo->camera);

		pickedNodes = new osg::Group();

		setupGUI();

		connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
		_timer.start( 10 );
	}

//Visualizer Widget contains 4 ViewWidgets with different perspectives of the scene

QWidget* VisualizerWidget::addViewWidget(osg::Camera* camera, osg::Vec3 eye){
		osgViewer::View* view = new osgViewer::View;
		view->setCamera( camera );
		addView( view );
		
		view->setSceneData( root );
		view->addEventHandler( new osgViewer::StatsHandler );
		view->addEventHandler(new PickHandler());
		//view->addEventHandler(new SelectionBoxHandler());

		osgGA::TrackballManipulator* tm = new osgGA::TrackballManipulator();

		osg::Vec3 center(17.4f,8.5f, -1);//compute center here
		osg::Vec3 relEye(eye.operator+(center));


		tm->setHomePosition(eye, center, osg::Vec3(0,0,1));

		view->setCameraManipulator( tm );
		
		osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
		return gw ? gw->getGLWidget() : NULL;
	}

osg::Camera* VisualizerWidget::createCamera( int x, int y, int w, int h, const std::string& name, bool windowDecoration)
	{
		osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
		osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
		traits->windowName = name;
		traits->windowDecoration = windowDecoration;
		traits->x = x;
		traits->y = y;
		traits->width = w;
		traits->height = h;
		traits->doubleBuffer = true;
		traits->alpha = ds->getMinimumNumAlphaBits();
		traits->stencil = ds->getMinimumNumStencilBits();
		traits->sampleBuffers = ds->getMultiSamples();
		traits->samples = ds->getNumMultiSamples();
		
		osg::ref_ptr<osg::Camera> camera = new osg::Camera;
		camera->setGraphicsContext( new osgQt::GraphicsWindowQt(traits.get()) );
		
		camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
		camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
		camera->setProjectionMatrixAsPerspective(
			30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
		return camera.release();
	}

//Adds GeometricVisObject to the scene. 

/*void VisualizerWidget::addObject(tigl::CTiglPolyData& inputObject, char* objectName)
{

	osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject(inputObject, objectName);
	root->addChild(geode.get());

}*/

//Adds GeometricVisObject read from a VTK-File

void VisualizerWidget::addObject(char* filename, char* objectName)
{
	osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject(filename, objectName);
	root->addChild(geode.get());

}
	
	
void VisualizerWidget::unpickNodes(){
	for(unsigned int i = 0; i < this->pickedNodes->getNumChildren(); i++){
		GeometricVisObject* geode = (GeometricVisObject*) this->pickedNodes->getChild(i);
		geode->unpick();
	}
		
	this->pickedNodes->removeChildren(0,this->pickedNodes->getNumChildren());

}

void VisualizerWidget::setupGUI(){
	QWidget* widget1 = addViewWidget( createCamera(0,0,100,100), osg::Vec3(50,0,0));
	QWidget* widget2 = addViewWidget( createCamera(0,0,100,100), osg::Vec3(50,50,20));
	QWidget* widget3 = addViewWidget( createCamera(0,0,100,100), osg::Vec3(0,0,30));
	QWidget* widget4 = addViewWidget( createCamera(0,0,100,100), osg::Vec3(-50,-50,20));

	grid = new QGridLayout(this);
	grid->addWidget( widget1, 0, 0 );
	grid->addWidget( widget2, 0, 1 );
	grid->addWidget( widget3, 1, 0 );
	grid->addWidget( widget4, 1, 1 );
	

	
}