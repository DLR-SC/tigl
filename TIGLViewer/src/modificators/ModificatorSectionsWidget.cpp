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
#include "TIGLViewerErrorDialog.h"

ModificatorSectionsWidget::ModificatorSectionsWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ModificatorSectionsWidget)
{
    ui->setupUi(this);
    createConnectedElementI = nullptr;
    connect(ui->addConnectedElementBtn, SIGNAL(pressed()), this, SLOT(execNewConnectedElementDialog()));
}

ModificatorSectionsWidget::~ModificatorSectionsWidget()
{
    delete ui;
}

void ModificatorSectionsWidget::setCreateConnectedElementI(tigl::CreateConnectedElementI& elementI)
{
    createConnectedElementI = &elementI;
}

void ModificatorSectionsWidget::execNewConnectedElementDialog()
{
    if (createConnectedElementI == nullptr) {
        LOG(ERROR) << "ModificatorSectionsWidget:: is not correctly set!";
        return;
    }

    std::vector<std::string> elementUIDs = createConnectedElementI->GetOrderedConnectedElement();
    QStringList elementUIDsQList;
    for (int i = 0; i < elementUIDs.size(); i++) {
        elementUIDsQList.push_back(elementUIDs.at(i).c_str());
    }

    NewConnectedElementDialog newElementDialog(elementUIDsQList, this);
    if (newElementDialog.exec() == QDialog::Accepted) {
        std::string startUID                   = newElementDialog.getStartUID().toStdString();
        NewConnectedElementDialog::Where where = newElementDialog.getWhere();
        try {
            if (where == NewConnectedElementDialog::Before) {
                createConnectedElementI->CreateNewConnectedElementBefore(startUID);
            }
            else if (where == NewConnectedElementDialog::After) {
                createConnectedElementI->CreateNewConnectedElementAfter(startUID);
            }
        }
        catch (const tigl::CTiglError& err) {
            TIGLViewerErrorDialog errDialog(this);
            errDialog.setMessage(
                QString("<b>%1</b><br /><br />%2").arg("Fail to create the new connected element ").arg(err.what()));
            errDialog.setWindowTitle("Error");
            errDialog.setDetailsText(err.what());
            errDialog.exec();
            return;
        }
        emit undoCommandRequired();
    }
}
