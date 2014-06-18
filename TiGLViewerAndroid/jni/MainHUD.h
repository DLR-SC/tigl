/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-06-11 Mahmoud Aly <Mahmoud.Aly@dlr.de>
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

#ifndef MAINHUD_H_
#define MAINHUD_H_

#include <osg/Camera>

class MainHUD : public osg::Camera
{
public:
    MainHUD();
    MainHUD(int screenWidth, int screenHeight);
    virtual ~MainHUD();
    void showCenterCross();
    void updateViewPort(int screenWidth, int screenHeight);

protected:
    int _screenWidth;
    int _screenHeight;

private:
    void init();
    void createCenterCross();
    osg::ref_ptr<Node> centerCross;
};

#endif /* MAINHUD_H_ */
