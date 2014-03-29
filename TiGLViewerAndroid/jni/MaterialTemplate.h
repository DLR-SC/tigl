#ifndef MATERIALTEMPLATE_H
#define MATERIALTEMPLATE_H

#include<osg/Material>

enum TIGL_MATERIALS
{
    RED, GREEN, BLUE, WHITE, SELECTED, UNSELECTED
};

class MaterialTemplate
{
public:
    static osg::ref_ptr<osg::Material> getMaterial(TIGL_MATERIALS material);
};

#endif 
