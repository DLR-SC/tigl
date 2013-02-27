#include "GeometricVisObject.h"
#include "Visualizer.h"
#include "PickHandler.h"
#include "SelectionBoxHandler.h"
#include <osgDB/Registry>
int main(){
	Visualizer* vis = new Visualizer(100,100,800,600);
	vis->addObject("wing4.vtp", "wing4");
	vis->addObject("f19-2.vtp", "f-19");

	vis->start();
	for(unsigned int i  = 0 ; i < vis->getPickedNodes()->getNumChildren() ; i++)
		std::cout << "das ist Picked: " << vis->getPickedNodes()->getChild(i)->getName()<< std::endl;

	std::getchar();

	return 0;
}