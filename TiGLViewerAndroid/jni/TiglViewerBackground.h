/*
 * TiglViewerBackground.h
 *
 *  Created on: 05.09.2013
 *      Author: sigg_ma
 */

#ifndef TIGLVIEWERBACKGROUND_H_
#define TIGLVIEWERBACKGROUND_H_

#include <osg/Camera>

class TiglViewerBackground: public osg::Camera
{
public:
    TiglViewerBackground();

    void makeGradient(const osg::Vec4& col);
    void makeGradient(const osg::Vec4& lowCol, const osg::Vec4& upCol);

    virtual ~TiglViewerBackground();
};

#endif /* TIGLVIEWERBACKGROUND_H_ */
