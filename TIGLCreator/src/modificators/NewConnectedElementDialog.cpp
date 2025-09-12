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

#include "NewConnectedElementDialog.h"
#include "TIGLCreatorErrorDialog.h"
#include "ui_NewConnectedElementDialog.h"
#include <QString>

NewConnectedElementDialog::NewConnectedElementDialog(QStringList connectedElements, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::NewConnectedElementDialog)
{
    ui->setupUi(this);
    ui->comboBoxStartUID->addItems(connectedElements);
    ui->comboBoxWhere->addItem("After");
    ui->comboBoxWhere->addItem("Before");

    connect(
        ui->comboBoxStartUID,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(activate_eta())
    );

    connect(
        ui->comboBoxWhere,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(activate_eta())
    );

    connect(
        ui->comboBoxStartUID,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(updateDefaultName())
    );

    connect(
        ui->comboBoxWhere,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(updateDefaultName())
    );

    activate_eta();
    updateDefaultName();
}

void NewConnectedElementDialog::activate_eta()
{
    // Be careful: Here, we assume that the UID list is sorted

    bool at_start = ui->comboBoxStartUID->currentIndex() == 0 && ui->comboBoxWhere->currentText() == "Before";
    bool at_end = ui->comboBoxStartUID->currentIndex() == ui->comboBoxStartUID->count()-1 && ui->comboBoxWhere->currentText() == "After";
    between_sections = !(at_start || at_end);
    if (between_sections) {
        ui->eta_label->setText(QString("Set the eta parameter at which the new element should be placed within the segment.\nChoose a value between 0.0001 and 0.9999."));
        ui->eta_spinbox->setEnabled(true);
    } else {
        ui->eta_label->setText(QString("The new section will be added at the boundary"));
        ui->eta_spinbox->setEnabled(false);
    }
}

void NewConnectedElementDialog::updateDefaultName()
{
    QString uid = ui->comboBoxStartUID->currentText();
    QString where = ui->comboBoxWhere->currentText();

    QString defaultName = QString("%1_%2").arg(uid, where);

    // Only update if the user hasn’t typed a custom name
    if (ui->lineEditName->text().isEmpty() ||
        ui->lineEditName->text().endsWith("Before") ||
        ui->lineEditName->text().endsWith("After")) 
    {
        ui->lineEditName->setText(defaultName);
    }
}

NewConnectedElementDialog::~NewConnectedElementDialog()
{
    delete ui;
}

QString NewConnectedElementDialog::getStartUID() const
{
    return ui->comboBoxStartUID->currentText();
}

void NewConnectedElementDialog::setStartUID(QString const& text)
{
    ui->comboBoxStartUID->setCurrentText(text);
}

QString NewConnectedElementDialog::getSectionName() const
{
    return ui->lineEditName->text();
}

void NewConnectedElementDialog::applySelection(Ui::ElementModificatorInterface &element)
{
    std::string startUID                    = getStartUID().toStdString();
    std::string sectionName                 = getSectionName().toStdString();
    NewConnectedElementDialog::Where where  = getWhere();
    std::optional<double> eta               = getEta();
    try {
        if (where == NewConnectedElementDialog::Before) {
            auto elementUIDBefore = element.GetElementUIDBeforeNewElement(startUID);
            if (elementUIDBefore) {
                if (eta) { // Security check. Should be set if elementUIDBefore is true
                    element.CreateNewConnectedElementBetween(*elementUIDBefore, startUID, *eta, sectionName);
                }
                else {
                    throw tigl::CTiglError("No eta value set!");
                }
            }
            else {
                element.CreateNewConnectedElementBefore(startUID, sectionName);
            }
        }
        else if (where == NewConnectedElementDialog::After) {
            auto elementUIDAfter = element.GetElementUIDAfterNewElement(startUID);
            if (elementUIDAfter) {
                if (eta) { // Security check. Should be set if elementUIDAfter is true
                    element.CreateNewConnectedElementBetween(startUID, *elementUIDAfter, *eta, sectionName);
                }
                else {
                    throw tigl::CTiglError("No eta value set!");
                }
            }
            else {
                element.CreateNewConnectedElementAfter(startUID, sectionName);
            }
        }
    }
    catch (const tigl::CTiglError& err) {
        TIGLCreatorErrorDialog errDialog(this);
        errDialog.setMessage(
            QString("<b>%1</b><br /><br />%2").arg("Fail to create the new connected element ").arg(err.what()));
        errDialog.setWindowTitle("Error");
        errDialog.setDetailsText(err.what());
        errDialog.exec();
        return;
    }
}

NewConnectedElementDialog::Where NewConnectedElementDialog::getWhere() const
{
    if (ui->comboBoxWhere->currentText() == "Before") {
        return Before;
    }
    return After;
}

void NewConnectedElementDialog::setWhere(Where where)
{
    if (where == Where::Before) {
        ui->comboBoxWhere->setCurrentText("Before");
    } else {
        ui->comboBoxWhere->setCurrentText("After");
    }
}

std::optional<double> NewConnectedElementDialog::getEta() const
{
    if (between_sections) {
        return ui->eta_spinbox->value();
    }
    else {
        return std::nullopt;
    }
}
