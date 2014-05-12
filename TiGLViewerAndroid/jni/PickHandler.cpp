/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "PickHandler.h"
#include <osgUtil/LineSegmentIntersector>
#include <iostream>
#include "GeometricVisObject.h"
#include "MaterialTemplate.h"
#include <osgGA/StandardManipulator>

#define TIME_TO_CENTER 0.4
#define PICK_MOVEMENT_THRESHOLD 0.05

PickHandler::PickHandler()
{
    _lastx = 0.;
    _lasty = 0.;
    _lastTime= 0.;
}

bool PickHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    bool add = false;
    if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL) {
        add = true;
    }

    switch (ea.getEventType()) {
    case (osgGA::GUIEventAdapter::RELEASE): {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
        double dx = _lastx - ea.getXnormalized();
        double dy = _lasty - ea.getYnormalized();
        double distance = sqrt(dx*dx + dy*dy);
        _lastx = ea.getXnormalized();
        _lasty = ea.getYnormalized();
        if (view && distance < PICK_MOVEMENT_THRESHOLD) {
            if (ea.getTime() - _lastTime >= TIME_TO_CENTER) {
                // center view picked point
                osgUtil::LineSegmentIntersector::Intersections its = rayIntersection(view, ea, add);
                changeCOR(view,its);
            }
            else {
                // select object
                osgUtil::LineSegmentIntersector::Intersections its = rayIntersection(view, ea, add);
                pick(its);
            }
            return true;
        }
        else {
           return false;
        }
    }
    case (osgGA::GUIEventAdapter::PUSH): {
        _lastTime = ea.getTime();
        _lastx = ea.getXnormalized();
        _lasty = ea.getYnormalized();
        return false;
    }
    default:
        return false;

    }


}
osgUtil::LineSegmentIntersector::Intersections PickHandler::rayIntersection(osgViewer::View* view, const osgGA::GUIEventAdapter& ea, bool add)
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

    osgUtil::LineSegmentIntersector::Intersections allIntersections;
    if (picker->containsIntersections()) {

        allIntersections = picker->getIntersections();
    }
    return allIntersections;
}

void PickHandler::pick(osgUtil::LineSegmentIntersector::Intersections allIntersections)
{
   if (allIntersections.empty()) {
       return;
   }

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

void PickHandler::changeCOR(osgViewer::View* view, osgUtil::LineSegmentIntersector::Intersections allIntersections)
{
    osg::Vec3d eye, oldCenter, up;
    osgGA::StandardManipulator* m = dynamic_cast<osgGA::StandardManipulator*> (view->getCameraManipulator());
    m->getTransformation( eye, oldCenter, up );

    osgUtil::LineSegmentIntersector::Intersections::iterator intersectionsIterator = allIntersections.begin();
    for (int i = 0; i < intersectionsIterator->nodePath.size(); i++) {

        GeometricVisObject* pickedObject = dynamic_cast<GeometricVisObject*>(intersectionsIterator->nodePath.at(i));

        if (pickedObject) {
            osg::Vec3d newCenter = intersectionsIterator->getWorldIntersectPoint();
            osg::Vec3d displacement = newCenter - oldCenter;
            m->setTransformation( eye + displacement, newCenter, up );
            break;
        }
    }
}

