#include "OrthoManipulator.h"

OrthoManipulator::OrthoManipulator(osg::Camera* cam)
    : osgGA::OrbitManipulator(), _camera(cam)
{
}

// doc in parent
bool OrthoManipulator::performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy )
{
    // pan model
    float scale = getThrowScale( eventTimeDelta );
    panModel( dx*scale, dy*scale, 0);
    return true;
}

// doc in OthoManipulator2
bool OrthoManipulator::performMovementRightMouseButton( const double eventTimeDelta, const double /*dx*/, const double dy )
{
    // zoom model
    zoomModel( dy * getThrowScale( eventTimeDelta ), true );
    return true;
}

/** Moves camera in x,y,z directions given in camera local coordinates.*/
void OrthoManipulator::panModel( const float dx, const float dy, const float dz )
{
    
    osg::Vec3d deltaScreen(dx,dy,0);
    osg::Vec3d deltaWorld = -deltaScreen * osg::Matrixd::inverse(_camera->getProjectionMatrix());
    deltaWorld.z() = 0.;

    osg::Matrix rotation_matrix;
    rotation_matrix.makeRotate( _rotation );


    _center += deltaWorld * rotation_matrix;
}

void OrthoManipulator::zoomModel( const float dy, bool /* pushForwardIfNeeded */)
{
    // scale
    float l = 1.0f + dy;

    double left, right, buttom, top, zFar, zNear;
    _camera->getProjectionMatrixAsOrtho(left, right, buttom, top, zFar, zNear);
    _camera->setProjectionMatrixAsOrtho(left*l, right * l, buttom * l, top * l, zFar, zNear);
}
