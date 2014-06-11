/*
 * mainHUD.h
 *
 *  Created on: 03.06.2014
 *      Author: aly_mm
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
