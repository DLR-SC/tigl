/*
* Copyright (C) 2019 CFS Engineering
*
* Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef MODIFICATORSECTIONWIDGET_H
#define MODIFICATORSECTIONWIDGET_H

#include <QWidget>
#include "modificators/ModificatorWidget.h"
#include "ModificatorElementWidget.h"
#include "CTiglSectionElement.h"
#include "ProfilesDBManager.h"
#include <QList>

namespace Ui
{
class ModificatorSectionWidget;
}

class ModificatorSectionWidget : public ModificatorWidget
{
    Q_OBJECT

public:
    explicit ModificatorSectionWidget(QWidget* parent = nullptr);
    ~ModificatorSectionWidget();

    void setAssociatedElements(QList<tigl::CTiglSectionElement*> elements,  ProfilesDBManager* profilesDB);

    bool apply() override;

    void reset() override;

protected:
    void deleteElementWidgets();

private:
    Ui::ModificatorSectionWidget* ui;
    QList<ModificatorElementWidget*> elementWidgets;
};

#endif // MODIFICATORSECTIONWIDGET_H
