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

#include "ModificatorPositioningWidget.h"
#include "ui_ModificatorPositioningWidget.h"
#include "CTiglLogging.h"
#include "CTiglUIDManager.h"
#include "CCPACSWingSection.h"
#include "CCPACSFuselageSection.h"

ModificatorPositioningWidget::ModificatorPositioningWidget(QWidget* parent)
    : ModificatorWidget(parent)
    , ui(new Ui::ModificatorPositioningWidget)
{
    ui->setupUi(this);
    associatedFuselage = nullptr;
    associatedWing     = nullptr;
}

ModificatorPositioningWidget::~ModificatorPositioningWidget()
{
    delete ui;
}

void ModificatorPositioningWidget::setPositioning(tigl::CCPACSFuselage& fuselage, tigl::CCPACSPositioning& pos)
{
    associatedWing     = nullptr;
    associatedFuselage = &(fuselage);

    setPositioning(pos);
}

void ModificatorPositioningWidget::setPositioning(tigl::CCPACSWing& wing, tigl::CCPACSPositioning& pos)
{
    associatedWing     = &(wing);
    associatedFuselage = nullptr;

    setPositioning(pos);
}

void ModificatorPositioningWidget::setPositioning(tigl::CCPACSPositioning& pos)
{
    positioning = &(pos);
    ui->spinBoxLength->setValue(positioning->GetLength());
    ui->spinBoxSweep->setValue(positioning->GetSweepAngle());
    ui->spinBoxDihedral->setValue(positioning->GetDihedralAngle());
    QStringList sectionUIDs = getSectionUIDsList();
    ui->comboBoxToUID->clear();
    ui->comboBoxToUID->addItems(sectionUIDs);
    ui->comboBoxToUID->setCurrentText(positioning->GetToSectionUID().c_str());
    // for the fromUID, we can get a empty string that means that we need to start from the world origin
    sectionUIDs.push_back("");
    ui->comboBoxFromUID->clear();
    ui->comboBoxFromUID->addItems(sectionUIDs);
    if (boost::optional<std::string> from = positioning->GetFromSectionUID()) {
        ui->comboBoxFromUID->setCurrentText((*from).c_str());
    }
    else {
        ui->comboBoxFromUID->setCurrentText("");
    }
}

void ModificatorPositioningWidget::reset()
{
    if (positioning == nullptr) {
        return;
    }
    else {
        setPositioning(*positioning);
    }
}

bool ModificatorPositioningWidget::apply()
{
    if (positioning == nullptr) {
        LOG(WARNING) << "ModificatorPositioningWidget::apply: Impossible to call the apply function because the "
                        "positioning is not set for the widget.";
        return false;
    }
    bool wasModified = false;

    bool lengthHasChanged   = ((!isApprox(positioning->GetLength(), ui->spinBoxLength->value())));
    bool sweepHasChanged    = ((!isApprox(positioning->GetSweepAngle(), ui->spinBoxSweep->value())));
    bool dihedralHasChanged = ((!isApprox(positioning->GetDihedralAngle(), ui->spinBoxDihedral->value())));
    bool fromUIDHasChanged  = (ui->comboBoxFromUID->currentText().toStdString() != positioning->GetFromSectionUID());
    bool toUIDHasChanged    = (ui->comboBoxToUID->currentText().toStdString() != positioning->GetToSectionUID());

    if (lengthHasChanged) {
        positioning->SetLength(ui->spinBoxLength->value());
        wasModified = true;
    }

    if (sweepHasChanged) {
        positioning->SetSweepAngle(ui->spinBoxSweep->value());
        wasModified = true;
    }

    if (dihedralHasChanged) {
        positioning->SetDihedralAngle(ui->spinBoxDihedral->value());
        wasModified = true;
    }

    if (fromUIDHasChanged) {
        positioning->SetFromSectionUID(ui->comboBoxFromUID->currentText().toStdString());
        wasModified = true;
    }

    if (toUIDHasChanged) {
        positioning->SetToSectionUID(ui->comboBoxToUID->currentText().toStdString());
        wasModified = true;
    }

    if (wasModified) {
        if (associatedWing != nullptr && associatedFuselage == nullptr) {
            associatedWing->Invalidate();
        }
        else if (associatedWing == nullptr && associatedFuselage != nullptr) {
            associatedFuselage->Invalidate();
        }
        else {
            LOG(ERROR) << "ModificatorPositioningWidget::apply: associate wing and fuselage state are invalid!";
        }

        reset();
    }

    return wasModified;
}

QStringList ModificatorPositioningWidget::getSectionUIDsList()
{
    QStringList list;

    if (associatedWing != nullptr && associatedFuselage == nullptr) {
        for (int i = 1; i < associatedWing->GetSectionCount() + 1; i++) {
            list.push_back(associatedWing->GetSection(i).GetUID().c_str());
        }
    }
    else if (associatedWing == nullptr && associatedFuselage != nullptr) {
        for (int i = 1; i < associatedFuselage->GetSectionCount() + 1; i++) {
            list.push_back(associatedFuselage->GetSection(i).GetUID().c_str());
        }
    }

    return list;
}
