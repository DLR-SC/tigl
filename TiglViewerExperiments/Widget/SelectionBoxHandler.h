#ifndef SELECTION_BOX_HANDLER_H
#define SELECTION_BOX_HANDLER_H

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>

class SelectionBoxHandler : public osgGA::GUIEventHandler
{
public:
	SelectionBoxHandler(){};
	~SelectionBoxHandler(void){};
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
private:
	bool select(osgViewer::View* view, const double currentX, const double currentY);
	double start_X;
	double start_Y;

};

#endif