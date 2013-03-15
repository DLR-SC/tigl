#ifndef PICKHANDLER_H
#define PICKHANDLER_H
#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <iostream>
class PickHandler : public osgGA::GUIEventHandler
{
public:
	static int hitCount;
	~PickHandler(){};
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
private:
	virtual void pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea, bool add);


};

#endif
