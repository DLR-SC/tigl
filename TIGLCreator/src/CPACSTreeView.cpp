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
 , contextMenuRequested(false)
{
    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, [=](){ contextMenuRequested = true; });
}

void CPACSTreeView::onContextMenuDone()
{
    contextMenuRequested = false;
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

    QPainter p(viewport());
    p.setPen(QPen(Qt::lightGray, 2));

    auto paint = [&](){
        p.drawLine(line);                  // draw line between elements
        // To Do: Loose focus of hovered item
    };

    // keep the visual feedback of selected gap while context menu is open
    if (contextMenuRequested) {
        paint();
        return;
    }

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

    if (hoverPos.y() < rect.top() + 4) {
        left = QPoint(rect.left(), rect.top());
        right = QPoint(rect.right(), rect.top());
        line = QLine(left, right);
        paint();
    } else if (hoverPos.y() > rect.bottom() - 4) {
        left = QPoint(rect.left(), rect.bottom());
        right = QPoint(rect.right(), rect.bottom());
        line = QLine(left, right);
        paint();
    }
}
