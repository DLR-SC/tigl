#include <tigl.h>
#include <CCPACSConfigurationManager.h>
#include <CCPACSConfiguration.h>
#include <CCPACSWing.h>
#include <CTiglTriangularizer.h>

#include "GeometricVisObject.h"
#include "PickHandler.h"
//#include "SelectionBoxHandler.h"
//#include "VisualizerWidget.h"
#include "MainWindow.h"
#include <iostream>
#include <tixi.h>


int main(int argc, char** argv){


	/*TixiDocumentHandle handle = -1;
	TiglCPACSConfigurationHandle tiglHandle = -1; 
	 
	if(tixiOpenDocument( "CPACS_21_D150.xml", &handle ) != SUCCESS){
		std::cout << "Error reading in plane" << std::endl;
		return 1;
	}
	tiglOpenCPACSConfiguration(handle, "", &tiglHandle);

	tigl::CCPACSConfigurationManager & manager = tigl::CCPACSConfigurationManager::GetInstance();
    tigl::CCPACSConfiguration & config = manager.GetConfiguration(tiglHandle);
    tigl::CCPACSWing& wing = config.GetWing(1);
    
    tigl::CTiglTriangularizer t(wing.GetLoft(), 0.001);

	t.switchObject(1);
		//t.writeVTK("mywing.vtp");
	std::cout << "Number of points: " << t.currentObject().getNVertices() << std::endl;*/


/*    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
    while (arguments.read("--SingleThreaded")) threadingModel = osgViewer::ViewerBase::SingleThreaded;
    while (arguments.read("--CullDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullDrawThreadPerContext;
    while (arguments.read("--DrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::DrawThreadPerContext;
    while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) threadingModel = osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext;*/
    
    QApplication app(argc, argv);
    // we need to set us locale as we use "." for decimal point
    setlocale(LC_NUMERIC, "C");

	//VisualizerWidget* vis = new VisualizerWidget(NULL);
	MainWindow * window = new MainWindow;



	//window->getWidget()->addObject(t, "wing4");
	//window->getWidget()->addObject("mywing.vtp", "wing41000");
	window->show();


	/*for(unsigned int i  = 0 ; i < vis->getPickedNodes()->getNumChildren() ; i++)
		std::cout << "das ist Picked: " << vis->getPickedNodes()->getChild(i)->getName()<< std::endl;

	std::getchar();*/


    return app.exec();
}
