#include <osg/Material>
#include <osg/Node>
#include <osg/Timer>


using namespace std;

class FadeOutCallback : public osg::NodeCallback
{
public:
    FadeOutCallback( osg::Material* mat );
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    void updateDiffuse(double t);
    osg::Material* _material;
    osg::Timer_t _intialTime;
};

