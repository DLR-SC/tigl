/*
* Copyright (C) 2025 German Aerospace Center (DLR/SC)
*
* Created: 2025-11-12 Ole Albers <ole.albers@dlr.de>
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

#include "ModificatorDisplayOptionsWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QSlider>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QColorDialog>
#include "../TIGLCreatorDocument.h"
#include <CPACSTreeItem.h>
#include "TIGLCreatorMaterials.h"
#include "../TIGLCreatorContext.h"
#include "ui_ModificatorDisplayOptionsWidget.h"


ModificatorDisplayOptionsWidget::ModificatorDisplayOptionsWidget(QWidget* parent)
    : CpacsEditorBase(parent)
{
    ui = new Ui::ModificatorDisplayOptionsWidget();
    ui->setupUi(this);

    infoLabel = ui->infoLabel;
    transparencySlider = ui->transparencySlider;
    renderingModeCombo = ui->renderingModeCombo;
    colorButton = ui->colorButton;
    materialCombo = ui->materialCombo;

    renderingModeCombo->addItem("Wireframe", 0);
    renderingModeCombo->addItem("Shaded", 1);
    // populate materials list after ui setup
    materialCombo->clear();
    for (const auto &kv : tiglMaterials::materialMap) {
        materialCombo->addItem(kv.first);
    }

    // interactions
    connect(colorButton, &QPushButton::clicked, this, &ModificatorDisplayOptionsWidget::onColorChosen);

    // immediate apply connections (display-only)
    connect(transparencySlider, &QSlider::valueChanged, this, &ModificatorDisplayOptionsWidget::onTransparencyChanged);
    // use idClicked (not deprecated) instead of buttonClicked
    connect(renderingModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModificatorDisplayOptionsWidget::onRenderingModeChanged);
    connect(materialCombo, &QComboBox::currentTextChanged, this, &ModificatorDisplayOptionsWidget::onMaterialChanged);
}

ModificatorDisplayOptionsWidget::~ModificatorDisplayOptionsWidget() = default;

void ModificatorDisplayOptionsWidget::setFromItem(cpcr::CPACSTreeItem* item, TIGLCreatorDocument* doc, TIGLCreatorContext* context)
{

    currentItem = item;
    currentDoc = doc;
    currentContext = context;

    if (!item) {
        // No item selected: show info text and hide interactive controls
        infoLabel->setText(tr("Please select an editable element from the CPACS Tree."));

        if (ui) {
            ui->labelTransparency->setVisible(false);
            ui->transparencySlider->setVisible(false);
            ui->labelRenderingMode->setVisible(false);
            ui->renderingModeCombo->setVisible(false);
            ui->colorButton->setVisible(false);
            ui->labelMaterial->setVisible(false);
            ui->materialCombo->setVisible(false);
        }
        currentItem = nullptr;
        return;
    }

    if (ui) {
        ui->labelTransparency->setVisible(true);
        ui->transparencySlider->setVisible(true);
        ui->labelRenderingMode->setVisible(true);
        ui->renderingModeCombo->setVisible(true);
        ui->colorButton->setVisible(true);
        ui->labelMaterial->setVisible(true);
        ui->materialCombo->setVisible(true);
    }
    
    const QString type = QString::fromStdString(item->getType());
    const QString uid = QString::fromStdString(item->getUid());


    // get current values
    auto &sm = context->GetShapeManager();
    if (sm.HasShapeEntry(uid.toStdString())) {
        auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
        
        auto obj = objs[0];
        Standard_Real transparency;
        int displayMode;

        if (!obj.IsNull()) {
            Handle(Prs3d_Drawer) drawer = obj->Attributes();
            transparency = drawer->Transparency(); 
            displayMode = drawer->DisplayMode();

        }
        transparencySlider->setValue(transparency*100);
        renderingModeCombo->setCurrentIndex(displayMode);

    }

    materialCombo->setCurrentIndex(0);
    
    infoLabel->setText(uid.isEmpty() ? tr("Group: %1").arg(type) : tr("Item: %1 (%2)").arg(type, uid));

}

bool ModificatorDisplayOptionsWidget::apply()
{
    if (!currentContext) {
        return false;
    }

    if (currentItem) {
        const QString uid = QString::fromStdString(currentItem->getUid());
        if (!uid.isEmpty()) {
            auto &sm = currentContext->GetShapeManager();
            if (sm.HasShapeEntry(uid.toStdString())) {
                auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
                
                auto obj = objs[0];
                Standard_Real transparency;
                int displayMode;
                if (!obj.IsNull()) {
                    Handle(Prs3d_Drawer) drawer = obj->Attributes();
                    transparency = drawer->Transparency(); 
                    displayMode = drawer->DisplayMode();
                }
                transparency = transparencySlider ? transparencySlider->value()*0.01 : transparency;

                QColor qc = colorButton ? colorButton->palette().color(QPalette::Button) : QColor();
                Quantity_Color qcolor;
                bool haveColor = false;
                if (qc.isValid()) {
                    qcolor = Quantity_Color(qc.redF(), qc.greenF(), qc.blueF(), Quantity_TOC_RGB);
                    haveColor = true;
                }

                QString mat = materialCombo ? materialCombo->currentText() : QString();
                auto matIt = tiglMaterials::materialMap.end();
                if (!mat.isEmpty()) matIt = tiglMaterials::materialMap.find(mat);

                auto context = currentContext->getContext();
                for (auto &obj : objs) {
                    if (obj.IsNull()) {
                        continue;
                    }
                    if (!context.IsNull()) {
                        // transparency
                        context->SetTransparency(obj, transparency, Standard_True);
                        // display mode
                        context->SetDisplayMode(obj, displayMode, Standard_True);
                        
                        // color
                        if (haveColor) {
                            context->SetColor(obj, qcolor, Standard_True);
                        }
                        // material
                        if (matIt != tiglMaterials::materialMap.end()) {
                            context->SetMaterial(obj, matIt->second, Standard_True);
                        }
                    }
                }

                // Update viewer after batch changes
                if (!context.IsNull()) {
                    currentContext->updateViewer();
                }

                return false;
            }
        }
    }

    return false;
}

void ModificatorDisplayOptionsWidget::reset()
{
    setFromItem(currentItem, currentDoc);
}

void ModificatorDisplayOptionsWidget::onTransparencyChanged(int value)
{
    if (!currentContext) {
        return;
    }
    if (!currentItem) {
        return;
    }
    const QString uid = QString::fromStdString(currentItem->getUid());
    if (uid.isEmpty()) {
        return;
    }
    auto &sm = currentContext->GetShapeManager();
    if (!sm.HasShapeEntry(uid.toStdString()) && currentDoc) {
        currentDoc->drawComponentByUID(uid);
    }
    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    Standard_Real tr = value * 0.01;
    auto ctx = currentContext->getContext();
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
        if (!ctx.IsNull()) {
            ctx->SetTransparency(obj, tr, Standard_True);
        }
    }
    if (!currentContext->getContext().IsNull()) {
        currentContext->updateViewer();
    }
}

void ModificatorDisplayOptionsWidget::onRenderingModeChanged(int displayMode)
{
    if (!currentContext) {
        return;
    }
    if (!currentItem) {
        return;
    }
    const QString uid = QString::fromStdString(currentItem->getUid());
    if (uid.isEmpty()) {
        return;
    }
    auto &sm = currentContext->GetShapeManager();
    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    auto ctx = currentContext->getContext();
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
        if (!ctx.IsNull()) {
            ctx->SetDisplayMode(obj, displayMode, Standard_True);
        }
    }
    if (!currentContext->getContext().IsNull()) {
        currentContext->updateViewer();
    }
}

void ModificatorDisplayOptionsWidget::onColorChosen()
{

    if (!currentContext) {
        return;
    }
    if (!currentItem) {
        return;
    }
    QColor initial;
    const QString uid = QString::fromStdString(currentItem->getUid());
    auto &sm = currentContext->GetShapeManager();
    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    if (!objs.empty()) {
        auto obj = objs[0];
        if (!obj.IsNull()) {
            Handle(Prs3d_Drawer) drawer = obj->Attributes();
            Quantity_Color qc = drawer->Color();
            initial = QColor::fromRgbF(qc.Red(), qc.Green(), qc.Blue());
        }
    }
    
    

    QColor c = QColorDialog::getColor(initial, this);
    if (!c.isValid()) {
        return;
    }


    if (uid.isEmpty()) {
        return;
    }
    if (!sm.HasShapeEntry(uid.toStdString()) && currentDoc) {
        currentDoc->drawComponentByUID(uid);
    }
    Quantity_Color qcolor(c.redF(), c.greenF(), c.blueF(), Quantity_TOC_RGB);
    auto ctx = currentContext->getContext();
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
        if (!ctx.IsNull()) {
            ctx->SetColor(obj, qcolor, Standard_True);
        }
    }
    if (!currentContext->getContext().IsNull()) {
        currentContext->updateViewer();
    }
}

void ModificatorDisplayOptionsWidget::onMaterialChanged(const QString &mat)
{
    if (!currentContext) {
        return;
    }
    if (!currentItem) {
        return;
    }
    const QString uid = QString::fromStdString(currentItem->getUid());
    if (uid.isEmpty()) {
        return;
    }
    auto &sm = currentContext->GetShapeManager();
    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    auto it = tiglMaterials::materialMap.find(mat);
    if (it == tiglMaterials::materialMap.end()) {
        return;
    }
    auto ctx = currentContext->getContext();
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
        if (!ctx.IsNull()) {
            ctx->SetMaterial(obj, it->second, Standard_True);
        }
    }
    if (!currentContext->getContext().IsNull()) {
        currentContext->updateViewer();
    }
}
