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

#include "ModificatorSectionsWidget.h"
#include "ui_ModificatorSectionsWidget.h"

#include "NewConnectedElementDialog.h"
#include "CTiglLogging.h"
#include "CTiglError.h"
#include "TIGLCreatorErrorDialog.h"
#include "DeleteDialog.h"
#include <optional>

ModificatorSectionsWidget::ModificatorSectionsWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ModificatorSectionsWidget)
{
    ui->setupUi(this);
    connect(ui->addConnectedElementBtn, SIGNAL(pressed()), this, SLOT(execNewConnectedElementDialog()));
    connect(ui->deleteConnectedElementBtn, SIGNAL(pressed()), this, SLOT(execDeleteConnectedElementDialog()));
}

ModificatorSectionsWidget::~ModificatorSectionsWidget()
{
    delete ui;
}

void ModificatorSectionsWidget::setCreateConnectedElement(Ui::ElementModificatorInterface const& element)
{
    createConnectedElement = element;

    // at least two sections required
    ui->deleteConnectedElementBtn->setDisabled(element.GetOrderedConnectedElement().size() <= 2);
}

void ModificatorSectionsWidget::execNewConnectedElementDialog()
{
    if (createConnectedElement == std::nullopt) {
        LOG(ERROR) << "ModificatorSectionsWidget:: is not correctly set!";
        return;
    }

    std::vector<std::string> elementUIDs = createConnectedElement->GetOrderedConnectedElement();

    QStringList elementUIDsQList;
    for (int i = 0; i < elementUIDs.size(); i++) {
        elementUIDsQList.push_back(elementUIDs.at(i).c_str());
    }

    NewConnectedElementDialog newElementDialog(elementUIDsQList, this);
    if (newElementDialog.exec() == QDialog::Accepted) {
        std::string startUID                    = newElementDialog.getStartUID().toStdString();
        std::string sectionName                 = newElementDialog.getSectionName().toStdString();
        NewConnectedElementDialog::Where where  = newElementDialog.getWhere();
        std::optional<double> eta               = newElementDialog.getEta();
        try {
            if (where == NewConnectedElementDialog::Before) {
                auto elementUIDBefore = createConnectedElement->GetElementUIDBeforeNewElement(startUID);
                if (elementUIDBefore) {
                    if (eta) { // Security check. Should be set if elementUIDBefore is true
                        createConnectedElement->CreateNewConnectedElementBetween(*elementUIDBefore, startUID, *eta, sectionName);
                    }
                    else {
                        throw tigl::CTiglError("No eta value set!");
                    }
                }
                else {
                    createConnectedElement->CreateNewConnectedElementBefore(startUID, sectionName);
                }
            }
            else if (where == NewConnectedElementDialog::After) {
                auto elementUIDAfter = createConnectedElement->GetElementUIDAfterNewElement(startUID);
                if (elementUIDAfter) {
                    if (eta) { // Security check. Should be set if elementUIDAfter is true
                        createConnectedElement->CreateNewConnectedElementBetween(startUID, *elementUIDAfter, *eta, sectionName);
                    }
                    else {
                        throw tigl::CTiglError("No eta value set!");
                    }
                }
                else {
                    createConnectedElement->CreateNewConnectedElementAfter(startUID, sectionName);
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
        emit undoCommandRequired();
        ui->deleteConnectedElementBtn->setDisabled(createConnectedElement->GetOrderedConnectedElement().size() <= 2);
    }
}


void ModificatorSectionsWidget::execDeleteConnectedElementDialog()
{

    if (createConnectedElement == std::nullopt) {
        LOG(ERROR) << "ModificatorWingsWidget::execDeleteWingDialog: wings is not set!";
        return;
    }

    std::vector<std::string> elementUIDs = createConnectedElement->GetOrderedConnectedElement();

    QStringList elementUIDsQList;
    for (int i = 0; i < elementUIDs.size(); i++) {
        elementUIDsQList.push_back(elementUIDs.at(i).c_str());
    }

    DeleteDialog deleteDialog(elementUIDsQList);
    if (deleteDialog.exec() == QDialog::Accepted) {
        QString uid = deleteDialog.getUIDToDelete();
        try {
            createConnectedElement->DeleteConnectedElement(uid.toStdString());
        }
        catch (const tigl::CTiglError& err) {
            TIGLCreatorErrorDialog errDialog(this);
            errDialog.setMessage(QString("<b>%1</b><br /><br />%2").arg("Fail to delete the section (aka connected element)").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            return;
        }
        emit undoCommandRequired();
    }

}
