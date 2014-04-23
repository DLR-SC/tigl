/*
 * OrthoManipulator.cpp
 *
 *  Created on: 15.04.2014
 *      Author: aly_mm
 */

#include "OrthoManipulator.h"

#include <CTiglLogging.h>

#undef DEBUG

 OrthoManipulator::OrthoManipulator(osg::Camera* c)
    : _center(0, 0, 0), _distance(100), _camera(c)
    {
        // disable automove or autopan after input events
        setAllowThrow(false);
    }


osg::Matrixd OrthoManipulator::getMatrix() const
{
    osg::Matrixd t1;
    t1.postMultTranslate(osg::Vec3(0,0, _distance));
    t1.postMultRotate(_rotation);
    t1.postMultTranslate(_center);

    return t1;
}

osg::Matrixd OrthoManipulator::getInverseMatrix() const
{
    osg::Matrixd t2;
    t2.postMultTranslate(-_center);
    t2.postMultRotate(_rotation.inverse());
    t2.postMultTranslate(osg::Vec3(0,0,-_distance));

    return t2;
}

void OrthoManipulator::getTransformation( osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up ) const
{
    center = _center;
    eye = _center + _rotation * osg::Vec3d( 0., 0., _distance );
    up = _rotation * osg::Vec3d( 0., 1., 0. );
}


void OrthoManipulator::setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up)
{
    osg::Vec3d lv( center - eye );

    osg::Vec3d f( lv );
    f.normalize();
    osg::Vec3d s( f^up );
    s.normalize();
    osg::Vec3d u( s^f );
    u.normalize();

    osg::Matrixd rotation_matrix( s[0], u[0], -f[0], 0.0f,
                                  s[1], u[1], -f[1], 0.0f,
                                  s[2], u[2], -f[2], 0.0f,
                                  0.0f, 0.0f,  0.0f, 1.0f );

    _center   = center;
    _distance = lv.length();
    _rotation = rotation_matrix.getRotate().inverse();

    // fix current rotation
    if( getVerticalAxisFixed() )
        fixVerticalAxis( _center, _rotation, true );
}


void OrthoManipulator::zoom(double l)
{
    double left, right, buttom, top, zFar, zNear;
    _camera->getProjectionMatrixAsOrtho(left, right, buttom, top, zFar, zNear);
    _camera->setProjectionMatrixAsOrtho(left*l, right * l, buttom * l, top * l, zFar, zNear);
}

// Rotate on left move
bool OrthoManipulator::performMovementLeftMouseButton(const double eventTimeDelta, const double dx, const double dy ){

    rotateModel(dx,dy);
#ifdef DEBUG
    LOG(INFO) << "Left Mouse Action " << std::endl;
#endif
    return true;
}

// pan on middle button move
bool OrthoManipulator::performMovementMiddleMouseButton(const double eventTimeDelta, const double dx, const double dy )
{
    panModel(dx, dy);
#ifdef DEBUG
    LOG(INFO)  << "Middle Mouse Action " << std::endl;
#endif
    return true;
}

// zoom on right button move
bool OrthoManipulator::performMovementRightMouseButton(const double eventTimeDelta, const double dx, const double dy )
{
#ifdef DEBUG
    LOG(INFO) << "Right Mouse Action with " << dy << std::endl;
#endif

    // make zoom speed proportional to movement
    float zoomStep = pow(0.95, -dy*20.);
    zoom(zoomStep);

    return true;
}

bool OrthoManipulator::handleMouseWheel( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
#ifdef DEBUG
    LOG(INFO) << "Mouse Wheel motion " << std::endl;
#endif
    return true;
}

bool OrthoManipulator::handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    /*
    if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) {
        handleMouseWheel(ea, us);
        //setCenterByMousePointerIntersection( ea, us );
    }
    */
    return true;
}


bool OrthoManipulator::handleMouseRelease( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us )
{
    flushMouseEventStack();
    return true;
}

void OrthoManipulator::panModel(const double dx, const double dy )
{
    osg::Vec3d deltaScreen(dx,dy,0);
    osg::Vec3d deltaWorld = -deltaScreen * osg::Matrixd::inverse(_camera->getProjectionMatrix());
    deltaWorld.z() = 0.;

    _center += _rotation * deltaWorld;
}

void OrthoManipulator::rotateModel(const double dx, const double dy)
{
    osg::CoordinateFrame coordinateFrame = getCoordinateFrame( _center );
    osg::Vec3d localUp = getUpVector( coordinateFrame );

    rotateYawPitch( _rotation, dx, dy, localUp );
}


