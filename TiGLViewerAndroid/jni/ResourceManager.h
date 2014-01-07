/*
 * ResourceManager.h
 *
 *  Created on: 22.12.2013
 *      Author: sigg_ma
 */

#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

#include <istream>
#include <osg/ref_ptr>
#include <osgText/Font>

class ResourceManager {
public:
    ResourceManager();

    static osg::ref_ptr<osgText::Font> OpenFontFile(const char * filename);

    virtual ~ResourceManager();
};

#endif /* RESOURCEMANAGER_H_ */
