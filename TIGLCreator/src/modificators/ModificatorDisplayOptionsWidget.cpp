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
#include <QFileDialog>
#include <QCheckBox>
#include <QSignalBlocker>
#include "TIGLCreatorWindow.h"
#include "../DrawOptionsActions.h"
#include "CTiglAbstractGeometricComponent.h"
#include "TiglSymmetryAxis.h"

#define BTN_STYLE "#%2 {background-color: %1; color: black; border: 1px solid black; border-radius: 5px;} #%2:hover {border: 1px solid white;}"

ModificatorDisplayOptionsWidget::ModificatorDisplayOptionsWidget(QWidget* parent)
    : CpacsEditorBase(parent)
{
    ui = new Ui::ModificatorDisplayOptionsWidget();
    ui->setupUi(this);
    connect(ui->drawOptionsCombo, QOverload<int>::of(&QComboBox::activated), this,
    [this](int idx) {
        if (idx >= 0 && idx < static_cast<int>(drawCallbacks.size()) && drawCallbacks[idx]) {
            drawCallbacks[idx]();
        }
    });

    infoLabel = ui->infoLabel;
    transparencySlider = ui->transparencySlider;
    renderingModeCombo = ui->renderingModeCombo;
    buttonColorChoser = ui->buttonColorChoser;
    buttonResetOptions = ui->buttonResetOptions;
    materialCombo = ui->materialCombo;
    labelSymmetry = ui->labelSymmetry;
    checkBoxShowSymmetry = ui->checkBoxShowSymmetry;

    renderingModeCombo->addItem("Wireframe", 0);
    renderingModeCombo->addItem("Shaded", 1);
    renderingModeCombo->addItem("Textured", 2);
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
    connect(buttonResetOptions, SIGNAL(clicked()), this, SLOT(onResetOptions()));
    connect(checkBoxShowSymmetry, SIGNAL(toggled(bool)), this, SLOT(onShowSymmetryToggled(bool)));

}

ModificatorDisplayOptionsWidget::~ModificatorDisplayOptionsWidget() = default;

void ModificatorDisplayOptionsWidget::setFromItem(cpcr::CPACSTreeItem* item, TIGLCreatorDocument* doc, TIGLCreatorContext* context)
{

    currentItem = item;
    currentDoc = doc;
    currentContext = context;

    if (!item) {

        if (ui) {
            ui->infoLabel->setVisible(true);
            ui->labelTransparency->setVisible(false);
            ui->transparencySlider->setVisible(false);
            ui->labelRenderingMode->setVisible(false);
            ui->renderingModeCombo->setVisible(false);
            ui->labelColor->setVisible(false);
            ui->buttonColorChoser->setVisible(false);
            ui->labelColor->setVisible(false);
            ui->labelMaterial->setVisible(false);
            ui->materialCombo->setVisible(false);
            ui->buttonResetOptions->setVisible(false);
            ui->labelDrawOptions->setVisible(false);
            ui->drawOptionsCombo->setVisible(false);
            ui->labelSymmetry->setVisible(false);
            ui->checkBoxShowSymmetry->setVisible(false);
        }
        currentItem = nullptr;
        return;
    }
    if (item) 
    {
        const QString uid = QString::fromStdString(item->getUid());
        if ((!uid.isEmpty() && doc->GetConfiguration().GetUIDManager().HasGeometricComponent(uid.toStdString()))) {
            const auto& shapes = doc->GetConfiguration().GetUIDManager().GetShapeContainer();
            auto it = shapes.find(uid.toStdString());
            if (it != shapes.end() && it->second != nullptr) {
                tigl::ITiglGeometricComponent* comp = it->second;
                if (comp->GetComponentType() != TIGL_COMPONENT_PLANE && comp->GetComponentType() != TIGL_COMPONENT_CROSS_BEAM_STRUT) {
                    // "Show Symmetry" checkbox: only relevant for components with a symmetry axis
                    auto* geometricComp = dynamic_cast<tigl::CTiglAbstractGeometricComponent*>(comp);
                    bool hasSymmetry = geometricComp && geometricComp->GetSymmetryAxis() != TIGL_NO_SYMMETRY;
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
                        ui->buttonResetOptions->setVisible(true);
                        ui->labelDrawOptions->setVisible(true);
                        ui->drawOptionsCombo->setVisible(true);
                        ui->labelSymmetry->setVisible(hasSymmetry);
                        ui->checkBoxShowSymmetry->setVisible(hasSymmetry);
                    }

                    // get current values
                    auto &sm = context->GetShapeManager(currentDoc->docId());
                    if (sm.HasShapeEntry(uid.toStdString())) {
                        auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
                        if (objs.empty()) {
                            LOG(WARNING) << "No objects found for shape with uid " << uid.toStdString();
                            return;
                        }
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

                    if (hasSymmetry) {
                        // Read the persisted preference rather than the live AIS display state:
                        // the latter gets reset whenever the shape is redrawn (e.g. by toggling
                        // this component's visibility in the CPACS tree), which would otherwise
                        // desync the checkbox from what is actually shown in the viewer.
                        bool shown = sm.GetSymmetryVisible(uid.toStdString());
                        QSignalBlocker blocker(checkBoxShowSymmetry);
                        checkBoxShowSymmetry->setChecked(shown);
                    }
                }
                else if (comp->GetComponentType() == TIGL_COMPONENT_PLANE) {
                    if (ui) {
                        ui->infoLabel->setVisible(true);
                        ui->labelTransparency->setVisible(false);
                        ui->transparencySlider->setVisible(false);
                        ui->labelRenderingMode->setVisible(false);
                        ui->renderingModeCombo->setVisible(false);
                        ui->buttonColorChoser->setVisible(false);
                        ui->labelColor->setVisible(false);
                        ui->labelMaterial->setVisible(false);
                        ui->materialCombo->setVisible(false);
                        ui->buttonResetOptions->setVisible(false);
                        ui->labelDrawOptions->setVisible(true);
                        ui->drawOptionsCombo->setVisible(true);
                        ui->labelSymmetry->setVisible(false);
                        ui->checkBoxShowSymmetry->setVisible(false);
                    }
                }
            }
        }
        else {
            if (ui) {
                ui->infoLabel->setVisible(true);
                ui->labelTransparency->setVisible(false);
                ui->transparencySlider->setVisible(false);
                ui->labelRenderingMode->setVisible(false);
                ui->renderingModeCombo->setVisible(false);
                ui->buttonColorChoser->setVisible(false);
                ui->labelColor->setVisible(false);
                ui->labelMaterial->setVisible(false);
                ui->materialCombo->setVisible(false);
                ui->buttonResetOptions->setVisible(false);
                ui->labelSymmetry->setVisible(false);
                ui->checkBoxShowSymmetry->setVisible(false);
                ui->labelDrawOptions->setVisible(false);
                ui->drawOptionsCombo->setVisible(false);
            }
        }
    

        // Populate draw options combo when the selected item is a wing (type or uid == "wing").
        ui->drawOptionsCombo->clear();
        drawCallbacks.clear();
        if (!uid.isEmpty() && doc->GetConfiguration().GetUIDManager().HasGeometricComponent(uid.toStdString())) {
            auto type = doc->GetConfiguration().GetUIDManager().GetGeometricComponent(uid.toStdString()).GetComponentType();


            if (type == TIGL_COMPONENT_WING) {
                drawCallbacks.clear();
                ui->drawOptionsCombo->clear();
                for (const auto& action : getWingDrawOptionsActions()) {
                     if (action.isAvailable && !action.isAvailable(doc, uid)) {
                        continue;
                    }
                    ui->drawOptionsCombo->addItem(tr(action.label.toUtf8()));
                    drawCallbacks.push_back([this, action, uid, doc]() {
                        action.handler(doc, uid);
                    });
                }
            }

            if (type == TIGL_COMPONENT_FUSELAGE) {
                drawCallbacks.clear();
                ui->drawOptionsCombo->clear();
                for (const auto& action : getFuselageDrawOptionsActions()) {
                    ui->drawOptionsCombo->addItem(tr(action.label.toUtf8()));
                    drawCallbacks.push_back([this, action, uid, doc]() {
                        action.handler(doc, uid);
                    });
                }
            }

            if (type == TIGL_COMPONENT_PLANE) {
                drawCallbacks.clear();
                ui->drawOptionsCombo->clear();
                for (const auto& action : getPlaneDisplayOptionsActions()) {
                    if (action.isAvailable && !action.isAvailable(doc, uid)) {
                        continue;
                    }

                    ui->drawOptionsCombo->addItem(tr(action.label.toUtf8()));
                    drawCallbacks.push_back([this, action, uid, doc]() {
                        action.handler(doc, uid);
                    });
                } 
            }

            if (type == TIGL_COMPONENT_ROTORBLADE) {
                drawCallbacks.clear();
                ui->drawOptionsCombo->clear();
                for (const auto& action : getRotorBladeDrawOptionsActions()) {
                    ui->drawOptionsCombo->addItem(tr(action.label.toUtf8()));
                    drawCallbacks.push_back([this, action, uid, doc]() {
                        action.handler(doc, uid);
                    });
                }  
            }
            
            if (type == TIGL_COMPONENT_ROTOR) {
                drawCallbacks.clear();
                ui->drawOptionsCombo->clear();
                for (const auto& action : getRotorDrawOptionsActions()) {
                    ui->drawOptionsCombo->addItem(tr(action.label.toUtf8()));
                    drawCallbacks.push_back([this, action, uid, doc]() {
                        action.handler(doc, uid);
                    });
                }
            }
            
            if (type == TIGL_COMPONENT_ENGINE_PYLON || type == TIGL_COMPONENT_ENGINE_NACELLE) {
                 drawCallbacks.clear();
                ui->drawOptionsCombo->clear();
                ui->drawOptionsCombo->setVisible(false);
                ui->labelDrawOptions->setVisible(false);
            }
        }
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
            auto &sm = currentContext->GetShapeManager(currentDoc->docId());
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
                    // Skip objects hidden via the "Show Symmetry" toggle so they don't get
                    // implicitly redisplayed by an unrelated property change.
                    if (!context.IsNull() && !context->IsDisplayed(obj)) {
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
    setFromItem(currentItem, currentDoc, currentContext);
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
    auto &sm = currentContext->GetShapeManager(currentDoc->docId());
    if (!sm.HasShapeEntry(uid.toStdString()) && currentDoc) {
        currentDoc->drawComponentByUID(uid);
    }
    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    Standard_Real tr = value * 0.01;
    auto context = currentContext->getContext();
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
        if (!context.IsNull() && !context->IsDisplayed(obj)) {
            continue;
        }
        if (!context.IsNull()) {
            context->SetTransparency(obj, tr, Standard_True);
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
    auto context = currentContext->getContext();

    if (displayMode == 2) { // Textured
        QString fileName = QFileDialog::getOpenFileName (this,
                                                     tr("Choose texture image"),
                                                     QString(),
                                                     tr("Images (*.png *.jpeg *.jpg *.bmp);") );

        if (!fileName.isEmpty()) {
            currentContext->setObjectsTexture(fileName);
        }
    }

    const QString uid = QString::fromStdString(currentItem->getUid());
    if (uid.isEmpty()) {
        return;
    }
    auto &sm = currentContext->GetShapeManager(currentDoc->docId());
    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
        if (!context.IsNull() && !context->IsDisplayed(obj)) {
            continue;
        }
        if (!context.IsNull()) {
            context->SetDisplayMode(obj, displayMode, Standard_True);
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
    auto &sm = currentContext->GetShapeManager(currentDoc->docId());
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
        if (obj.IsNull()) {
            continue;
        }
        if (!context.IsNull() && !context->IsDisplayed(obj)) {
            continue;
        }
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
    auto &sm = currentContext->GetShapeManager(currentDoc->docId());
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
        if (!context.IsNull() && !context->IsDisplayed(obj)) {
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

void ModificatorDisplayOptionsWidget::onResetOptions()
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
    auto &sm = currentContext->GetShapeManager(currentDoc->docId());
    // Reset Options should also restore the default "Show Symmetry" state.
    sm.SetSymmetryVisible(uid.toStdString(), true);
    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    auto context = currentContext->getContext();
    for (auto &obj : objs) {
        if (obj.IsNull()) {
            continue;
        }
         if (!context.IsNull() && currentDoc) {
            // redraw component to reset options (necessary to reset different colors on mirrored components)
            context->Remove(obj, Standard_False);
            sm.removeObject(obj);

        }
    }

    auto type = currentDoc->GetConfiguration().GetUIDManager().GetGeometricComponent(uid.toStdString()).GetComponentType();
    if (type == TIGL_COMPONENT_WING) {
        tigl::CCPACSWing& wing = currentDoc->GetConfiguration().GetWing(uid.toStdString());
        wing.SetBuildFlaps(false);
        currentDoc->drawWing(uid);

    }

    else {
        currentDoc->drawComponentByUID(uid);
    }       

    if (!currentContext->getContext().IsNull()) {
        currentContext->updateViewer();
    }

    setFromItem(currentItem, currentDoc, currentContext);
}

void ModificatorDisplayOptionsWidget::onShowSymmetryToggled(bool checked)
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
    auto &sm = currentContext->GetShapeManager(currentDoc->docId());
    // Persist the preference so it survives the mirrored shape being torn down and
    // re-created (e.g. by toggling this component's visibility in the CPACS tree).
    sm.SetSymmetryVisible(uid.toStdString(), checked);

    if (!sm.HasShapeEntry(uid.toStdString()) && currentDoc) {
        currentDoc->drawComponentByUID(uid);
    }

    // If the component itself is currently hidden (visibility toggled off in the CPACS
    // tree), don't touch the AIS display state now: showing just the mirrored shape while
    // the main one stays hidden would look inconsistent. The preference just recorded above
    // will be picked up once the component's visibility is toggled back on.
    if (!sm.GetVisibility(uid.toStdString())) {
        return;
    }

    auto objs = sm.GetIObjectsFromShapeName(uid.toStdString());
    if (objs.size() <= 1 || objs[1].IsNull()) {
        return;
    }
    auto context = currentContext->getContext();
    if (context.IsNull()) {
        return;
    }
    if (checked) {
        context->Display(objs[1], Standard_True);
    }
    else {
        context->Erase(objs[1], Standard_True);
    }
    currentContext->updateViewer();
}
