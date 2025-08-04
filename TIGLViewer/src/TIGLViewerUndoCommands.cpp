#include "TIGLViewerUndoCommands.h"

namespace TiGLViewer
{

void DeleteObjects::redo()
{
    for(Handle(AIS_InteractiveObject) obj : myObjects) {
        myContext->Remove(obj, Standard_False);
    }
    myContext->UpdateCurrentViewer();

}

void DeleteObjects::undo()
{
    for(Handle(AIS_InteractiveObject) obj : myObjects) {
       myContext->Display(obj, Standard_False);
    }
    myContext->UpdateCurrentViewer();

}

ChangeObjectsColor::ChangeObjectsColor(Handle(AIS_InteractiveContext) context,
                                       std::vector<Handle(AIS_InteractiveObject)> objects,
                                       QColor col)
    : myContext(context),
      myObjects(objects),
      myColor(col)
{
    setText("Change color");

    // store old colors
    for(Handle(AIS_InteractiveObject) obj : myObjects) {
       Quantity_Color oldCol;
       obj->Color(oldCol);
       myOldCols.push_back(oldCol);
    }
}

void ChangeObjectsColor::redo()
{
    for(Handle(AIS_InteractiveObject) obj : myObjects) {
       myContext->SetColor (obj, Quantity_Color(myColor.red()/255., myColor.green()/255., myColor.blue()/255., Quantity_TOC_RGB), Standard_False);
    }
    myContext->UpdateCurrentViewer();

}

void ChangeObjectsColor::undo()
{
    for (size_t iobj = 0; iobj < myObjects.size(); iobj++) {
        Handle(AIS_InteractiveObject) obj = myObjects[iobj];
        Quantity_Color& col = myOldCols[iobj];
        myContext->SetColor (obj, col, Standard_False);
    }
    myContext->UpdateCurrentViewer();

}

ModifyTiglObject::ModifyTiglObject(ModificatorManager& manager)
    : manager(manager)
{
    newConfig     = "";
    oldConfig     = "";
    isInitialized = false;
    setText("Creator edition");
}

void ModifyTiglObject::redo()
{
    if (!isInitialized) {
        initialize();
        isInitialized = true;
    }
    else {
        manager.updateCpacsConfigurationFromString(newConfig);
    }
}

void ModifyTiglObject::undo()
{
    manager.updateCpacsConfigurationFromString(oldConfig);
}

void ModifyTiglObject::initialize()
{
    oldConfig = manager.getConfigurationAsString();
    
    // write the new version
    manager.writeCPACS();

    newConfig = manager.getConfigurationAsString();
}

} // namespace TiGLViewer
