#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <osgViewer/CompositeViewer>
#include "GeometricVisObject.h"
#include "VirtualVisObject.h"
class Visualizer : public osgViewer::CompositeViewer
{

public:

	Visualizer(int posX, int posY, int width, int height);
	~Visualizer(void);
	void addObject(char* filename, char* objectName);
	void start();
	void showXYGrid();
	void showCross();

	osg::Group* getPickedNodes(){return this->pickedNodes;};

	void addPickedNode(osg::Geode* pickedNode){this->pickedNodes->addChild(pickedNode);};
	void unpickNodes();

private:
	osg::ref_ptr<osg::Group> pickedNodes;
	osg::ref_ptr<osg::Group> root; 
	osg::ref_ptr<VirtualVisObject> vvo;

	
	std::vector<osg::ref_ptr<osgViewer::View>> views;
};


#endif