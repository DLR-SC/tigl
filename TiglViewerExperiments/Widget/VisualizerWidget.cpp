#include "VisualizerWidget.h"

#include "GeometricVisObject.h"
#include "PickHandler.h"
#include "SelectionBoxHandler.h"
#include "ViewInteractionsHandler.h"
#include "AuxiliaryViewUpdater.h"
#include "MaterialTemplate.h"

#include <iostream>

#include <tigl.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSConfiguration.h>
#include <CCPACSWing.h>
#include <CTiglTriangularizer.h>

#include <QtGui/QGridLayout>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgQt/GraphicsWindowQt>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>


#include "crossnodegeode.h"

//VisualizerWidget is responsible for the visualization. GeometricVisObject and VirtualVisObjects are added here.

VisualizerWidget::VisualizerWidget(QWidget* widget) : QWidget(widget)
{
        setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);

        GroupWindow1 = new osg::Group();
        GroupWindow1->setName("Window 1");
        GroupWindow2 = new osg::Group();
        GroupWindow2->setName("Window 2");
        GroupWindow3 = new osg::Group();
        GroupWindow3->setName("Window 3");
        GroupWindow4 = new osg::Group();
        GroupWindow4->setName("Window 4");

        vvo = new VirtualVisObject();

        GroupWindow1->addChild(vvo);
        GroupWindow1->addChild(vvo->camera);

        GroupWindow2->addChild(vvo);
        GroupWindow2->addChild(vvo->camera);

        GroupWindow3->addChild(vvo);
        GroupWindow3->addChild(vvo->camera);

        GroupWindow4->addChild(vvo);
        GroupWindow4->addChild(vvo->camera);

        pickedNodes = new osg::Group();

        xWP = 200;
        yWP = 200;

        setupGUI();

        connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
        _timer.start( 40 );
    }

//method to add a crossNode to the root of a given scene.
void VisualizerWidget::addCross(osg::ref_ptr<CrossNode>& crossnode,osgViewer::View* view , osg::Group* group)
{
    osg::ref_ptr<CrossNodeGeode> crossgeode = new CrossNodeGeode;
    crossnode = new CrossNode();
    crossnode->setViewport(0.0,0.0,xWP,yWP);
    crossnode->setProjectionMatrix(osg::Matrixd::ortho(-1.5, 1.5, -1.5, 1.5, -10.0, 10.0));
    crossnode->setCrossBody(crossgeode->initNodeGeode());
    crossnode->setMainCamera(view->getCamera());
    crossnode->setRenderOrder( osg::Camera::POST_RENDER );
    crossnode->setClearMask( GL_DEPTH_BUFFER_BIT );
    crossnode->setAllowEventFocus( false );
    crossnode->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    crossnode->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    crossnode->setName("CrossNode");

    group->addChild(crossnode.get());

    osg::LightSource* lightSource = new osg::LightSource;
    lightSource->getLight()->setPosition(osg::Vec4d( 0.0f, 0.0f, 5.f, 1.0f )); // point light
    crossnode->addChild(lightSource);
}

QWidget* VisualizerWidget::create2DView(osg::Camera* camera, osg::Vec3 eye, unsigned i)
{
    osgViewer::View* view = new osgViewer::View;
    view->setCamera(camera);

    addView( view );

    switch(i)
    {
    case(1):
    {
        addCross(crossnode11,view , GroupWindow1);
        view->setSceneData( GroupWindow1 );
        camera->setViewMatrixAsLookAt(eye, osg::Vec3(0,0,0),osg::Vec3(0,0,1));
        break;
    }
    case(2):
    {
        addCross(crossnode12,view , GroupWindow2);
        view->setSceneData( GroupWindow2 );
        camera->setViewMatrixAsLookAt(eye, osg::Vec3(0,0,0),osg::Vec3(0,0,1));
        break;
    }
    case(3):
    {
        addCross(crossnode21,view , GroupWindow3);
        view->setSceneData( GroupWindow3 );
        camera->setViewMatrixAsLookAt(eye, osg::Vec3(0,0,0),osg::Vec3(0,1,0));
        break;
    }
    default:
        break;
    }

    view->addEventHandler( new osgViewer::StatsHandler );
    view->addEventHandler(new PickHandler());
    view->addEventHandler(new ViewInteractionsHandler());
    view->addEventHandler(new AuxiliaryViewUpdater());



    osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
    return gw ? gw->getGLWidget() : NULL;
}

QWidget* VisualizerWidget::addViewWidget(osg::Camera* camera, osg::Vec3 eye){
        osgViewer::View* view = new osgViewer::View;
        view->setCamera( camera );
        addView(view);


       addCross(crossnode22,view , GroupWindow4);
       view->setSceneData( GroupWindow4 );




        view->addEventHandler( new osgViewer::StatsHandler );
        view->addEventHandler(new PickHandler());
        view->addEventHandler(new ViewInteractionsHandler());

        //view->addEventHandler(new SelectionBoxHandler());

        osgGA::TrackballManipulator* tm = new osgGA::TrackballManipulator();

        tm->setHomePosition(eye, osg::Vec3(0,0,0), osg::Vec3(0,0,1));

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
		
        camera->setClearColor( osg::Vec4(0.5, 0.8, 0.98, 1.0) );
        camera->setViewport( new osg::Viewport(0, 0, w, h) );
        //camera->setProjectionMatrixAsPerspective(
        //     60.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
        camera->setProjectionMatrixAsOrtho(-(static_cast<double>(traits->width))/2, static_cast<double>(traits->width)/2,
                                           -(static_cast<double>(traits->height))/2, static_cast<double>(traits->height)/2,
                                           -100.0, 100.0);


        return camera.release();
    }


void VisualizerWidget::addObject(char* filename, char* objectName)
{
    osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject(filename, objectName);
//	root->addChild(geode.get());

}


void VisualizerWidget::addObject()
{

    TixiDocumentHandle handle = -1;
    TiglCPACSConfigurationHandle tiglHandle = -1;

    if(tixiOpenDocument( "CPACS_21_D150.xml", &handle ) != SUCCESS){
        std::cout << "Error reading in plane" << std::endl;
    }
    tiglOpenCPACSConfiguration(handle, "", &tiglHandle);

    tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);

    tigl::CTiglTriangularizer t(wing.GetLoftWithLeadingEdge(), 0.01);

    t.switchObject(1);

    std::cout << "Number of points: " << t.currentObject().getNVertices() << std::endl;
        osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject(t, "wing4");
        GroupWindow1->addChild(geode.get());
        GroupWindow2->addChild(geode.get());
        GroupWindow3->addChild(geode.get());
        GroupWindow4->addChild(geode.get());

}
	

void VisualizerWidget::unpickNodes()
{
    for(unsigned int i = 0; i < this->pickedNodes->getNumChildren(); i++){
        GeometricVisObject* geode = (GeometricVisObject*) this->pickedNodes->getChild(i);
        geode->unpick();
    }
		
    this->pickedNodes->removeChildren(0,this->pickedNodes->getNumChildren());

}


void VisualizerWidget::printPickedNodes()
{
    std::cout<<"Print Picked Nodes"<<std::endl;
    for(unsigned int i  = 0 ; i < this->getPickedNodes()->getNumChildren() ; i++)
        std::cout << "das ist Picked: " << this->getPickedNodes()->getChild(i)->getName()<< std::endl;
}



void VisualizerWidget::setupGUI()
{

    QWidget* widget0 =  addViewWidget( createCamera(0,0,100,100), osg::Vec3(150,150,150));
    WidgetList[0] = widget0;
    QWidget* widget1 = create2DView(createCamera(0,0,100,100),osg::Vec3(-150,0,0),1);
    WidgetList[1] = widget1;
    QWidget* widget2 = create2DView(createCamera(0,0,100,100),osg::Vec3(0,150,0),2);
    WidgetList[2] = widget2;
    QWidget* widget3 = create2DView(createCamera(0,0,100,100),osg::Vec3(0,0,150),3);
    WidgetList[3] = widget3;





    grid = new QGridLayout(this);
    grid->addWidget( widget0, 0, 0 );
    grid->addWidget( widget1, 0, 1 );
    grid->addWidget( widget2, 1, 0 );
    grid->addWidget( widget3, 1, 1 );
}



//Set the ViewPort of the CrossNode once the windown is resized.

void VisualizerWidget::resizeEvent(QResizeEvent * event)
{

    int x = WidgetList[0]->size().width();
    int y = WidgetList[0]->size().height();

    xWP = x/4;
    yWP = y/4;



    if( x > 0 && y > 0){

    if(crossnode11.valid()) crossnode11->setViewport(0,0,std::max(xWP,yWP),std::max(xWP,yWP));
    if(crossnode12.valid()) crossnode12->setViewport(0,0,std::max(xWP,yWP),std::max(xWP,yWP));
    if(crossnode21.valid()) crossnode21->setViewport(0,0,std::max(xWP,yWP),std::max(xWP,yWP));
    if(crossnode22.valid()) crossnode22->setViewport(0,0,std::max(xWP,yWP),std::max(xWP,yWP));
   }


}
