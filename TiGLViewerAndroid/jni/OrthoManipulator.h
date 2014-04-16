/*
 * OrthoManipulator.h
 *
 *  Created on: 15.04.2014
 *      Author: aly_mm
 */

#ifndef ORTHOMANIPULATOR_H_
#define ORTHOMANIPULATOR_H_

#include <osgGA/StandardManipulator>
#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

class OrthoManipulator : public osgGA::StandardManipulator
{
public:
    OrthoManipulator(osg::Camera* c);

    virtual osg::Matrixd getMatrix() const;
    virtual osg::Matrixd getInverseMatrix() const;

    /** set the position of the matrix manipulator using a 4x4 Matrix.*/
    virtual void setByMatrix(const osg::Matrixd& matrix){}

    /** set the position of the matrix manipulator using a 4x4 Matrix.*/
    virtual void setByInverseMatrix(const osg::Matrixd& matrix){}

    virtual void setTransformation( const osg::Vec3d&, const osg::Quat& ) {}
    virtual void setTransformation( const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up);
    virtual void getTransformation( osg::Vec3d&, osg::Quat& ) const {}
    virtual void getTransformation( osg::Vec3d&, osg::Vec3d&, osg::Vec3d& ) const {}

protected:
    virtual bool performMovementLeftMouseButton(const double eventTimeDelta, const double dx, const double dy );
    virtual bool performMovementMiddleMouseButton(const double eventTimeDelta, const double dx, const double dy );
    virtual bool performMovementRightMouseButton(const double eventTimeDelta, const double dx, const double dy );
    virtual bool handleMouseWheel( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual bool handleMousePush( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    virtual bool handleMouseRelease( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us );
    void rotateModel(const double dx, const double dy);
    void panModel(const double dx, const double dy );
    void zoom(double level);

private:
    osg::Camera* _camera;

    osg::Vec3d _center;
    osg::Quat _rotation;
    float _distance;
};

#endif /* ORTHOMANIPULATOR_H_ */
