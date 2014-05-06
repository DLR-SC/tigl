#include "PickHandler.h"
#include <osgUtil/LineSegmentIntersector>
#include <iostream>
#include "GeometricVisObject.h"
#include "MaterialTemplate.h"

int PickHandler::hitCount = 0;

PickHandler::PickHandler()
{
    _lastx = 0.;
    _lasty = 0.;
}

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    bool add = false;
    if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)
        add = true;


    switch (ea.getEventType()) {
    case (osgGA::GUIEventAdapter::RELEASE):

    {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
        double dx = _lastx - ea.getXnormalized();
        double dy = _lasty - ea.getYnormalized();
        double distance = sqrt(dx*dx + dy*dy);
        _lastx = ea.getXnormalized();
        _lasty = ea.getYnormalized();
        if (view && distance < 0.05) {
            pick(view, ea, add);
            return true;
        }
        else {
            return false;
        }
    }
    case (osgGA::GUIEventAdapter::PUSH):
    {
        _lastx = ea.getXnormalized();
        _lasty = ea.getYnormalized();
        return false;
    }
    default:
        return false;

    }


}

void PickHandler::pick(osgViewer::View* view, const osgGA::GUIEventAdapter& ea, bool add)
{
    float xwindow =  ea.getX()/(ea.getXmax()- ea.getXmin())  * 2. - 1;
    float ywindow = -ea.getY()/(ea.getYmax()- ea.getYmin())  * 2. + 1;


    osg::Camera* cam = view->getCamera();

    osg::Matrixd m;
    m.preMult(cam->getProjectionMatrix());
    m.preMult(cam->getViewMatrix());

    // define intersection ray
    osg::Vec3d startPoint (xwindow, ywindow, -1);
    osg::Vec3d endPoint(xwindow, ywindow,  1);

    osg::Matrixd i;
    i.invert(m);

    osg::Vec3d wStart =  startPoint * i;
    osg::Vec3d wEnd   =  endPoint   * i;

    osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = new osgUtil::LineSegmentIntersector(wStart, wEnd);
    osgUtil::IntersectionVisitor iv(picker.get());
    cam->accept(iv);
    iv.setTraversalMask(~0x1);

    if (picker->containsIntersections()) {

        osgUtil::LineSegmentIntersector::Intersections allIntersections = picker->getIntersections();
        osgUtil::LineSegmentIntersector::Intersections::iterator intersectionsIterator = allIntersections.begin();

        GeometricVisObject* pickedObject;
        std::string nameOfpickedObject;

        for (int i = 0; i < intersectionsIterator->nodePath.size(); i++) {

            pickedObject = (GeometricVisObject*) intersectionsIterator->nodePath.at(i);
            nameOfpickedObject = intersectionsIterator->nodePath.at(i)->getName();

            if (!(intersectionsIterator->nodePath.at(i)->getName().empty())) {
                if (!pickedObject->isPicked()) {
                    pickedObject->pick();
                }
                else {
                    pickedObject->unpick();
                }
            }

        }
    }
}
