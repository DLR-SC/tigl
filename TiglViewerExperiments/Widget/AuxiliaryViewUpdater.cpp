#include "AuxiliaryViewUpdater.h"

#include <ostream>
#include <istream>
#include <osg/io_utils>


using namespace std;


bool AuxiliaryViewUpdater::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa){

    osgViewer::View* view = static_cast<osgViewer::View*>(&aa);
    if ( view )
    {
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::PUSH:
            _lastDragX = -1.0f;
            _lastDragY = -1.0f;
            break;
        case osgGA::GUIEventAdapter::DRAG:
            if ( _lastDragX>0.0f && _lastDragY>0.0f )
            {
                if ( ea.getButtonMask()==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON )
                {
                    _offsetX += ea.getX() - _lastDragX;
                    _offsetY += ea.getY() - _lastDragY;
                }
                else if ( ea.getButtonMask()==osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON )
                {
                    float dy = ea.getY() - _lastDragY;
                    _distance *= 1.0 + dy / ea.getWindowHeight();
                    if ( _distance<1.0 ) _distance = 1.0;
                }
            }
            _lastDragX = ea.getX();
            _lastDragY = ea.getY();
            break;
        case osgGA::GUIEventAdapter::FRAME:
            if ( view->getCamera() )
            {
                osg::Vec3d eye, center, up;
                view->getCamera()->getViewMatrixAsLookAt( eye, center, up );

                osg::Vec3d lookDir = center - eye; lookDir.normalize();
                osg::Vec3d side = lookDir ^ up; side.normalize();

                const osg::BoundingSphere& bs = view->getSceneData()->getBound();
                if ( _distance<0.0 ) _distance = bs.radius() * 3.0;
                center = bs.center();

                center -= (side * _offsetX + up * _offsetY) * 0.1;
                view->getCamera()->setViewMatrixAsLookAt( center-lookDir*_distance, center, up );
            }
            break;
        }
    }
    return false;
}

