#include "VirtualVisObject.hpp"
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <iostream>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

void VirtualVisObject::setXYGrid(bool active, int size, int unit)
{
    if(!xyGeode) initXYGeode(size, unit);

    if(active)
        main->addChild(xyGeode.get());
    else
        main->removeChild(xyGeode.get());

    xy = active;
}

void VirtualVisObject::setXZGrid(bool active, int size, int unit)
{
    if(!xzGeode) initXZGeode(size, unit);

    if(active)
        main->addChild(xzGeode.get());
    else
        main->removeChild(xzGeode.get());

    xz = active;
}


void VirtualVisObject::setYZGrid(bool active, int size, int unit)
{
    if(!yzGeode) initYZGeode(size, unit);

    if(active)
        main->addChild(yzGeode.get());
    else
        main->removeChild(yzGeode.get());

    yz=active;
}


void VirtualVisObject::setAxes(bool active)
{
    if(!axesGeode) initAxesGeode();

    if(active)
        main->addChild(axesGeode.get());
    else
        main->removeChild(axesGeode.get());

    axes = active;
}

void VirtualVisObject::initXYGeode(int size, int unit){

    xyGeode = new osg::Geode();
    osg::Geometry* lines      = new osg::Geometry();
    osg::Geometry* lines_wide = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec3Array* vertices_wide = new osg::Vec3Array();
    for (int i = 0; i <= size; i+=unit) {
        if(i % (10*unit) != 0){
            vertices->push_back(osg::Vec3(-size,i,0));
            vertices->push_back(osg::Vec3(size,i,0));
            vertices->push_back(osg::Vec3(-size,-i,0));
            vertices->push_back(osg::Vec3(size,-i,0));

            vertices->push_back(osg::Vec3( i,-size,0));
            vertices->push_back(osg::Vec3( i, size,0));
            vertices->push_back(osg::Vec3(-i,-size,0));
            vertices->push_back(osg::Vec3(-i, size,0));
        }
        else {
            vertices_wide->push_back(osg::Vec3(-size, i,0));
            vertices_wide->push_back(osg::Vec3( size, i,0));
            vertices_wide->push_back(osg::Vec3(-size,-i,0));
            vertices_wide->push_back(osg::Vec3( size,-i,0));

            vertices_wide->push_back(osg::Vec3( i,-size,0));
            vertices_wide->push_back(osg::Vec3( i, size,0));
            vertices_wide->push_back(osg::Vec3(-i,-size,0));
            vertices_wide->push_back(osg::Vec3(-i, size,0));
        }
    }

    lines->setVertexArray(vertices);
    lines_wide->setVertexArray(vertices_wide);
    lines->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));
    lines_wide->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices_wide->size()));

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //red
    osg::Vec4Array* color_white = new osg::Vec4Array;
    color_white->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) ); //white

    lines->setColorArray(colors);
    lines_wide->setColorArray(color_white);

    lines->setColorBinding(osg::Geometry::BIND_OVERALL);
    lines_wide->setColorBinding(osg::Geometry::BIND_OVERALL);

    xyGeode->addDrawable(lines);
    xyGeode->addDrawable(lines_wide);
    osg::StateSet* stateset = xyGeode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
}

void VirtualVisObject::initXZGeode(int size,int unit)
{
    xzGeode = new osg::Geode();
    osg::Geometry* linesX = new osg::Geometry();

    osg::Vec3Array* vertices = new osg::Vec3Array();
    for (int i = 0; i < size/unit+1; i++) {
        vertices->push_back(osg::Vec3(size, 0, i*unit));
        vertices->push_back(osg::Vec3(-size,0, i*unit));
        vertices->push_back(osg::Vec3(-size,0, -i*unit));
        vertices->push_back(osg::Vec3(size ,0, -i*unit));
    }

    linesX->setVertexArray(vertices);
    linesX->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

    osg::Geometry* linesZ = new osg::Geometry();

    vertices = new osg::Vec3Array();
    for (int i = 0; i < size/unit+1; i++) {
        vertices->push_back(osg::Vec3(i*unit,0,-size));
        vertices->push_back(osg::Vec3(i*unit,0,size));
        vertices->push_back(osg::Vec3(-i*unit,0,-size));
        vertices->push_back(osg::Vec3(-i*unit,0,size));
    }

    linesZ->setVertexArray(vertices);
    linesZ->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

    xzGeode->addDrawable(linesX);
    xzGeode->addDrawable(linesZ);
}


void VirtualVisObject::initYZGeode(int size, int unit)
{
    yzGeode = new osg::Geode();

    osg::Geometry* linesY = new osg::Geometry();

    osg::Vec3Array* vertices = new osg::Vec3Array();
    for(int i = 0; i < size/unit+1; i++){
        vertices->push_back(osg::Vec3(0, size, i*unit));
        vertices->push_back(osg::Vec3(0, -size, i*unit));
        vertices->push_back(osg::Vec3(0, -size, -i*unit));
        vertices->push_back(osg::Vec3(0, size , -i*unit));
    }

    linesY->setVertexArray(vertices);
    linesY->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

    osg::Geometry* linesZ = new osg::Geometry();

    vertices = new osg::Vec3Array();
    for (int i = 0; i < size/unit+1; i++) {
        vertices->push_back(osg::Vec3(0,i*unit,-size));
        vertices->push_back(osg::Vec3(0,i*unit,size));  
        vertices->push_back(osg::Vec3(0,-i*unit,size));
        vertices->push_back(osg::Vec3(0,-i*unit,-size));
    }

    linesZ->setVertexArray(vertices);
    linesZ->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,vertices->size()));

    yzGeode->addDrawable(linesY);
    yzGeode->addDrawable(linesZ);
}

void VirtualVisObject::initAxesGeode()
{
    axesGeode = new osg::Geode();

    osg::ref_ptr<osg::Cylinder> xAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 300.0f);
    xAxis->setRotation(osg::Quat(osg::PI_2, osg::Vec3(0,1,0)));
    osg::ref_ptr<osg::Cylinder> yAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);
    yAxis->setRotation(osg::Quat(osg::PI_2, osg::Vec3(1,0,0)));
    osg::ref_ptr<osg::Cylinder> zAxis = new osg::Cylinder(osg::Vec3(0,0,0), 0.05f, 100.0f);

    osg::ShapeDrawable * xax = new osg::ShapeDrawable(xAxis.get());
    osg::ShapeDrawable * yax = new osg::ShapeDrawable(yAxis.get());
    osg::ShapeDrawable * zax = new osg::ShapeDrawable(zAxis.get());

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f) ); //index 0 red
    xax->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));

    axesGeode->addDrawable(xax);
    axesGeode->addDrawable(yax);
    axesGeode->addDrawable(zax);
}
