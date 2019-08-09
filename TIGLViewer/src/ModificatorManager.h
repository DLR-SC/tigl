/*
 * Copyright (C) 2018 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TIGL_MODIFICATORMANAGER_H
#define TIGL_MODIFICATORMANAGER_H

#include "TIGLViewerDocument.h"
#include "CPACSTreeItem.h"
#include "ModificatorContainerWidget.h"
#include "CPACSTreeWidget.h"
#include <QUndoStack>

class TIGLViewerWindow;

/**
 * @brief Main class to articulate the modification interface and the tree view.
 *
 * This class holds three main components:
 * CCPACSConfiguration: that is the main tigl object to work with.
 * CPACSTreeWidget: that manage the tree interface.
 * ModificatorContainerWidget: that is the interface that holds the specialized modificator for
 * particular tigl object.
 *
 * To work with this class, first we need to set the CCPACSConfiguration. The
 * CPACSTreeView will be updated and the correct tree will be build and
 * displayed. Once the CPACSConfiguration is set, it will wait for a
 * "newSelectedTreeItem" signal. If a "newSelectedTreeItem" signal is emit, the
 * "dispatch" function will be called. The dispatch function will look at the
 * new selected element and if the element has a associate ModificatorWidget, it
 * will set this particular ModificatorWidget by a call on the modificatorContainerWidget.
 *
 * @author Malo Drougard
 *
 */
class ModificatorManager : public QObject
{
    Q_OBJECT

signals:
    void configurationEdited();

public slots:
    void dispatch(cpcr::CPACSTreeItem* item);
    void createUndoCommand();
    void updateTree();

public:
    ModificatorManager(CPACSTreeWidget* treeWidget, ModificatorContainerWidget* modificatorContainerWidget, QUndoStack* undoStack);

    void setCPACSConfiguration(TIGLViewerDocument* newDoc);

protected:

    QStringList getAvailableFuselageProfileUIDs();  // we be managed by the profiles DB managed in future
    QStringList getAvailableWingProfileUIDs();

    inline bool configurationIsSet()
    {
        return (doc != nullptr && doc->getCpacsHandle() > 0);
    }

private:
    TIGLViewerDocument* doc;

    CPACSTreeWidget* treeWidget;
    ModificatorContainerWidget* modificatorContainerWidget;
    QUndoStack* myUndoStack;
};

#endif // TIGL_MODIFICATORMANAGER_H
