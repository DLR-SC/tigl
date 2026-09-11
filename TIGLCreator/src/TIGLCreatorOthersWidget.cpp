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

#include "TIGLCreatorOthersWidget.h"
#include "TIGLCreatorSpotlightManager.h"
#include "TIGLCreatorAddSpotlightDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QSplitter>
#include <gp_Pnt.hxx>

TIGLCreatorOthersWidget::TIGLCreatorOthersWidget(QWidget* parent)
    : QWidget(parent)
    , mySpotlightManager(nullptr)
    , myIsRefreshingSpotlightList(false)
    , myIsTogglingSpotlight(false)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter* mainSplitter = new QSplitter(Qt::Vertical);
    mainSplitter->setChildrenCollapsible(false);
    mainLayout->addWidget(mainSplitter);

    // Category tree at the top
    myCategoryTree = new QTreeWidget();
    myCategoryTree->setHeaderHidden(true);
    mainSplitter->addWidget(myCategoryTree);

    // Detail stack at the bottom
    myDetailStack = new QStackedWidget();
    mainSplitter->addWidget(myDetailStack);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setSizes(QList<int>() << 24 << 300);

    // Placeholder page (index 0 in the stack) - shown when nothing is selected
    QWidget* placeholder = new QWidget();
    QVBoxLayout* placeholderLayout = new QVBoxLayout(placeholder);
    placeholderLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* placeholderLabel = new QLabel("Please select a category above to manage its entries");
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLayout->addWidget(placeholderLabel, 1);
    myDetailStack->addWidget(placeholder);

    // Create the spotlight panel (index 1 in the stack)
    myDetailStack->addWidget(createSpotlightPanel());

    // Add "Manage Spotlights" category
    QTreeWidgetItem* spotlightItem = new QTreeWidgetItem(myCategoryTree);
    spotlightItem->setText(0, "Manage Spotlights");
    spotlightItem->setData(0, Qt::UserRole, 1);
    myCategoryTree->addTopLevelItem(spotlightItem);
    spotlightItem->setExpanded(true);

    // No item selected by default - placeholder is shown
    myDetailStack->setCurrentIndex(0);

    connect(myCategoryTree, &QTreeWidget::currentItemChanged,
            this, &TIGLCreatorOthersWidget::onCategorySelectionChanged);
}

void TIGLCreatorOthersWidget::setSpotlightManager(TIGLCreatorSpotlightManager* manager)
{
    if (mySpotlightManager) {
        disconnect(mySpotlightManager, &TIGLCreatorSpotlightManager::spotlightsChanged,
                   this, &TIGLCreatorOthersWidget::refreshSpotlightList);
    }

    mySpotlightManager = manager;

    if (mySpotlightManager) {
        connect(mySpotlightManager, &TIGLCreatorSpotlightManager::spotlightsChanged,
                this, &TIGLCreatorOthersWidget::refreshSpotlightList);
        refreshSpotlightList();
    }
}

QWidget* TIGLCreatorOthersWidget::createSpotlightPanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    // Spotlight list
    mySpotlightList = new QListWidget();
    layout->addWidget(mySpotlightList, 1);

    // Button row
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    myAddButton = new QPushButton("Add");
    myEditButton = new QPushButton("Edit");
    myCopyButton = new QPushButton("Copy");
    myDeleteButton = new QPushButton("Delete");
    buttonLayout->addWidget(myAddButton);
    buttonLayout->addWidget(myEditButton);
    buttonLayout->addWidget(myCopyButton);
    buttonLayout->addWidget(myDeleteButton);
    layout->addLayout(buttonLayout);

    // Initially disable edit/copy/delete (nothing selected)
    myEditButton->setEnabled(false);
    myCopyButton->setEnabled(false);
    myDeleteButton->setEnabled(false);

    connect(myAddButton, &QPushButton::clicked, this, &TIGLCreatorOthersWidget::onAddSpotlight);
    connect(myEditButton, &QPushButton::clicked, this, &TIGLCreatorOthersWidget::onEditSpotlight);
    connect(myCopyButton, &QPushButton::clicked, this, &TIGLCreatorOthersWidget::onCopySpotlight);
    connect(myDeleteButton, &QPushButton::clicked, this, &TIGLCreatorOthersWidget::onDeleteSpotlight);
    connect(mySpotlightList, &QListWidget::currentRowChanged, this, [this](int) {
        bool hasSelection = mySpotlightList->currentRow() >= 0;
        myEditButton->setEnabled(hasSelection);
        myCopyButton->setEnabled(hasSelection);
        myDeleteButton->setEnabled(hasSelection);
    });
    connect(mySpotlightList, &QListWidget::itemChanged,
            this, &TIGLCreatorOthersWidget::onSpotlightVisibChanged);

    return panel;
}

void TIGLCreatorOthersWidget::onCategorySelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
    if (!current) {
        myDetailStack->setCurrentIndex(0);
        return;
    }
    int index = current->data(0, Qt::UserRole).toInt();
    myDetailStack->setCurrentIndex(index);
}

void TIGLCreatorOthersWidget::onAddSpotlight()
{
    if (!mySpotlightManager) {
        return;
    }

    TIGLCreatorAddSpotlightDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    tigl::CTiglPoint pos = dialog.getPosition();
    tigl::CTiglPoint dir = dialog.getDirection();
    double conc = dialog.getConcentration();

    mySpotlightManager->addSpotlight(pos.x, pos.y, pos.z, dir.x, dir.y, dir.z, conc);
}

void TIGLCreatorOthersWidget::onEditSpotlight()
{
    if (!mySpotlightManager) {
        return;
    }

    int row = mySpotlightList->currentRow();
    if (row < 0) {
        return;
    }

    QList<SpotlightData> spotlights = mySpotlightManager->getSpotlights();
    if (row >= spotlights.size()) {
        return;
    }

    const SpotlightData& data = spotlights[row];
    gp_Pnt pos = data.light->Position();
    double conc = data.light->Concentration();

    // Direction is read from the stored value, not from the OCCT handle.
    // OCCT normalizes the direction and would show different values than the user entered
    TIGLCreatorAddSpotlightDialog dialog(
        pos.X(), pos.Y(), pos.Z(),
        data.direction.X(), data.direction.Y(), data.direction.Z(),
        conc, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    tigl::CTiglPoint newPos = dialog.getPosition();
    tigl::CTiglPoint newDir = dialog.getDirection();
    double newConc = dialog.getConcentration();

    mySpotlightManager->updateSpotlight(row, newPos.x, newPos.y, newPos.z, newDir.x, newDir.y, newDir.z, newConc);
}

void TIGLCreatorOthersWidget::onCopySpotlight()
{
    if (!mySpotlightManager) {
        return;
    }

    int row = mySpotlightList->currentRow();
    if (row < 0) {
        return;
    }

    mySpotlightManager->copySpotlight(row);
}

void TIGLCreatorOthersWidget::onDeleteSpotlight()
{
    if (!mySpotlightManager) {
        return;
    }

    int row = mySpotlightList->currentRow();
    if (row < 0) {
        return;
    }

    QList<SpotlightData> spotlights = mySpotlightManager->getSpotlights();
    if (row >= spotlights.size()) {
        return;
    }

    const SpotlightData& data = spotlights[row];

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete Spotlight",
        QString("Are you sure you want to delete \"%1\"?").arg(data.name),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        mySpotlightManager->removeSpotlight(row);
    }
}

void TIGLCreatorOthersWidget::onSpotlightVisibChanged(QListWidgetItem* item)
{
    if (!mySpotlightManager || !item || myIsRefreshingSpotlightList || myIsTogglingSpotlight) {
        return;
    }

    int row = mySpotlightList->row(item);
    if (row < 0) {
        return;
    }

    bool newState = (item->checkState() == Qt::Checked);
    Qt::CheckState previousState = newState ? Qt::Unchecked : Qt::Checked;

    myIsTogglingSpotlight = true;
    bool success = mySpotlightManager->setSpotlightEnabled(row, newState);
    if (!success) {
        item->setCheckState(previousState);
    }
    myIsTogglingSpotlight = false;
}

void TIGLCreatorOthersWidget::refreshSpotlightList()
{
    if (!mySpotlightList || myIsRefreshingSpotlightList || myIsTogglingSpotlight) {
        return;
    }

    myIsRefreshingSpotlightList = true;

    QString currentName;
    if (mySpotlightList->currentRow() >= 0) {
        currentName = mySpotlightList->currentItem()->text();
    }

    // Rebuild whole list since after change (add, edit, delete) it is not clear which spotlight changed
    mySpotlightList->clear();

    if (mySpotlightManager) {
        QList<SpotlightData> spotlights = mySpotlightManager->getSpotlights();
        for (const auto& s : spotlights) {
            QListWidgetItem* item = new QListWidgetItem(s.name);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(s.enabled ? Qt::Checked : Qt::Unchecked);
            mySpotlightList->addItem(item);
        }
    }

    myIsRefreshingSpotlightList = false;

    // Try to restore the previously selected spotlight by name
    if (!currentName.isEmpty()) {
        for (int i = 0; i < mySpotlightList->count(); ++i) {
            if (mySpotlightList->item(i)->text() == currentName) {
                mySpotlightList->setCurrentRow(i);
                break;
            }
        }
    }
}
