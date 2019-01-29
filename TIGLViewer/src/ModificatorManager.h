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

#include "CCPACSConfiguration.h"
#include "CPACSTreeItem.h"
#include "CPACSTreeView.h"
#include "ModificatorFuselageWidget.h"
#include "ModificatorTransformationWidget.h"
#include "ModificatorWidget.h"
#include "ModificatorWingWidget.h"

#include <QPushButton>

class TIGLViewerWindow;

/**
 * @brief Main class to articulate the modification interface.
 *
 * This class holds four main components:
 * CCPACSConfiguration: that is the main tigl object to work with.
 * CPACSTreeView: that is the manage the tree interface.
 * ModificatorWidgetS: that are the interfaces for particular tigl object.
 * ApllyWidget: that contains the commit and cancel button
 *
 * To work with this class, first we need to set the CCPACSConfiguration. The
 * CPACSTreeView will be updated and the correct tree will be build and
 * displayed. Oncethe CPACSConfiguration is set, it will wait for a
 * "newSelectedTreeItem" signal. If a "newSelectedTreeItem" signal is emit, the
 * "dispatch" function will be called. The dispatch function will look at the
 * new selected element and if the element has a associate ModificatorWidget, it
 * will set this particular ModificatorWidget and display it. Then if the user,
 * click on the "apply" function, the modificator manager will apply the change
 * onto the CPACSConfiguration though the ModificatorWidget. When the scene new
 * to be update because the CCPACSConfiguration was modify, the signal
 * "configurationEdited" is emit.
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
    void applyCurrentModifications();
    void applyCurrentCancellation();

public:
    ModificatorManager(QTreeView* qView, QWidget* applyInterface,
                       ModificatorTransformationWidget* transformationModificator,
                       ModificatorWingWidget* wingModificator, ModificatorFuselageWidget* fuselageModificator);

    void setCPACSConfiguration(tigl::CCPACSConfiguration* newConfig);

    void setTransformationModificator(cpcr::CPACSTreeItem* item);
    void setWingModificator(cpcr::CPACSTreeItem* item);
    void setFuselageModificator(cpcr::CPACSTreeItem* item);
    void hideAll();

protected:
    inline bool configurationIsSet()
    {
        return (config != nullptr);
    }

private:
    tigl::CCPACSConfiguration* config;

    CPACSTreeView* treeViewManager;

    ModificatorTransformationWidget* transformationModificator;
    ModificatorWingWidget* wingModificator;
    ModificatorFuselageWidget* fuselageModificator;

    ModificatorWidget* currentModificator;

    // cancel/apply interface
    QWidget* widgetApply;
    QPushButton* commitButton;
    QPushButton* cancelButton;
};

#endif // TIGL_MODIFICATORMANAGER_H
