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

#ifndef TIGLGEOMETRYCHOSERDIALOG_H
#define TIGLGEOMETRYCHOSERDIALOG_H

#include "CTiglUIDManager.h"
#include <QDialog>

namespace Ui {
class TIGLGeometryChoserDialog;
}

class TIGLGeometryChoserDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TIGLGeometryChoserDialog(const tigl::CTiglUIDManager& uidManager, QWidget *parent = 0);
    ~TIGLGeometryChoserDialog();
    
    QStringList GetSelectedUids() const;
    
public slots:
    void onFilterChanged(QString filter);
    
private:
    Ui::TIGLGeometryChoserDialog *ui;
    const tigl::CTiglUIDManager& m_uidMgr;
    class QSortFilterProxyModel* m_model;

    void SetComponentUIDs();
};

#endif // TIGLGEOMETRYCHOSERDIALOG_H
