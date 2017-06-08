/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-10-28 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "TIGLViewerErrorDialog.h"
#include "ui_TIGLViewerErrorDialog.h"
#include <QStyle>

TIGLViewerErrorDialog::TIGLViewerErrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerErrorDialog)
{
    ui->setupUi(this);

    detailsButton = new QPushButton("Show details", this);
    ui->buttonBox->addButton(detailsButton, QDialogButtonBox::ActionRole);

    ui->buttonBox->button(QDialogButtonBox::Close)->setFocus();
    
    setDetailsVisible(false);
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    
    QObject::connect(detailsButton, SIGNAL(clicked()), this, SLOT(toggleDetails()));
}

void TIGLViewerErrorDialog::setMessage(const QString &msg)
{
    ui->errorMessageLabel->setText(msg);
}

void TIGLViewerErrorDialog::setDetailsText(const QString &text)
{
    ui->logBrowser->setText(text);
}

void TIGLViewerErrorDialog::toggleDetails()
{
    detailsVisible = !detailsVisible;
    setDetailsVisible(detailsVisible);
}

void TIGLViewerErrorDialog::setDetailsVisible(bool visible)
{
    detailsVisible = visible;
    ui->groupBox->setVisible(visible);
    if (visible) {
        detailsButton->setText("Hide details");
    }
    else {
        detailsButton->setText("Show details");
    }
    this->readjustSize();
}

void TIGLViewerErrorDialog::readjustSize(){}

TIGLViewerErrorDialog::~TIGLViewerErrorDialog()
{
    delete ui;
}
