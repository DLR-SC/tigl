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

#include "CPACSTreeView.h"

#include <QMouseEvent>
#include <QPainter>

CPACSTreeView::CPACSTreeView(QWidget *parent)
 : QTreeView(parent)
{
    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
}

void CPACSTreeView::mouseMoveEvent(QMouseEvent *event)
{
    hoverPos = event->pos();
    QWidget::mouseMoveEvent(event);
    update();
}

void CPACSTreeView::leaveEvent(QEvent*)
{
    hoverPos = QPoint();
    update();
}

void CPACSTreeView::paintEvent(QPaintEvent *event)
{
    QTreeView::paintEvent(event);

    if (hoverPos.isNull()) {
        return;
    }

    QModelIndex index = indexAt(hoverPos);
    if (!index.isValid()){
        return;
    }

    QRect rect = visualRect(index);

    QModelIndex parent = index.parent();
    if (!parent.isValid() || !parent.data(Qt::UserRole).toBool()) {
        return;
    }

    QPainter p(viewport());
    p.setPen(QPen(Qt::blue, 2));

    if (hoverPos.y() < rect.top() + 4) {
        p.drawLine(rect.left(), rect.top(), rect.right(), rect.top());
    } else if (hoverPos.y() > rect.bottom() - 4) {
        p.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    }
}
