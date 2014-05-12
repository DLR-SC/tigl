/*
* Copyright (C) 2007-2012 German Aerospace Center (DLR/SC)
*
* Created: 2013-09-05 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef TIGLVIEWER_HUD_H
#define TIGLVIEWER_HUD_H

#include <osg/MatrixTransform>
#include <osg/Camera>

class TiglViewerHUD : public osg::Camera
{
public:
    TiglViewerHUD();
    TiglViewerHUD(const TiglViewerHUD& copy, osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY);

    // enables or disables the coordinate cross
    void setCoordinateCrossEnabled(bool);

    META_Node(osg, TiglViewerHUD);

    void setMainCamera( osg::Camera* camera )
    {
        _mainCamera = camera;
    }

protected:
    void init();

    virtual ~TiglViewerHUD();

    virtual void traverse(osg::NodeVisitor& nv);

    osg::ref_ptr<osg::MatrixTransform> _coordinateCross;
    osg::observer_ptr<osg::Camera> _mainCamera;

private:
    osg::MatrixTransform * createCoordinateCross();

};

#endif // TIGLVIEWER_HUD_H
