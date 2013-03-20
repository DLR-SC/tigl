#ifndef VISUALIZER_WIDGET_H
#define VISUALIZER_WIDGET_H
#include <tigl.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSConfiguration.h>
#include <CCPACSWing.h>
#include <CTiglTriangularizer.h>
#include <tixi.h>


#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGridLayout>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QDialog>

#include <iostream>
#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgQt/GraphicsWindowQt>

#include "GeometricVisObject.h"
#include "VirtualVisObject.h"

#include "CTiglPolyData.h"


class VisualizerWidget : public QWidget, public osgViewer::CompositeViewer
{
	Q_OBJECT

public:
    VisualizerWidget(QWidget*);
    

	//Visualizer Widget contains 4 ViewWidgets with different perspectives of the scene
	//Add a viewWidget by calling this function
    QWidget* addViewWidget( osg::Camera* camera, osg::Vec3 eye);

	void addObject(char* filename, char* objectName);
	void start();
	void showXYGrid();
	void showCross();


	osg::Group* getPickedNodes(){return this->pickedNodes;};


	//functions for picking/unpicking Nodes
	void addPickedNode(osg::Geode* pickedNode){this->pickedNodes->addChild(pickedNode);};
	void unpickNodes();



    osg::Camera* createCamera( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false );
    
    virtual void paintEvent( QPaintEvent* event )
	{   
		frame(); 
	}

public slots:
	void printPickedNodes(){
		std::cout<<"Print Picked Nodes"<<std::endl;
		for(unsigned int i  = 0 ; i < this->getPickedNodes()->getNumChildren() ; i++)
			std::cout << "das ist Picked: " << this->getPickedNodes()->getChild(i)->getName()<< std::endl;
	}
	void addObject(){	
		
	TixiDocumentHandle handle = -1;
	TiglCPACSConfigurationHandle tiglHandle = -1; 
	 
	if(tixiOpenDocument( "CPACS_21_D150.xml", &handle ) != SUCCESS){
		std::cout << "Error reading in plane" << std::endl;
	}
	tiglOpenCPACSConfiguration(handle, "", &tiglHandle);

	tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    
    tigl::CTiglTriangularizer t(wing.GetLoft(), 0.001);

	t.switchObject(1);

	std::cout << "Number of points: " << t.currentObject().getNVertices() << std::endl;
		osg::ref_ptr<GeometricVisObject> geode = new GeometricVisObject(t, "wing4");
		root->addChild(geode.get());
	};



protected:
    
    QTimer _timer;

private:

	//adding the views
	void setupGUI();


	//void checkButtons();
	class QGridLayout * grid;

	//contains all picked Nodes
	osg::ref_ptr<osg::Group> pickedNodes;

	//contains all scene Data
	osg::ref_ptr<osg::Group> root;

	//contains geodes of VirtualVisObject class
	osg::ref_ptr<VirtualVisObject> vvo;



};

#endif