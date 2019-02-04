#ifndef TIGLVIEWERUNDOCOMMANDS_H
#define TIGLVIEWERUNDOCOMMANDS_H

#include "tigl_internal.h"

#include <QUndoCommand>
#include <QColor>
#include <vector>
#include <Quantity_Color.hxx>
#include <AIS_InteractiveObject.hxx>
#include <AIS_InteractiveContext.hxx>

namespace TiGLViewer
{

class DrawObjects : public QUndoCommand
{
public:
    DrawObjects(Handle(AIS_InteractiveContext) context, Handle(AIS_InteractiveObject) object, const std::string& objectName, bool updateViewer)
        : myContext(context)
        , myUpdate(updateViewer)
    {
        setText(QString("Draw %1").arg(objectName.c_str()));
        myObjects.push_back(object);
    }

    void redo()
    {
        for(Handle(AIS_InteractiveObject) obj : myObjects) {
            myContext->Display(obj, Standard_False);
        }
        if (myUpdate) {
            myContext->UpdateCurrentViewer();
        }
    }

    void undo()
    {
        myUpdate = true;
        for(Handle(AIS_InteractiveObject) obj : myObjects) {
            myContext->Erase(obj, Standard_False);
        }
        myContext->UpdateCurrentViewer();
    }

    bool mergeWith(const QUndoCommand* other)
    {
        if (other->id() != id()) {
            return false;
        }

        setText("Draw Objects");

        for(Handle(AIS_InteractiveObject) obj : static_cast<const DrawObjects*>(other)->myObjects) {
            myObjects.push_back(obj);
        }
        return true;
    }

    int id() const OVERRIDE {return 1002;}

    private:
        Handle(AIS_InteractiveContext) myContext;
        std::vector<Handle(AIS_InteractiveObject)> myObjects;
        bool myUpdate;
};

class DeleteObjects : public QUndoCommand
{
public:
    DeleteObjects(Handle(AIS_InteractiveContext) context, std::vector<Handle(AIS_InteractiveObject)> objects)
        : myContext(context),
          myObjects(objects)
    {
        setText("Delete objects");
    }

    // QUndoCommand interface
public:
    void redo() OVERRIDE;
    void undo() OVERRIDE;
    int id() const OVERRIDE {return 1000;}

private:
    Handle(AIS_InteractiveContext) myContext;
    std::vector<Handle(AIS_InteractiveObject)> myObjects;
};

class ChangeObjectsColor : public QUndoCommand
{
public:
    ChangeObjectsColor(Handle(AIS_InteractiveContext) context,
                       std::vector<Handle(AIS_InteractiveObject)> objects,
                       QColor col);

    // QUndoCommand interface
public:
    void redo() OVERRIDE;
    void undo() OVERRIDE;
    int id() const OVERRIDE {return 1001;}

private:
    Handle(AIS_InteractiveContext) myContext;
    std::vector<Handle(AIS_InteractiveObject)> myObjects;
    QColor myColor;
    std::vector<Quantity_Color> myOldCols;
};


}
#endif // TIGLVIEWERUNDOCOMMANDS_H
