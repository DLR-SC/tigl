#ifndef RENDERINGTHREAD_H
#define RENDERINGTHREAD_H

#include <QtCore/QtCore>
#include <osgViewer/Viewer>


class RenderingThread : public QThread

{
public:
    RenderingThread() : QThread(), viewerPtr(0) {}

    virtual ~RenderingThread()
    { if (viewerPtr) viewerPtr->setDone(true); wait(); }

    osgViewer::Viewer* viewerPtr;

protected:
    virtual void run()
    { if (viewerPtr) viewerPtr->run(); }
};
#endif // RENDERINGTHREAD_H
