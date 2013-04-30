#include "Visualizer.h"
#include "PickHandler.h"
#include <osgGA/TrackballManipulator>
#include "MaterialTemplate.h"
//#include <osgQt/GraphicsWindowQt>

//osg::ref_ptr<osg::Group> root = new osg::Group();



Visualizer::Visualizer(int posX, int posY, int width, int height)
{
	views.push_back(new osgViewer::View());
	views[0]->setUpViewInWindow(posX, posY, width, height, 0);
	views.push_back(new osgViewer::View());
	//osgQt::GraphicsWindowQt* gw = new osgQt::GraphicsWindowQt();

	
	root = new osg::Group();
	vvo = new VirtualVisObject();
	root->addChild(vvo);

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
