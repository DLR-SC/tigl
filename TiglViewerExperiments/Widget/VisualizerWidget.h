#ifndef VISUALIZER_WIDGET_H
#define VISUALIZER_WIDGET_H
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


class VisualizerWidget : public QWidget, public osgViewer::CompositeViewer
{
	Q_OBJECT

public:
    VisualizerWidget(QWidget*);
    
    
    QWidget* addViewWidget( osg::Camera* camera);
	void addObject(char* filename, char* objectName);
	void start();
	void showXYGrid();
	void showCross();

	osg::Group* getPickedNodes(){return this->pickedNodes;};

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



protected:
    
    QTimer _timer;

private:

	void setupGUI();


	//void checkButtons();
	class QGridLayout * grid;

	osg::ref_ptr<osg::Group> pickedNodes;
	osg::ref_ptr<osg::Group> root; 
	osg::ref_ptr<VirtualVisObject> vvo;



};

#endif