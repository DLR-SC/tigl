#ifndef AUXILIARYVIEWUPDATER_H
#define AUXILIARYVIEWUPDATER_H

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <iostream>

class AuxiliaryViewUpdater : public osgGA::GUIEventHandler
{
public:
    AuxiliaryViewUpdater(): _distance(1.0), _offsetX(0.0f), _offsetY(0.0f), _lastDragX(-1.0f), _lastDragY(-1.0f) {}
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
double _distance;
float _offsetX, _offsetY;
float _lastDragX, _lastDragY;
int count;
};

#endif // AUXILIARYVIEWUPDATER_H
