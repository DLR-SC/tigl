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


#include "TiglViewerBackground.h"

#include <osg/Geometry>
#include <osg/Depth>
#include <osg/Geode>

TiglViewerBackground::TiglViewerBackground()
{
    setCullingActive( false );
    setClearMask( 0 );
    setAllowEventFocus( false );
    setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    setRenderOrder( osg::Camera::POST_RENDER, 0 );
    setProjectionMatrix( osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0) );

    osg::StateSet* ss = getOrCreateStateSet();
    ss->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    ss->setAttributeAndModes( new osg::Depth(osg::Depth::LEQUAL, 1.0, 1.0) );

}

TiglViewerBackground::~TiglViewerBackground()
{
}

void TiglViewerBackground::makeGradient(const osg::Vec4& downCol, const osg::Vec4& upCol)
{
    unsigned int nChilds = getNumChildren();
    removeChildren(0,nChilds);

    osg::Vec3Array *vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0,0,0));
    vertices->push_back(osg::Vec3(1,0,0));
    vertices->push_back(osg::Vec3(1,1,0));
    vertices->push_back(osg::Vec3(0,1,0));

    osg::Vec4Array *  colors = new osg::Vec4Array;
    colors->push_back( downCol );
    colors->push_back( downCol );
    colors->push_back( upCol );
    colors->push_back( upCol );

    osg::DrawArrays* array = new osg::DrawArrays(osg::PrimitiveSet::QUADS , 0 , vertices->size());

    osg::Geometry * geometry = new osg::Geometry;
    geometry->addPrimitiveSet(array);
    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors);
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    osg::Geode * bgnode = new osg::Geode;
    bgnode->addDrawable(geometry);

    addChild(bgnode);
}

void TiglViewerBackground::makeGradient(const osg::Vec4& col)
{
    double R1 = col.r();
    double G1 = col.g();
    double B1 = col.b();
    double alpha = col.a();

    double fu = 2.;
    double fd = 0.2;

    osg::Vec4 up  (R1*fu > 1 ? 1. : R1*fu, G1*fu > 1 ? 1. : G1*fu, B1*fu > 1 ? 1. : B1*fu, alpha);
    osg::Vec4 down(R1*fd > 1 ? 1. : R1*fd, G1*fd > 1 ? 1. : G1*fd, B1*fd > 1 ? 1. : B1*fd, alpha);

    makeGradient(down, up);
}
