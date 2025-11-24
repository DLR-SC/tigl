/*
* Copyright (C) 2018 German Aerospace Center (DLR/SC)
*
* Created: 2018-07-24 Martin Siggel <martin.siggel@dlr.de>
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

#include "TIGLGeometryChoserDialog.h"
#include "ui_TIGLGeometryChoserDialog.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

TIGLGeometryChoserDialog::TIGLGeometryChoserDialog(const tigl::CTiglUIDManager& uidManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TIGLGeometryChoserDialog)
    , m_uidMgr(uidManager)
{
    ui->setupUi(this);

#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
    ui->searchBox->setPlaceholderText("Filter...");
#endif

    m_model = new QSortFilterProxyModel(this);
    m_model->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->objectsList->setModel(m_model);
    SetComponentUIDs();

    connect(ui->searchBox, SIGNAL(textChanged(QString)), this, SLOT(onFilterChanged(QString)));
}

void TIGLGeometryChoserDialog::SetComponentUIDs()
{
    QStandardItemModel *model = new QStandardItemModel(0, 1, this);

    tigl::ShapeContainerType shapes = m_uidMgr.GetShapeContainer();
    for (tigl::ShapeContainerType::const_iterator it = shapes.begin(); it != shapes.end(); ++it) {
        int count = model->rowCount();
        model->insertRow(count);
        model->setData(model->index(count, 0), it->first.c_str());
    }

    m_model->setSourceModel(model);
}

TIGLGeometryChoserDialog::~TIGLGeometryChoserDialog()
{
    delete ui;
}

QStringList TIGLGeometryChoserDialog::GetSelectedUids() const
{
    QStringList result;

    // Multiple rows can be selected
    foreach (const QModelIndex &index, ui->objectsList->selectionModel()->selectedIndexes()) {
        QString uid = index.data().toString();
        result << uid;
    }
    return result;
}

void TIGLGeometryChoserDialog::onFilterChanged(QString filter)
{
    m_model->setFilterRegExp(filter);
}
