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
#include "TIGLCreatorWindow.h"

#define BTN_STYLE "#%2 {background-color: %1; color: black; border: 1px solid black; border-radius: 5px;} #%2:hover {border: 1px solid white;}"

ModificatorDisplayOptionsWidget::ModificatorDisplayOptionsWidget(QWidget* parent)
    : CpacsEditorBase(parent)
{
    ui = new Ui::ModificatorDisplayOptionsWidget();
    ui->setupUi(this);

    infoLabel = ui->infoLabel;
    transparencySlider = ui->transparencySlider;
    renderingModeCombo = ui->renderingModeCombo;
    buttonColorChoser = ui->buttonColorChoser;
    materialCombo = ui->materialCombo;

    renderingModeCombo->addItem("Wireframe", 0);
    renderingModeCombo->addItem("Shaded", 1);
    // populate materials list after ui setup
    materialCombo->clear();
    for (const auto &kv : tiglMaterials::materialMap) {
        materialCombo->addItem(kv.first);
    }

    // General Display Options
    connect(transparencySlider, SIGNAL(valueChanged(int)), this, SLOT(onTransparencyChanged(int)));
    connect(renderingModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onRenderingModeChanged(int)));
    connect(materialCombo, SIGNAL(currentTextChanged(const QString &)), this, SLOT(onMaterialChanged(const QString &)));
    connect(buttonColorChoser, SIGNAL(clicked()), this, SLOT(onColorChosen()));

    // Wing draw actions are invoked via the drawOptionsCombo callbacks (set in setFromItem)
}

ModificatorDisplayOptionsWidget::~ModificatorDisplayOptionsWidget() = default;

void ModificatorDisplayOptionsWidget::setFromItem(cpcr::CPACSTreeItem* item, TIGLCreatorDocument* doc, TIGLCreatorContext* context)
{

    currentItem = item;
    currentDoc = doc;
    currentContext = context;

    if (!item) {
        // infoLabel->setText(tr("Please select an editable element from the CPACS Tree."));

        if (ui) {
            ui->infoLabel->setVisible(true);
            ui->labelTransparency->setVisible(false);
            ui->transparencySlider->setVisible(false);
            ui->labelRenderingMode->setVisible(false);
            ui->renderingModeCombo->setVisible(false);
            ui->labelColor->setVisible(false);
            ui->buttonColorChoser->setVisible(false);
            ui->labelMaterial->setVisible(false);
            ui->materialCombo->setVisible(false);
            ui->drawOptionsCombo->setVisible(false);
        }
        currentItem = nullptr;
        return;
    }

    if (ui) {
        ui->infoLabel->setVisible(false);
        ui->labelTransparency->setVisible(true);
        ui->transparencySlider->setVisible(true);
        ui->labelRenderingMode->setVisible(true);
        ui->renderingModeCombo->setVisible(true);
        ui->labelColor->setVisible(true);
        ui->buttonColorChoser->setVisible(true);
        ui->labelMaterial->setVisible(true);
        ui->materialCombo->setVisible(true);
        ui->drawOptionsCombo->setVisible(true);
    }
    
    // const QString type = QString::fromStdString(item->getType());
    const QString uid = QString::fromStdString(item->getUid());


    // get current values
    auto &sm = context->GetShapeManager();
    if (sm.HasShapeEntry(uid.toStdString())) {
        auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
        
        auto obj = objs[0];
        Standard_Real transparency;
        int displayMode;
        QColor color;

        if (!obj.IsNull()) {
            Handle(Prs3d_Drawer) drawer = obj->Attributes();
            transparency = drawer->Transparency(); 
            displayMode = drawer->DisplayMode();
            Quantity_Color qc = drawer->Color();
            color = QColor::fromRgbF(qc.Red(), qc.Green(), qc.Blue());

        }
        transparencySlider->setValue(transparency*100);
        renderingModeCombo->setCurrentIndex(displayMode);
        updateColorButton(color);

    }

    materialCombo->setCurrentIndex(0);
    // Populate draw options combo when the selected item is a wing (type or uid == "wing").
    ui->drawOptionsCombo->clear();
    drawCallbacks.clear();
    auto type = doc->GetConfiguration().GetUIDManager().GetGeometricComponent(uid.toStdString()).GetComponentType();

    if (type == TIGL_COMPONENT_WING) {
        // Add entries and store callbacks that call the document drawing functions
        ui->drawOptionsCombo->addItem(tr("Profiles"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingProfiles(); });

        ui->drawOptionsCombo->addItem(tr("Overlay profile points"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingOverlayProfilePoints(); });

        ui->drawOptionsCombo->addItem(tr("Guide curves"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingGuideCurves(); });

        ui->drawOptionsCombo->addItem(tr("Wing"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWing(); });

        ui->drawOptionsCombo->addItem(tr("Triangulation"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingTriangulation(); });

        ui->drawOptionsCombo->addItem(tr("Sample points"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingSamplePoints(); });

        ui->drawOptionsCombo->addItem(tr("Fused wing"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawFusedWing(); });

        ui->drawOptionsCombo->addItem(tr("Component segment"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingComponentSegment(); });

        ui->drawOptionsCombo->addItem(tr("Component segment points"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingComponentSegmentPoints(); });

        ui->drawOptionsCombo->addItem(tr("Shells"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingShells(); });

        ui->drawOptionsCombo->addItem(tr("Structure"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingStructure(); });

        ui->drawOptionsCombo->addItem(tr("Flaps"));
        drawCallbacks.push_back([doc]() { if (doc) doc->drawWingFlaps(); });

        // Trigger the corresponding callback when an item is activated. Use UniqueConnection to avoid duplicate connections.
        connect(ui->drawOptionsCombo, QOverload<int>::of(&QComboBox::activated), this,
            [this](int idx) {
                if (idx >= 0 && idx < static_cast<int>(drawCallbacks.size()) && drawCallbacks[idx]) {
                    drawCallbacks[idx]();
                }
            }, Qt::UniqueConnection);
    }
    

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

                QColor qc = buttonColorChoser ? buttonColorChoser->palette().color(QPalette::Button) : QColor();
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
    
    QColor color = QColorDialog::getColor(initial, this);
    if (!color.isValid()) {
        return;
    }

    if (uid.isEmpty()) {
        return;
    }

    Quantity_Color qcolor(color.redF(), color.greenF(), color.blueF(), Quantity_TOC_RGB);
    auto context = currentContext->getContext();
    for (auto &obj : objs) {
        if (!context.IsNull()) {
            context->SetColor(obj, qcolor, Standard_True);
        }
    }
    if (!currentContext->getContext().IsNull()) {
        currentContext->updateViewer();
    }
    updateColorButton(color);
}

void ModificatorDisplayOptionsWidget::updateColorButton(QColor color)
{
    QString qss = QString(BTN_STYLE).arg(color.name(), "buttonColorChoser");
    buttonColorChoser->setStyleSheet(qss);
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
    auto context = currentContext->getContext();
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
        if (!context.IsNull()) {
            context->SetMaterial(obj, it->second, Standard_True);
        }
    }
    if (!currentContext->getContext().IsNull()) {
        currentContext->updateViewer();
    }
}
