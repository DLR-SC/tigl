/*
 * Copyright (C) 2025 DLR
 *
 * Created: 2025 Jan Kleinert <jan.kleinert@dlr.de>
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

#include <QTreeWidget>
#include <optional>

namespace Ui
{
class CPACSTreeView;
}

/**
 * @brief This class inherits QTreeView.
 *
 * The purpose of the customization is special painting for
 * hovering a mouse between two tree elements. This is useful
 * for context menus.
 *
 * @author Jan Kleinert
 */
class CPACSTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit CPACSTreeView(QWidget* parent = nullptr);

public slots:
    void onContextMenuDone();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool contextMenuRequested;
    QLine line; // remember the line between two elements
    QPoint hoverPos;
};
