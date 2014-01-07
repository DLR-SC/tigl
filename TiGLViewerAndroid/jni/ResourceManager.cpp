/*
 * ResourceManager.cpp
 *
 *  Created on: 22.12.2013
 *      Author: sigg_ma
 */

#include "ResourceManager.h"

#include "OsgMainApp.hpp"
#include <android/asset_manager.h>
#include <sstream>
#include <istream>

ResourceManager::ResourceManager()
{
}

osg::ref_ptr<osgText::Font> ResourceManager::OpenFontFile(const char * filename)
{
    AAssetManager* mgr = OsgMainApp::Instance().getAssetManager();
    AAsset * asset = AAssetManager_open(mgr, filename, AASSET_MODE_UNKNOWN);
    std::stringstream str;
    if(asset) {
        char c;
        while(AAsset_read(asset, &c, 1) > 0) {
            str << c;
        }
        AAsset_close(asset);
    }

    osg::ref_ptr<osgText::Font> font = osgText::readRefFontStream(str);
    return font;
}

ResourceManager::~ResourceManager()
{
}

