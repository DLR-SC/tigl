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

#include "StandardizeDialog.h"
#include "ui_StandardizeDialog.h"
#include "CCPACSWing.h"
#include "CCPACSFuselage.h"
#include "CTiglStandardizer.h"
#include "QRegExp"
#include "QRegularExpressionMatch"

StandardizeDialog::StandardizeDialog(tigl::CCPACSConfiguration& configuration, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::StandardizeDialog)
{
    ui->setupUi(this);
    config = &configuration;

    wingPrefix     = "Wing-";
    fuselagePrefix = "Fuselage-";

    QStringList components;
    for (int w = 1; w <= config->GetWingCount(); w++) {
        components.push_back(wingPrefix + config->GetWing(w).GetUID().c_str());
    }

    for (int f = 1; f <= config->GetFuselageCount(); f++) {
        components.push_back(fuselagePrefix + config->GetFuselage(f).GetUID().c_str());
    }
    components.push_back("All");

    ui->componentComboBox->addItems(components);
    int index = ui->componentComboBox->findText("All");
    ui->componentComboBox->setCurrentIndex(index);

    QStringList methods;
    methods.push_back("Complete decomposition");
    methods.push_back("Simple decomposition");
    ui->methodComboBox->addItems(methods);
    index = ui->methodComboBox->findText("Complete decomposition");
    ui->methodComboBox->setCurrentIndex(index);
}

StandardizeDialog::~StandardizeDialog()
{
    delete ui;
}

QString StandardizeDialog::getSelectedUID()
{
    QString currentText = ui->componentComboBox->currentText();
    if (isSelectedUIDAWing()) {
        currentText.remove(0, wingPrefix.size());
    }
    else if (isSelectedUIDAFuselage()) {
        currentText.remove(0, fuselagePrefix.size());
    }
    return currentText;
}

bool StandardizeDialog::isSelectedUIDAWing()
{
    QString currentText = ui->componentComboBox->currentText();
    QRegExp re;
    re.setPatternSyntax(QRegExp::RegExp2);
    re.setPattern("^" + wingPrefix + ".*");
    if (currentText.contains(re)) {
        return true;
    }
    return false;
}

bool StandardizeDialog::isSelectedUIDAFuselage()
{
    QString currentText = ui->componentComboBox->currentText();
    QRegExp re;
    re.setPatternSyntax(QRegExp::RegExp2);
    re.setPattern("^" + fuselagePrefix + ".*");
    if (currentText.contains(re)) {
        return true;
    }
    return false;
}

bool StandardizeDialog::useSimpleDecomposition()
{
    QString currentText = ui->methodComboBox->currentText();
    if (currentText == "Simple decomposition") {
        return true;
    }
    return false;
}
