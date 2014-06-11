/*
 * mainHUD.h
 *
 *  Created on: 03.06.2014
 *      Author: aly_mm
 */

#ifndef MAINHUD_H_
#define MAINHUD_H_

#include <osg/Camera>

class mainHUD : public osg::Camera
{
public:
    mainHUD();
    mainHUD(int screenWidth, int screenHeight);
    virtual ~mainHUD();
    void showCenterCross();
    void updateViewPort(int screenWidth, int screenHeight);

protected:
    int _screenWidth;
    int _screenHeight;

private:
    void init();
    void createCenterCross();
    osg::Node* centerCross;
};

#endif /* MAINHUD_H_ */
