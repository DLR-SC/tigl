/*
* Copyright (C) 2007-2026 German Aerospace Center (DLR/SC)
*
* Created: 2026-09-02 Sven Goldberg <sven.goldberg@dlr.de>
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

#ifndef TIGLCREATOROTHERSWIDGET_H
#define TIGLCREATOROTHERSWIDGET_H

#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;
class QStackedWidget;
class QListWidget;
class QPushButton;
class QListWidgetItem;
class TIGLCreatorSpotlightManager;

class TIGLCreatorOthersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TIGLCreatorOthersWidget(QWidget* parent = nullptr);

    void setSpotlightManager(TIGLCreatorSpotlightManager* manager);

private slots:
    void onCategorySelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onAddSpotlight();
    void onEditSpotlight();
    void onCopySpotlight();
    void onDeleteSpotlight();
    void onSpotlightVisibChanged(QListWidgetItem* item);

private:
    QWidget* createSpotlightPanel();
    void refreshSpotlightList();

    QTreeWidget* myCategoryTree;
    QStackedWidget* myDetailStack;

    QListWidget* mySpotlightList;
    QPushButton* myAddButton;
    QPushButton* myEditButton;
    QPushButton* myCopyButton;
    QPushButton* myDeleteButton;

    TIGLCreatorSpotlightManager* mySpotlightManager;
    bool myIsRefreshingSpotlightList;
    bool myIsTogglingSpotlight;
};

#endif // TIGLCREATOROTHERSWIDGET_H
