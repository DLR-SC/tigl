#include "VisualizerWidget.h"
#include "PickHandler.h"
#include "SelectionBoxHandler.h"
#include <osgGA/TrackballManipulator>
#include "MaterialTemplate.h"
#include <iostream>

VisualizerWidget::VisualizerWidget(QWidget* widget) : QWidget(widget)
{
		setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);

		root = new osg::Group();
		vvo = new VirtualVisObject();
		root->addChild(vvo);


		pickedNodes = new osg::Group();

		setupGUI();

		connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
		_timer.start( 10 );
	}

QWidget* VisualizerWidget::addViewWidget(osg::Camera* camera){
		osgViewer::View* view = new osgViewer::View;
		view->setCamera( camera );
		addView( view );
		
		view->setSceneData( root );
		view->addEventHandler( new osgViewer::StatsHandler );
		view->addEventHandler(new PickHandler());
		view->addEventHandler(new SelectionBoxHandler());
		view->setCameraManipulator( new osgGA::TrackballManipulator );
		
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
	QWidget* widget1 = addViewWidget( createCamera(0,0,100,100));
	QWidget* widget2 = addViewWidget( createCamera(0,0,100,100));
	QWidget* widget3 = addViewWidget( createCamera(0,0,100,100));
	QWidget* widget4 = addViewWidget( createCamera(0,0,100,100));

	grid = new QGridLayout(this);
	grid->addWidget( widget1, 0, 0 );
	grid->addWidget( widget2, 0, 1 );
	grid->addWidget( widget3, 1, 0 );
	grid->addWidget( widget4, 1, 1 );
	

	
}

/*
void VisualizerWidget::checkButtons(){
	if(axisButton->isChecked()!=vvo->axesActive()) vvo->setAxes(axisButton->isChecked());
	if(xyButton->isChecked()!=vvo->xyActive()) vvo->setXYGrid(xyButton->isChecked());
	if(xzButton->isChecked()!=vvo->xzActive()) vvo->setXZGrid(xzButton->isChecked());
	if(yzButton->isChecked()!=vvo->yzActive()) vvo->setYZGrid(yzButton->isChecked());
	if(crossButton->isChecked()!=vvo->crossActive()) vvo->setCross(crossButton->isChecked());
}*/