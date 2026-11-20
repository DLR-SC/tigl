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
#include "TIGLCreatorLightSourceManager.h"
#include "TIGLCreatorAddSpotlightDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QSplitter>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QStyleOption>
#include <QMouseEvent>
#include <gp_Pnt.hxx>

namespace {

/// Renders the check state of an item centered in its rect instead of
/// at the left edge as the default item delegate does, and makes the
/// centered check box the clickable toggle area.
class CenteredCheckDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        QStyleOptionViewItem viewOption = option;
        initStyleOption(&viewOption, index);

        // Determine the check box rect first: subElementRect only returns
        // a valid rect while the HasCheckIndicator feature is still set
        const Qt::CheckState checkState = viewOption.checkState;
        const QRect rect = checkRect(viewOption);

        // Suppress the check indicator of the default rendering, otherwise
        // the check box would also be painted at the left edge
        // (QStyledItemDelegate::paint cannot be used, it re-initializes
        // the option from the index)
        viewOption.checkState = Qt::Unchecked;
        viewOption.features &= ~QStyleOptionViewItem::HasCheckIndicator;
        viewOption.widget->style()->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);

        QStyleOptionButton buttonOption;
        buttonOption.palette = viewOption.palette;
        buttonOption.state = viewOption.state;
        if (checkState == Qt::Checked) {
            buttonOption.state |= QStyle::State_On;
        }
        buttonOption.rect = rect;
        painter->save();
        painter->setClipRect(viewOption.rect);
        viewOption.widget->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &buttonOption, painter, viewOption.widget);
        painter->restore();
    }

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                     const QStyleOptionViewItem& option, const QModelIndex& index) override
    {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            // Use the same rect initialization as in paint()
            QStyleOptionViewItem viewOption = option;
            initStyleOption(&viewOption, index);
            if (mouseEvent->button() == Qt::LeftButton &&
                    checkRect(viewOption).adjusted(-2, -2, 2, 2).contains(mouseEvent->pos())) {
                const Qt::CheckState oldState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
                return model->setData(index,
                                       static_cast<int>(oldState == Qt::Checked ? Qt::Unchecked : Qt::Checked),
                                       Qt::CheckStateRole);
            }
            // A press anywhere else must not toggle the check state, the
            // base implementation would do so for the left edge indicator
            return false;
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

private:
    static QRect checkRect(const QStyleOptionViewItem& option)
    {
        // Start from the indicator rectangle the style itself uses for
        // the default (left aligned) check box, so the vertical position
        // matches the default rendering exactly, and only center it
        // horizontally within the item
        QRect rect = option.widget->style()->subElementRect(
            QStyle::SE_ItemViewItemCheckIndicator, &option, option.widget);
        rect.moveLeft(option.rect.x() + (option.rect.width() - rect.width()) / 2);
        return rect;
    }
};

}

TIGLCreatorOthersWidget::TIGLCreatorOthersWidget(QWidget* parent)
    : QWidget(parent)
    , myLightSourceManager(nullptr)
    , myIsRefreshingLightSourceList(false)
    , myIsTogglingLightSource(false)
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

    // Create the light source panel (index 1 in the stack)
    myDetailStack->addWidget(createLightSourcePanel());

    // Add "Manage Light Sources" category
    QTreeWidgetItem* lightSourceItem = new QTreeWidgetItem(myCategoryTree);
    lightSourceItem->setText(0, "Manage Light Sources");
    lightSourceItem->setData(0, Qt::UserRole, 1);
    myCategoryTree->addTopLevelItem(lightSourceItem);
    lightSourceItem->setExpanded(true);

    // No item selected by default - placeholder is shown
    myDetailStack->setCurrentIndex(0);

    connect(myCategoryTree, &QTreeWidget::currentItemChanged,
            this, &TIGLCreatorOthersWidget::onCategorySelectionChanged);
}

void TIGLCreatorOthersWidget::setLightSourceManager(TIGLCreatorLightSourceManager* manager)
{
    if (myLightSourceManager) {
        disconnect(myLightSourceManager, &TIGLCreatorLightSourceManager::spotlightsChanged,
                   this, &TIGLCreatorOthersWidget::refreshLightSourceList);
    }

    myLightSourceManager = manager;

    if (myLightSourceManager) {
        connect(myLightSourceManager, &TIGLCreatorLightSourceManager::spotlightsChanged,
                this, &TIGLCreatorOthersWidget::refreshLightSourceList);
        refreshLightSourceList();
    }
}

QWidget* TIGLCreatorOthersWidget::createLightSourcePanel()
{
    QWidget* panel = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    // Spotlight tree: first column toggles the spotlight, second column toggles the 3D cone
    // visibility, the wide third column lists the spotlight names
    myLightSourceTree = new QTreeWidget();
    myLightSourceTree->setColumnCount(3);
    myLightSourceTree->setHeaderLabels(QStringList() << "Show Light Source" << "Show Cone" << "Light Sources");
    myLightSourceTree->setRootIsDecorated(false);
    myLightSourceTree->setUniformRowHeights(true);
    myLightSourceTree->setSelectionMode(QAbstractItemView::SingleSelection);
    myLightSourceTree->setItemDelegateForColumn(0, new CenteredCheckDelegate());
    myLightSourceTree->setItemDelegateForColumn(1, new CenteredCheckDelegate());
    myLightSourceTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    myLightSourceTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    layout->addWidget(myLightSourceTree, 1);

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
    connect(myLightSourceTree, &QTreeWidget::itemSelectionChanged, this, [this]() {
        // currentLightSourceIndex returns -1 for the fixed "TiGL Default" row,
        // which must not be editable/copyable/deletable
        bool hasSelection = currentLightSourceIndex() >= 0;
        myEditButton->setEnabled(hasSelection);
        myCopyButton->setEnabled(hasSelection);
        myDeleteButton->setEnabled(hasSelection);
    });
    connect(myLightSourceTree, &QTreeWidget::itemChanged,
            this, &TIGLCreatorOthersWidget::onLightSourceItemChanged);

    return panel;
}

int TIGLCreatorOthersWidget::currentLightSourceIndex() const
{
    if (!myLightSourceTree || !myLightSourceTree->currentItem()) {
        return -1;
    }
    // The fixed "TiGL Default" row is tagged with -1, all spotlight rows with their index
    const QVariant indexVariant = myLightSourceTree->currentItem()->data(0, Qt::UserRole);
    return indexVariant.isValid() ? indexVariant.toInt() : -1;
}

void TIGLCreatorOthersWidget::onCategorySelectionChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
    if (!current) {
        myDetailStack->setCurrentIndex(0);
        return;
    }
    int index = current->data(0, Qt::UserRole).toInt();
    if (index < 0 || index >= myDetailStack->count()) {
        myDetailStack->setCurrentIndex(0);
        return;
    }
    myDetailStack->setCurrentIndex(index);
}

void TIGLCreatorOthersWidget::onAddSpotlight()
{
    if (!myLightSourceManager) {
        return;
    }

    TIGLCreatorAddSpotlightDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    tigl::CTiglPoint pos = dialog.getPosition();
    tigl::CTiglPoint dir = dialog.getDirection();
    double conc = dialog.getConcentration();

    myLightSourceManager->addSpotlight(pos.x, pos.y, pos.z, dir.x, dir.y, dir.z, conc);
}

void TIGLCreatorOthersWidget::onEditSpotlight()
{
    if (!myLightSourceManager) {
        return;
    }

    int row = currentLightSourceIndex();
    if (row < 0) {
        return;
    }

    const QList<SpotlightData>& spotlights = myLightSourceManager->getSpotlights();
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

    myLightSourceManager->updateSpotlight(row, newPos.x, newPos.y, newPos.z, newDir.x, newDir.y, newDir.z, newConc);
}

void TIGLCreatorOthersWidget::onCopySpotlight()
{
    if (!myLightSourceManager) {
        return;
    }

    int row = currentLightSourceIndex();
    if (row < 0) {
        return;
    }

    const QList<SpotlightData>& spotlights = myLightSourceManager->getSpotlights();
    if (row >= spotlights.size()) {
        return;
    }

    myLightSourceManager->copySpotlight(row);
}

void TIGLCreatorOthersWidget::onDeleteSpotlight()
{
    if (!myLightSourceManager) {
        return;
    }

    int row = currentLightSourceIndex();
    if (row < 0) {
        return;
    }

    const QList<SpotlightData>& spotlights = myLightSourceManager->getSpotlights();
    if (row >= spotlights.size()) {
        return;
    }

    const SpotlightData& data = spotlights[row];

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete Spotlight",
        QString("Are you sure you want to delete \"%1\"?").arg(data.name),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        myLightSourceManager->removeSpotlight(row);
    }
}

void TIGLCreatorOthersWidget::onLightSourceItemChanged(QTreeWidgetItem* item, int column)
{
    if (!myLightSourceManager || !item || myIsRefreshingLightSourceList || myIsTogglingLightSource) {
        return;
    }

    const QVariant indexVariant = item->data(0, Qt::UserRole);
    if (!indexVariant.isValid()) {
        return;
    }
    const int index = indexVariant.toInt();
    // The fixed "TiGL Default" row has no symbol checkbox (-> column 0 only)
    if (column == 1 && index == -1) {
        return;
    }
    if (column != 0 && column != 1) {
        return;
    }

    bool newState = (item->checkState(column) == Qt::Checked);
    Qt::CheckState previousState = newState ? Qt::Unchecked : Qt::Checked;

    myIsTogglingLightSource = true;
    bool success = false;
    if (index == -1) {
        success = myLightSourceManager->setDefaultLightEnabled(newState);
    }
    else if (column == 0) {
        success = myLightSourceManager->setSpotlightEnabled(index, newState);
    }
    else {
        success = myLightSourceManager->setSpotlightSymbolVisible(index, newState);
    }
    if (!success) {
        item->setCheckState(column, previousState);
    }
    myIsTogglingLightSource = false;
}

void TIGLCreatorOthersWidget::refreshLightSourceList()
{
    if (!myLightSourceTree || !myLightSourceManager || myIsRefreshingLightSourceList || myIsTogglingLightSource) {
        return;
    }

    myIsRefreshingLightSourceList = true;

    QString currentName;
    if (myLightSourceTree->currentItem()) {
        currentName = myLightSourceTree->currentItem()->text(2);
    }

    // Rebuild whole list since after change (add, edit, delete) it is not clear which spotlight changed
    myLightSourceTree->clear();

    // The fixed viewer-level default lights as a hard-wired entry: on/off only,
    // no cone checkbox (column 1 is left without a check state on purpose)
    QTreeWidgetItem* defaultItem = new QTreeWidgetItem();
    defaultItem->setFlags(defaultItem->flags() | Qt::ItemIsUserCheckable);
    defaultItem->setText(2, "TiGL Default");
    defaultItem->setData(0, Qt::UserRole, -1);
    defaultItem->setCheckState(0, myLightSourceManager->isDefaultLightEnabled() ? Qt::Checked : Qt::Unchecked);
    myLightSourceTree->addTopLevelItem(defaultItem);

    const QList<SpotlightData>& spotlights = myLightSourceManager->getSpotlights();
    for (int i = 0; i < spotlights.size(); ++i) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setText(2, spotlights[i].name);
        item->setData(0, Qt::UserRole, i);
        item->setCheckState(0, myLightSourceManager->isSpotlightEnabled(i) ? Qt::Checked : Qt::Unchecked);
        item->setCheckState(1, myLightSourceManager->isSpotlightSymbolVisible(i) ? Qt::Checked : Qt::Unchecked);
        myLightSourceTree->addTopLevelItem(item);
    }

    myIsRefreshingLightSourceList = false;

    // Try to restore the previously selected spotlight by name
    if (!currentName.isEmpty()) {
        for (int i = 0; i < myLightSourceTree->topLevelItemCount(); ++i) {
            if (myLightSourceTree->topLevelItem(i)->text(2) == currentName) {
                myLightSourceTree->setCurrentItem(myLightSourceTree->topLevelItem(i));
                break;
            }
        }
    }
}
