#include "ViewInteractionsHandler.h"

#include <ostream>
#include <istream>
#include <osg/io_utils>


using namespace std;



ViewInteractionsHandler::ViewInteractionsHandler()
{
}

bool   ViewInteractionsHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);

    switch(ea.getEventType())
    {
    case(osgGA::GUIEventAdapter::SCROLL):
       {

           //EventHandler to zoomin When the scroll wheel is scrolled Up
            if(ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_UP)
            {

             osg::Matrixd zoomIn = view->getCamera()->getProjectionMatrix();
             zoomIn(0,0) *=1.5;
             zoomIn(1,1) *=1.5;

             view->getCamera()->setProjectionMatrix(zoomIn);


            }
            //EventHandler to zoomout When the scroll wheel is scrolled Down
            if(ea.getScrollingMotion() == osgGA::GUIEventAdapter::SCROLL_DOWN)
            {
              osg::Matrixd zoomOut = view->getCamera()->getProjectionMatrix();
              zoomOut(0,0) *=0.7;
              zoomOut(1,1) *=0.7;

             view->getCamera()->setProjectionMatrix(zoomOut);

            }

            return false;
        }
//    case(osgGA::GUIEventAdapter::RESIZE):
//        {
//        cout << view->getScene()->getSceneData()->asGroup()->getName() << endl;

//           double newW = ea.getGraphicsContext()->getTraits()->width/3;
//           double newH = ea.getGraphicsContext()->getTraits()->height/3;

//           int numberOfChildren = view->getScene()->getSceneData()->asGroup()->getNumChildren();

//           for (int i=0 ; i < numberOfChildren ; i++)
//           {
//               if(view->getScene()->getSceneData()->asGroup()->getChild(i)->getName() == "CrossNode")
//                  {
//                    view->getScene()->getSceneData()->asGroup()->getChild(i)->asCamera()->setViewport(
//                                                                              0,0,std::max(newW,newH),std::max(newW,newH));
//                  }
//           }

//            return false;
//        }
    default:
        return false;

    }


    return true;
}

