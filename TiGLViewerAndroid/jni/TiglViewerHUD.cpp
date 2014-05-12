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

#include "TiglViewerHUD.h"

#include "MaterialTemplate.h"

#include <osg/Matrixd>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/LightSource>
#include <osgText/Text>
#include <osgText/Font>

#include "ResourceManager.h"
#include "OsgMainApp.hpp"

TiglViewerHUD::TiglViewerHUD()
{
    init();
}

TiglViewerHUD::TiglViewerHUD(const TiglViewerHUD &copy, osg::CopyOp copyop) :
    osg::Camera(copy, copyop),
    _coordinateCross(copy._coordinateCross),
    _mainCamera(copy._mainCamera)
{
}

TiglViewerHUD::~TiglViewerHUD()
{
    init();
}

void TiglViewerHUD::init()
{
    setProjectionMatrix(osg::Matrixd::ortho2D(-1.5, 1.5, -1.5, 1.5));
    setRenderOrder( osg::Camera::POST_RENDER, 1);
    setClearMask( GL_DEPTH_BUFFER_BIT );
    setAllowEventFocus( false );
    setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    setName("HUD");

    // TODO: check if this is necessary
    getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );

    osg::LightSource* lightSource = new osg::LightSource;
    lightSource->getLight()->setPosition(osg::Vec4d( 0.0f, 0.0f, 5.f, 1.0f )); // point light
    addChild(lightSource);
}

void TiglViewerHUD::traverse(osg::NodeVisitor &nv)
{
    // apply camera rotation to the coordinate cross
    if ( _mainCamera.valid() &&  _coordinateCross.valid() && nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR ) {
        osg::Vec3 trans, scale;
        osg::Quat rot, orient;
        _mainCamera->getViewMatrix().decompose(trans, rot, scale, orient);

        osg::Matrix rot_mat(rot);

        // the -10 moves the cross a bit back to prevent culling at the view plane
        rot_mat.postMult(osg::Matrix::translate(-0.3,-0.3,-10));

        _coordinateCross->setMatrix(rot_mat);

        _coordinateCross->accept( nv );
    }

    osg::Camera::traverse( nv );
}

void TiglViewerHUD::setCoordinateCrossEnabled(bool enabled)
{
    if (enabled) {
        _coordinateCross = createCoordinateCross();
    }
    else {
        _coordinateCross = NULL;
    }
}


/**
 * creates a coordinate cross that can be displayed inside the HUD
 */
osg::MatrixTransform* TiglViewerHUD::createCoordinateCross()
{
    int objectX = 0;
    int objectY = 0;
    int objectZ = 0;

    osg::Vec4Array* colors = new osg::Vec4Array();
    colors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));

    osg::Cylinder* xCylinder = new osg::Cylinder(osg::Vec3(objectX+0.25,objectY,objectZ), 0.07f, 0.6f);
    xCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
    osg::ShapeDrawable* xC = new osg::ShapeDrawable(xCylinder);
    xC->setColor(osg::Vec4(1.0f,0.0f,0.0f,1.0f));

    osg::Cylinder* yCylinder = new osg::Cylinder(osg::Vec3(objectX,objectY+0.25,objectZ), 0.07f, 0.6f);
    yCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* yC = new osg::ShapeDrawable(yCylinder);
    yC->setColor(osg::Vec4(0.0,1.0,0.0,1.0));

    osg::Cylinder* zCylinder = new osg::Cylinder(osg::Vec3(objectX,objectY,objectZ+0.25), 0.07f, 0.6f);
    zCylinder->setRotation(osg::Quat(osg::PI, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* zC = new osg::ShapeDrawable(zCylinder);
    zC->setColor(osg::Vec4(0.0,0.0,1.0,1.0));

    osg::Cone* xCone = new osg::Cone(osg::Vec3(objectX+0.6,objectY,objectZ), 0.12f, 0.2f);
    xCone->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
    osg::ShapeDrawable* xO = new osg::ShapeDrawable(xCone);
    xO->setColor(osg::Vec4(1.0,0.0,0.0,1.0));

    osg::Cone* yCone = new osg::Cone(osg::Vec3(objectX,objectY+0.6,objectZ), 0.12f, 0.2f);
    yCone->setRotation(osg::Quat(-osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* yO = new osg::ShapeDrawable(yCone);
    yO->setColor(osg::Vec4(0.0,1.0,0.0,1.0));

    osg::Cone* zCone = new osg::Cone(osg::Vec3(objectX,objectY,objectZ+0.6), 0.12f, 0.2f);
    //zCone->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* zO = new osg::ShapeDrawable(zCone);
    zO->setColor(osg::Vec4(0.0,0.0,1.0,1.0));

    // load font from file
    osg::ref_ptr<osgText::Font> font = ResourceManager::OpenFontFile("DroidSans.ttf");

    osg::ref_ptr<osgText::Text> xText = new osgText::Text;
    xText->setPosition(osg::Vec3(objectX + 0.95,objectY,objectZ));
    xText->setAlignment(osgText::Text::CENTER_CENTER);
    xText->setText("x");
    xText->setFont(font);
    xText->setCharacterSize(0.3);
    xText->setAxisAlignment(osgText::Text::SCREEN);

    osg::ref_ptr<osgText::Text> yText = new osgText::Text(*xText);
    yText->setPosition(osg::Vec3(objectX,objectY + 0.95,objectZ));
    yText->setText("y");

    osg::ref_ptr<osgText::Text> zText = new osgText::Text(*xText);
    zText->setPosition(osg::Vec3(objectX,objectY,objectZ + 0.95));
    zText->setText("z");

    // apply materials for correct lighting
    osg::Geode * xArrowGeode = new osg::Geode;
    xArrowGeode->addDrawable(xC);
    xArrowGeode->addDrawable(xO);
    xArrowGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(RED));

    osg::Geode * yArrowGeode = new osg::Geode;
    yArrowGeode->addDrawable(yC);
    yArrowGeode->addDrawable(yO);
    yArrowGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(GREEN));

    osg::Geode * zArrowGeode = new osg::Geode;
    zArrowGeode->addDrawable(zC);
    zArrowGeode->addDrawable(zO);
    zArrowGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(BLUE));



    osg::Geode* centerBallGeode = new osg::Geode();
    centerBallGeode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(objectX,objectY,objectZ), 0.10f)));
    centerBallGeode->getOrCreateStateSet()->setAttribute(MaterialTemplate::getMaterial(WHITE));

    centerBallGeode->addDrawable(xText);
    centerBallGeode->addDrawable(yText);
    centerBallGeode->addDrawable(zText);

    osg::ref_ptr<osg::MatrixTransform> output = new osg::MatrixTransform;
    output->addChild(centerBallGeode);
    output->addChild(xArrowGeode);
    output->addChild(yArrowGeode);
    output->addChild(zArrowGeode);

    osg::StateSet* stateset = output->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);


    return output.release();
}
