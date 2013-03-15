#include "GeometricVisObject.h"
#include "PickHandler.h"
//#include "SelectionBoxHandler.h"
//#include "VisualizerWidget.h"
#include "MainWindow.h"
#include <iostream>

int main(int argc, char** argv){
/*    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
    while (arguments.read("--SingleThreaded")) threadingModel = osgViewer::ViewerBase::SingleThreaded;
    while (arguments.read("--CullDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
    while (arguments.read("--DrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::DrawThreadPerContext;
    while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext;*/
    
    QApplication app(argc, argv);
	//VisualizerWidget* vis = new VisualizerWidget(NULL);
	MainWindow * window = new MainWindow;

	window->getWidget()->addObject("wing4.vtp", "wing4");
	window->getWidget()->addObject("f19-2.vtp", "f-19");

	window->show();




	/*for(unsigned int i  = 0 ; i < vis->getPickedNodes()->getNumChildren() ; i++)
		std::cout << "das ist Picked: " << vis->getPickedNodes()->getChild(i)->getName()<< std::endl;

	std::getchar();*/


    return app.exec();
}