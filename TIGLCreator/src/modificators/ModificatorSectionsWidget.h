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
#include <string>
#include <optional>
#include "CCPACSFuselage.h"
#include "CCPACSWing.h"
#include "ElementModificatorInterface.h"

namespace Ui
{
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

    void setCreateConnectedElement(Ui::ElementModificatorInterface const& element);

private:

    // disables the selection deletion, if we have two or less sections
    void update_delete_section_button_disabled_state();

    Ui::ModificatorSectionsWidget* ui;
    // std::optional used here to account for empty initializiation of member variable in construtor (ptr and nullptr used before)
    std::optional<Ui::ElementModificatorInterface> createConnectedElement;
};

#endif // MODIFICATORSECTIONSWIDGET_H
