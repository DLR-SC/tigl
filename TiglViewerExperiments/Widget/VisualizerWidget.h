#ifndef VISUALIZER_WIDGET_H
#define VISUALIZER_WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QResizeEvent>
#include <QSize>
#include <QList>


#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>
#include "VirtualVisObject.h"


#include "CrossNode.h"
#include "RenderingThread.h"

class VisualizerWidget : public QWidget, public osgViewer::CompositeViewer
{
	Q_OBJECT

public:
    VisualizerWidget(QWidget*);
    
    void addCross(osg::ref_ptr<CrossNode> &crossnode, osgViewer::View* view, osg::Group *group);
    QWidget *create2DView(osg::Camera *camera, osg::Vec3 eye, unsigned i);
    void set();
    QWidget* addViewWidget(osg::Camera* camera, osg::Vec3 eye);

	void addObject(char* filename, char* objectName);
	void start();
	void showXYGrid();
	void showCross();


    osg::Group* getPickedNodes(){return this->pickedNodes;}


	//functions for picking/unpicking Nodes
	void addPickedNode(osg::Geode* pickedNode){this->pickedNodes->addChild(pickedNode);}
	void unpickNodes();



    osg::Camera* createCamera( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false );
    
    virtual void paintEvent( QPaintEvent* event )
    {
        frame();
    }

    void resizeEvent(QResizeEvent * event);



public slots:
    void printPickedNodes();
    void addObject();


protected:
    
    QTimer _timer;

private:

    //adding the views
    void setupGUI();

    //List that Contains all widgets
    QList<QWidget*> WidgetList;

	//void checkButtons();
	class QGridLayout * grid;


	//contains all picked Nodes
	osg::ref_ptr<osg::Group> pickedNodes;

    //contains all Data scenes
    osg::ref_ptr<osg::Group> GroupWindow1;
    osg::ref_ptr<osg::Group> GroupWindow2;
    osg::ref_ptr<osg::Group> GroupWindow3;
    osg::ref_ptr<osg::Group> GroupWindow4;

	//contains geodes of VirtualVisObject class
	osg::ref_ptr<VirtualVisObject> vvo;

    //crossNodes for all
    osg::ref_ptr<CrossNode> crossnode11;
    osg::ref_ptr<CrossNode> crossnode12;
    osg::ref_ptr<CrossNode> crossnode21;
    osg::ref_ptr<CrossNode> crossnode22;
    //viewPort for the crossNode
    int xWP;
    int yWP;

};

#endif
