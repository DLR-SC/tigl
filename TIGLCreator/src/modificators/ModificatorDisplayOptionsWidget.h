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

#pragma once

#include "CpacsEditorBase.h"
#include <QWidget>

namespace Ui { class ModificatorDisplayOptionsWidget; }

namespace cpcr { class CPACSTreeItem; }
class TIGLCreatorDocument;
class TIGLCreatorContext;

class QCheckBox;
class QLabel;
class QSlider;
class QRadioButton;
class QButtonGroup;
class QPushButton;
class QComboBox;
class QColor;

class ModificatorDisplayOptionsWidget : public CpacsEditorBase
{
    Q_OBJECT

public:
    explicit ModificatorDisplayOptionsWidget(QWidget* parent = nullptr);
    ~ModificatorDisplayOptionsWidget();

    void setFromItem(cpcr::CPACSTreeItem* item, TIGLCreatorDocument* doc, TIGLCreatorContext* context=nullptr);

    bool apply() override;
    void reset() override;

private slots:
    void onTransparencyChanged(int value);
    void onRenderingModeChanged(int id);
    void onColorChosen();
    void onMaterialChanged(const QString& mat);

signals:
    void setTransparencyRequested(int value);
    void setRenderingModeRequested(int mode); 
    void setColorRequested(const QColor& color);
    void setMaterialRequested(const QString& material);

private:

    void updateColorButton(QColor color);

    Ui::ModificatorDisplayOptionsWidget* ui{nullptr};
    QLabel* infoLabel{nullptr};
    QSlider* transparencySlider{nullptr};
    QComboBox* renderingModeCombo{nullptr};
    QPushButton* buttonColorChoser{nullptr};
    QComboBox* materialCombo{nullptr};

    cpcr::CPACSTreeItem* currentItem{nullptr};
    TIGLCreatorDocument* currentDoc{nullptr};
    TIGLCreatorContext* currentContext{nullptr};
};


