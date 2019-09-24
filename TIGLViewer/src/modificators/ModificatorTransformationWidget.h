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

#ifndef MODIFICATORTRANSFORMATIONWIDGET_H
#define MODIFICATORTRANSFORMATIONWIDGET_H

#include "ModificatorWidget.h"
#include "CCPACSTransformation.h"
#include "CCPACSConfiguration.h"

namespace Ui
{
class ModificatorTransformationWidget;
}

class ModificatorTransformationWidget : public ModificatorWidget
{
    Q_OBJECT

public:
    explicit ModificatorTransformationWidget(QWidget* parent = nullptr);
    ~ModificatorTransformationWidget();

    void setTransformation(tigl::CCPACSTransformation& transformationItem, tigl::CCPACSConfiguration& config);
    void setSpinBoxesFromInternal();

    bool apply() override;
    void reset() override;

private:
    Ui::ModificatorTransformationWidget* ui;

public:
private:
    tigl::CCPACSTransformation* tiglTransformation;
    tigl::CCPACSConfiguration* config;
};

#endif // MODIFICATORTRANSFORMATIONWIDGET_H
