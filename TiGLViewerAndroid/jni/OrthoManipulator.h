#ifndef ORTHOMANIPULATOR_H
#define ORTHOMANIPULATOR_H

#include <osgGA/OrbitManipulator>

class OrthoManipulator : public osgGA::OrbitManipulator
{
public:
    OrthoManipulator(osg::Camera* cam);
    bool performMovementMiddleMouseButton( const double eventTimeDelta, const double dx, const double dy );
    bool performMovementRightMouseButton( const double eventTimeDelta, const double /*dx*/, const double dy );
    void panModel( const float dx, const float dy, const float dz );
    void zoomModel( const float dy, bool pushForwardIfNeeded);
    
private:
    osg::Camera* _camera;
};

#endif // ORTHOMANIPULATOR_H
