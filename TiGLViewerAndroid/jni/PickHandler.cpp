#include "PickHandler.h"
#include <osgUtil/LineSegmentIntersector>
#include <iostream>
#include "GeometricVisObject.h"
//#include "Visualizer.h"
#include "MaterialTemplate.h"

int PickHandler::hitCount = 0;

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    bool add = false;
    if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
        add = true;

    switch (ea.getEventType()) {
    case (osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON):

    {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
        if (view)
            pick(view, ea, add);
        return false;
    }
    default:
        return false;

    }

}

void PickHandler::pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea, bool add)
{
    osgUtil::LineSegmentIntersector::Intersections intersections;
    //Visualizer* visualizer = (Visualizer*) view->getViewerBase();
    //if(!add) visualizer->unpickNodes();

    if (view->computeIntersections(ea.getX(), ea.getY(), intersections)) {

        osgUtil::LineSegmentIntersector::Intersections::iterator hit = intersections.begin();

        if (!hit->nodePath.empty() && !(hit->nodePath.back()->getName().empty())) {
            GeometricVisObject* intersectedGeode = (GeometricVisObject*) hit->nodePath.back();
            if (!intersectedGeode->isPicked()) {
                intersectedGeode->pick();
                //visualizer->addPickedNode(intersectedGeode);
            }
            else {
                intersectedGeode->unpick();
            }
            std::cout << hitCount << "Hit:" << hit->nodePath.back()->getName() << std::endl;
        }
    }
}
