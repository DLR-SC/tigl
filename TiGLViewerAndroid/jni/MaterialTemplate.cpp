#include "MaterialTemplate.h"
#include<osg/Array>
#include<algorithm>


osg::ref_ptr<osg::Material> MaterialTemplate::getMaterial(int number){
	osg::ref_ptr<osg::Material> material = new osg::Material();
	switch(number){

	case 1: 
		material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(176/255.0f, 255/255.0f, 153/255.0f, 1.0f));
		material->setShininess(osg::Material::FRONT_AND_BACK, 64.0f);
		material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(88/255.0f, 127/255.0f, 76/255.0f, 1.0f));
		material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(226/255.0f, 255/255.0f, 221/255.0f, 1.0f));
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
