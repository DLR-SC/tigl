/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
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

#ifndef VIRTUAL_VIS_OBJECT_H
#define VIRTUAL_VIS_OBJECT_H

#include <osg/Group>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

class VirtualVisObject : public osg::Group
{
public:
    VirtualVisObject()
    {
        setXYGrid(true);
        setMainAxesEnabled(true);
    };

    void initGeodes();

    bool isPickable(){return false;};

    bool xyActive(){return xy;};
    bool xzActive(){return xz;};
    bool yzActive(){return yz;};
    bool axesActive(){return axes;};

    void setXYGrid(bool active, int size=500, int unit=10);
    void setXZGrid(bool active, int size=70, int unit=10);
    void setYZGrid(bool active, int size=70, int unit=10);
    void setMainAxesEnabled(bool active);

    //osg::Camera* camera;


private:
    osg::ref_ptr<osg::Geode> axesGeode;
    osg::ref_ptr<osg::Geode> xyGeode;
    osg::ref_ptr<osg::Geode> xzGeode;
    osg::ref_ptr<osg::Geode> yzGeode;

    osg::ref_ptr<osg::PositionAttitudeTransform> hudAxesTransform;

    void initXYGeode(int size, int unit);
    void initXZGeode(int size, int unit);
    void initYZGeode(int size, int unit);
    void initAxesGeode();

    bool xy;
    bool xz;
    bool yz;
    bool axes;
};

#endif

