#ifndef PICKHANDLER_H
#define PICKHANDLER_H
#include <osgGA/GUIEventHandler>
#include <osgViewer/View>

class PickHandler : public osgGA::GUIEventHandler
{
public:
    PickHandler();
    ~PickHandler(){};
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
private:
    virtual void pick(osgUtil::LineSegmentIntersector::Intersections allIntersections);
    osgUtil::LineSegmentIntersector::Intersections rayIntersection(osgViewer::View* view, const osgGA::GUIEventAdapter& ea, bool add);
    void changeCOR(osgViewer::View* view, osgUtil::LineSegmentIntersector::Intersections allIntersections);

    double _lastx, _lasty;
    double _lastTime;

};

#endif
