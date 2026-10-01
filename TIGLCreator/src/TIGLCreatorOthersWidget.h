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
class QPushButton;
class TIGLCreatorLightSourceManager;

class TIGLCreatorOthersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TIGLCreatorOthersWidget(QWidget* parent = nullptr);

    void setLightSourceManager(TIGLCreatorLightSourceManager* manager);

private slots:
    void onCategorySelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void onAddSpotlight();
    void onEditSpotlight();
    void onCopySpotlight();
    void onDeleteSpotlight();
    void onLightSourceItemChanged(QTreeWidgetItem* item, int column);

private:
    QWidget* createLightSourcePanel();
    void refreshLightSourceList();
    int currentLightSourceIndex() const;

    QTreeWidget* myCategoryTree;
    QStackedWidget* myDetailStack;

    QTreeWidget* myLightSourceTree;
    QPushButton* myAddButton;
    QPushButton* myEditButton;
    QPushButton* myCopyButton;
    QPushButton* myDeleteButton;

    TIGLCreatorLightSourceManager* myLightSourceManager;
    bool myIsRefreshingLightSourceList;
    bool myIsTogglingLightSource;
};

#endif // TIGLCREATOROTHERSWIDGET_H
