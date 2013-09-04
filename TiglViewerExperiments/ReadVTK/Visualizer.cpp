#include "Visualizer.h"
#include "PickHandler.h"
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/View>
#include <osg/Array>
#include <osg/Depth>
#include "MaterialTemplate.h"
#include "CrossNode.h"
#include "crossnodegeode.h"
//#include <osgQt/GraphicsWindowQt>

//osg::ref_ptr<osg::Group> root = new osg::Group();



void addCross(osgViewer::View* view , osg::Group* group ,
                          int x, int y, int w, int h)
{
    osg::ref_ptr<CrossNodeGeode> crossgeode = new CrossNodeGeode;
    CrossNode * crossnode = new CrossNode();
    crossnode->setViewport(y,y,h,h);
    crossnode->setProjectionMatrix(osg::Matrixd::ortho(-1.5, 1.5, -1.5, 1.5, -10.0, 10.0));
    crossnode->setCrossBody(crossgeode->initNodeGeode());
    crossnode->setMainCamera(view->getCamera());
    crossnode->setRenderOrder( osg::Camera::POST_RENDER, 1 );
    crossnode->setClearMask( GL_DEPTH_BUFFER_BIT );
    crossnode->setAllowEventFocus( false );
    crossnode->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    crossnode->setName("CrossNode");

    group->addChild(crossnode);

    osg::LightSource* lightSource = new osg::LightSource;
    lightSource->getLight()->setPosition(osg::Vec4d( 0.0f, 0.0f, 5.f, 1.0f )); // point light
    crossnode->addChild(lightSource);
}

osg::Node * addBGNode(const osg::Vec4& downCol, const osg::Vec4& upCol){
    osg::Vec3Array *vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0,0,0));
    vertices->push_back(osg::Vec3(1,0,0));
    vertices->push_back(osg::Vec3(1,1,0));
    vertices->push_back(osg::Vec3(0,1,0));

    osg::Vec4Array *  colors = new osg::Vec4Array;
    colors->push_back( downCol );
    colors->push_back( downCol );
    colors->push_back( upCol );
    colors->push_back( upCol );

    osg::DrawArrays* array = new osg::DrawArrays(osg::PrimitiveSet::QUADS , 0 , vertices->size());

    osg::Geometry * geometry = new osg::Geometry;
    geometry->addPrimitiveSet(array);
    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::Geode * bgnode = new osg::Geode;
    bgnode->addDrawable(geometry);

    osg::Camera* camera = new osg::Camera;
    camera->setCullingActive( false );
    camera->setClearMask( 0 );
    camera->setAllowEventFocus( false );
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setRenderOrder( osg::Camera::POST_RENDER, 0 );
    camera->setProjectionMatrix( osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0) );
    camera->addChild( bgnode );

    osg::StateSet* ss = camera->getOrCreateStateSet();
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    ss->setAttributeAndModes( new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0) );

    return camera;
}

Visualizer::Visualizer(int posX, int posY, int width, int height)
{
	views.push_back(new osgViewer::View());
	views[0]->setUpViewInWindow(posX, posY, width, height, 0);
	views.push_back(new osgViewer::View());
	//osgQt::GraphicsWindowQt* gw = new osgQt::GraphicsWindowQt();

    double R1 = 1.;
    double G1 = 235/255.;
    double B1 = 163/255.;
    double fu = 2.;
    double fd = 0.2;

    osg::Vec4 up  (R1*fu > 1 ? 1. : R1*fu, G1*fu > 1 ? 1. : G1*fu, B1*fu > 1 ? 1. : B1*fu, 1.);
    osg::Vec4 down(R1*fd > 1 ? 1. : R1*fd, G1*fd > 1 ? 1. : G1*fd, B1*fd > 1 ? 1. : B1*fd, 1.);

	root = new osg::Group();
	vvo = new VirtualVisObject();
    vvo->setAxes(false);
    root->addChild(vvo->main.get());
    addCross(views[0], root.get() , 0. , 0. , width/4., height/4.);
    root->addChild(addBGNode(down, up));

	pickedNodes = new osg::Group();
	pickedNodes->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(1));
}


Visualizer::~Visualizer(void)
{
}
void Visualizer::addObject(char* filename, char* objectName)
{
	osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject(filename, objectName);
	root->addChild(geode.get());
}

void Visualizer::start(){

	osg::ref_ptr<osgGA::TrackballManipulator> tm = new osgGA::TrackballManipulator();

	osg::Vec3d* tmEye = new osg::Vec3d(0,50,50);
	osg::Vec3d* tmCenter = new osg::Vec3d(0,0,0);
	osg::Vec3d* tmUp = new osg::Vec3d(0,0,1);


	tm->setHomePosition(*tmEye, *tmCenter, *tmUp);

	//this->setSceneData(root.get());
	views[0]->setSceneData(root);
	views[0]->setCameraManipulator(tm);
	views[0]->addEventHandler(new PickHandler());

	views[1]->setSceneData(root);
	views[1]->setCameraManipulator(tm);
	views[1]->addEventHandler(new PickHandler());
	//views[0]->addEventHandler(new SelectionBoxHandler());

	osgViewer::View * actView = views[0];
	this->addView(actView);
	//this->addView(views[1]);

	//this->setCameraManipulator(tm);

	this->run();

}

void Visualizer::unpickNodes(){
	for(unsigned int i = 0; i < this->pickedNodes->getNumChildren(); i++){
		GeometricVisObject* geode = (GeometricVisObject*) this->pickedNodes->getChild(i);
		geode->unpick();
	}
		
	this->pickedNodes->removeChildren(0,this->pickedNodes->getNumChildren());

}

void Visualizer::showCross(){
}

void Visualizer::showXYGrid(){

}
