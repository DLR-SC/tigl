#ifndef VIEWINTERACTIONSHANDLER_H
#define VIEWINTERACTIONSHANDLER_H

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <iostream>

class ViewInteractionsHandler : public osgGA::GUIEventHandler
{
public:
    ViewInteractionsHandler();
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

private:
    int Size;
    int counter;

};

#endif // VIEWINTERACTIONSHANDLER_H
