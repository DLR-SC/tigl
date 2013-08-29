

#include <osg/MatrixTransform>
#include <osg/Camera>

class CrossNode : public osg::Camera
{
public:
    CrossNode();
    CrossNode(const CrossNode& copy, osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY);
    META_Node(osg, CrossNode);

    void setCrossBody(osg::MatrixTransform* body)
    {
        _crossBody = body;
    }

    osg::MatrixTransform* getCrossBody()
    {
        return _crossBody.get();
    }

    const osg::MatrixTransform* getCrossBody() const
    {
        return _crossBody.get();
    }

    void setMainCamera( osg::Camera* camera )
    {
        _mainCamera = camera;
    }

    osg::Camera* getMainCamera()
    {
        return _mainCamera.get();
    }
    const osg::Camera* getMainCamera() const
    {
        return _mainCamera.get();
    }

    virtual void traverse(osg::NodeVisitor& nv);

protected:
    virtual ~CrossNode();
    osg::ref_ptr<osg::MatrixTransform> _crossBody;
    osg::observer_ptr<osg::Camera> _mainCamera;

};
