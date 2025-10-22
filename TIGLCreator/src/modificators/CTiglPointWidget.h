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

#ifndef CTIGLPOINTWIDGET_H
#define CTIGLPOINTWIDGET_H

#include <QWidget>
#include "CTiglPoint.h"

namespace Ui
{
class CTiglPointWidget;
}

/**
 * This widget displays and holds a CTiglPoint.
 * It is meant to be included in some larger widget.
 */
class CTiglPointWidget : public QWidget
{
    Q_OBJECT

public slots:

    void setGUIFromInternal();
    void setInternalFromGUI();

public:
    explicit CTiglPointWidget(QWidget* parent = nullptr);
    ~CTiglPointWidget();

    void setInternal(tigl::CTiglPoint newInternalPoint);
    tigl::CTiglPoint getInternalPoint()
    {
        return internalPoint;
    };
    void setLabel(QString labelText);

    /**
     * @return false if internal point is equal to the spin boxes, and true otherwise.
     */
    bool hasChanged();

private:
    Ui::CTiglPointWidget* ui;
    tigl::CTiglPoint internalPoint;
};

#endif // CTIGLPOINTWIDGET_H
