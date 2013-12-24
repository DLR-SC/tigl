#ifndef TIGLVIEWER_HUD_H
#define TIGLVIEWER_HUD_H


#include <osg/MatrixTransform>
#include <osg/Camera>

class TiglViewerHUD : public osg::Camera
{
public:
    TiglViewerHUD();
    TiglViewerHUD(const TiglViewerHUD& copy, osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY);

    // enables or disables the coordinate cross
    void setCoordinateCrossEnabled(bool);

    META_Node(osg, TiglViewerHUD);

    void setMainCamera( osg::Camera* camera )
    {
        _mainCamera = camera;
    }

protected:
    void init();

    virtual ~TiglViewerHUD();

    virtual void traverse(osg::NodeVisitor& nv);

    osg::ref_ptr<osg::MatrixTransform> _coordinateCross;
    osg::observer_ptr<osg::Camera> _mainCamera;

private:
    osg::MatrixTransform * createCoordinateCross();

};

#endif // TIGLVIEWER_HUD_H
