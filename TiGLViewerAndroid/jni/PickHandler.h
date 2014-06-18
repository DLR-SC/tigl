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
    bool centeringAction;

};

#endif
