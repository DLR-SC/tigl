
#include "CrossNode.h"
#include <osg/Matrixd>


CrossNode::CrossNode(){}
CrossNode::CrossNode(const CrossNode &copy, osg::CopyOp copyop):
    osg::Camera(copy, copyop),
    _crossBody(copy._crossBody),
    _mainCamera(copy._mainCamera){}
CrossNode::~CrossNode(){}

void CrossNode::traverse(osg::NodeVisitor &nv)
{
    if ( _mainCamera.valid() && nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR )
       {
           osg::Matrix matrix = _mainCamera->getViewMatrix();
           matrix.setTrans( osg::Vec3() );

           osg::Vec3 trans, scale;
           osg::Quat rot, orient;
           matrix.decompose(trans, rot, scale, orient);

           osg::Matrix res(rot);
           res.postMult(osg::Matrix::translate(-0.5,-0.5,-10));


           if ( _crossBody.valid() )
               _crossBody->setMatrix( osg::Matrix(res) );//osg::Matrix::rotate(angle, axis) );
       }

       _crossBody->accept( nv );
       osg::Camera::traverse( nv );
}
