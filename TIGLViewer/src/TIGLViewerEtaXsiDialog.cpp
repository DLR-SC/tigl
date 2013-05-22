/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-05-21 Martin Siggel <martin.siggel@dlr.de>
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

#include "TIGLViewerEtaXsiDialog.h"
#include "ui_TIGLViewerEtaXsiDialog.h"

EtaXsiDialog::EtaXsiDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EtaXsiDialog)
{
    ui->setupUi(this);
}

EtaXsiDialog::~EtaXsiDialog()
{
    delete ui;
}

int EtaXsiDialog::getEtaXsi(QWidget *parent, double &eta, double &xsi){
    EtaXsiDialog dialog(parent);
    int ok = dialog.exec();
    
    if(ok == QDialog::Accepted){
        eta = dialog.getEta();
        xsi = dialog.getXsi();
    }
    return ok;
}

double EtaXsiDialog::getEta() const {
    return ui->etaSpinBox->value();
}
double EtaXsiDialog::getXsi() const {
    return ui->xsiSpinBox->value();
}
