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
#include "ModificatorContainerWidget.h"
#include "CPACSTreeWidget.h"
#include <QPushButton>

class TIGLViewerWindow;


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
    ModificatorManager(CPACSTreeWidget* treeWidget, ModificatorContainerWidget*  modificatorContainerWidget);

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

    // info for the user
    QWidget* noInterfaceWidget;

    // cancel/apply interface
    QWidget* widgetApply;
    QPushButton* commitButton;
    QPushButton* cancelButton;
};

#endif // TIGL_MODIFICATORMANAGER_H
