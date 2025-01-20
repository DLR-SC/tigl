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

namespace Ui
{

// Interface-like structure to account for different possible object types whose member variables may be adapted by the CPACSCreator.
// It is currently used for CCPACSFuselage and CCPACSWing.
// Could be extended by Duct, Pylon, Tank, etc. in the future (observe: The respective classes need to define the listed functions).
struct ElementModificatorInterface
{
    // Here, functions are defined as member variables calling the 'right' (depending on present data type) function from CCPACSFuselage, CCPACSWing, etc. via lambdas
    template <typename T>
    ElementModificatorInterface(T&& t)
        : CreateNewConnectedElementAfter(
            [&t](std::string str){ return t.CreateNewConnectedElementAfter(str); }
            )
        , CreateNewConnectedElementBefore(
            [&t](std::string str){ return t.CreateNewConnectedElementBefore(str); }
            )
        , CreateNewConnectedElementBetween(
            [&t](std::string str1, std::string str2){ return t.CreateNewConnectedElementBetween(str1, str2); }
            )
        , DeleteConnectedElement(
            [&t](std::string str){ return t.DeleteConnectedElement(str); }
            )
        , GetOrderedConnectedElement(
            [&t](){ return t.GetOrderedConnectedElement(); }
            )
    {}

    std::function<void(std::string)> CreateNewConnectedElementAfter;
    std::function<void(std::string)> CreateNewConnectedElementBefore;
    std::function<void(std::string, std::string)> CreateNewConnectedElementBetween;
    std::function<void(std::string)> DeleteConnectedElement;
    std::function<std::vector<std::string>()> GetOrderedConnectedElement;
};

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
    Ui::ModificatorSectionsWidget* ui;
    // std::optional used here to account for empty initializiation of member variable in construtor (ptr and nullptr used before)
    std::optional<Ui::ElementModificatorInterface> createConnectedElement;
};

#endif // MODIFICATORSECTIONSWIDGET_H
