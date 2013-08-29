#ifndef CROSSNODEGEODE_H
#define CROSSNODEGEODE_H

#include <osg/Group>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>


class CrossNodeGeode: public osg::Geode
{
public:
    CrossNodeGeode();
    osg::MatrixTransform* initNodeGeode();
};

#endif // CROSSNODEGEODE_H
