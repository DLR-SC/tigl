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

#ifndef MODIFICATORSECTIONSWIDGET_H
#define MODIFICATORSECTIONSWIDGET_H

#include <QWidget>
#include <variant>
#include "CCPACSFuselage.h"
#include "CCPACSWing.h"

namespace Ui
{
using ElementModificatorInterface = std::variant<tigl::CCPACSFuselage, tigl::CCPACSWing>; // could be extended by Duct, Pylon, Tank in the future
class ModificatorSectionsWidget;
}

class ModificatorSectionsWidget : public QWidget
{
    Q_OBJECT

signals:
    void undoCommandRequired();

public slots:
    void execNewConnectedElementDialog();
    void execDeleteConnectedElementDialog();

public:
    explicit ModificatorSectionsWidget(QWidget* parent = nullptr);
    ~ModificatorSectionsWidget();

    void setCreateConnectedElement(Ui::ElementModificatorInterface& element);

private:
    Ui::ModificatorSectionsWidget* ui;
    // Defined as std::variant
    // Construction is used to avoid an abstract basis class from which all possible variant types had to be inherited
    Ui::ElementModificatorInterface* createConnectedElement;
};

#endif // MODIFICATORSECTIONSWIDGET_H
