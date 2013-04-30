#include "crossnodegeode.h"

#include "VirtualVisObject.h"
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <iostream>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>


CrossNodeGeode::CrossNodeGeode(){}
osg::MatrixTransform* CrossNodeGeode::initNodeGeode()
{
    int objectX = 0;
    int objectY = 0;
    int objectZ = 0;

    osg::Vec4Array* colors = new osg::Vec4Array();
    colors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));

    osg::ref_ptr<osg::Geode> crossGeode = new osg::Geode();

    osg::Cylinder* xCylinder = new osg::Cylinder(osg::Vec3(objectX+0.25,objectY,objectZ), 0.07f, 0.5f);
    xCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
    osg::ShapeDrawable* xC = new osg::ShapeDrawable(xCylinder);
    xC->setColor(osg::Vec4(1.0,0.0,0.0,1.0));

    osg::Cylinder* yCylinder = new osg::Cylinder(osg::Vec3(objectX,objectY+0.25,objectZ), 0.07f, 0.5f);
    yCylinder->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* yC = new osg::ShapeDrawable(yCylinder);
    yC->setColor(osg::Vec4(0.0,1.0,0.0,1.0));

    osg::Cylinder* zCylinder = new osg::Cylinder(osg::Vec3(objectX,objectY,objectZ+0.25), 0.07f, 0.5f);
    zCylinder->setRotation(osg::Quat(osg::PI, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* zC = new osg::ShapeDrawable(zCylinder);
    zC->setColor(osg::Vec4(0.0,0.0,1.0,1.0));

    osg::Cone* xCone = new osg::Cone(osg::Vec3(objectX+0.5,objectY,objectZ), 0.12f, 0.1f);
    xCone->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,0.5f,0)));
    osg::ShapeDrawable* xO = new osg::ShapeDrawable(xCone);
    xO->setColor(osg::Vec4(1.0,0.0,0.0,1.0));

    osg::Cone* yCone = new osg::Cone(osg::Vec3(objectX,objectY+0.5,objectZ), 0.12f, 0.1f);
    yCone->setRotation(osg::Quat(-osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* yO = new osg::ShapeDrawable(yCone);
    yO->setColor(osg::Vec4(0.0,1.0,0.0,1.0));

    osg::Cone* zCone = new osg::Cone(osg::Vec3(objectX,objectY,objectZ+0.5), 0.12f, 0.1f);
    //zCone->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0.5f,0,0)));
    osg::ShapeDrawable* zO = new osg::ShapeDrawable(zCone);
    zO->setColor(osg::Vec4(0.0,0.0,1.0,1.0));

    crossGeode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(objectX,objectY,objectZ), 0.10f)));
    crossGeode->addDrawable(xC);
    crossGeode->addDrawable(yC);
    crossGeode->addDrawable(zC);
    crossGeode->addDrawable(xO);
    crossGeode->addDrawable(yO);
    crossGeode->addDrawable(zO);

    osg::ref_ptr<osg::MatrixTransform> output = new osg::MatrixTransform;
    output->addChild(crossGeode.get());

    return output.release();
}

