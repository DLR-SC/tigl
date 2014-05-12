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
    if (!mgr) {
        osg::notify(osg::FATAL) << "Asset manager not yet defined. Cannot load font file" << std::endl;
        return NULL;
    }

    AAsset * asset = AAssetManager_open(mgr, filename, AASSET_MODE_UNKNOWN);
    std::stringstream str;
    if (asset) {
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

