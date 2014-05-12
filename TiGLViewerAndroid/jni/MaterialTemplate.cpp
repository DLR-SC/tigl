/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
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


#include "MaterialTemplate.h"
#include<osg/Array>
#include<algorithm>


osg::ref_ptr<osg::Material> MaterialTemplate::getMaterial(TIGL_MATERIALS number)
{
    osg::ref_ptr<osg::Material> material = new osg::Material();
    material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(88/255.0f, 127/255.0f, 76/255.0f, 1.0f));
    material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(226/255.0f, 255/255.0f, 221/255.0f, 1.0f));

    switch (number) {
    case SELECTED:
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(176/255.0f, 255/255.0f, 153/255.0f, 1.0f));
        material->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
        material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(88/255.0f, 127/255.0f, 76/255.0f, 1.0f));
        material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(226/255.0f, 255/255.0f, 221/255.0f, 1.0f));
        break;

    case RED:
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.f, 0.f, 0.f, 1.0f));
        break;

    case GREEN:
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.f, 1.f, 0.f, 1.0f));
        break;

    case BLUE:
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.f, 0.f, 1.f, 1.0f));
        break;

    case WHITE:
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.f, 1.f, 1.f, 1.0f));
        break;

    default:
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0., 170./255.,1., 1.0f));
        material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(112/510.0f, 14/510.0f, 255/510.0f, 1.0f));
        material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(249/255.0f,215/255.0f,255/255.0f, 1.0f));
        material->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
        break;
    }

    return material;
}
