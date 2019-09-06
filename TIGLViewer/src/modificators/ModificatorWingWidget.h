/*
 * Copyright (C) 2018 CFS Engineering
 *
 * Created: 2018 Malo Drougard <malo.drougard@protonmail.com>
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

#ifndef MODIFICATORWINGWIDGET_H
#define MODIFICATORWINGWIDGET_H

#include "ModificatorWidget.h"
#include "CCPACSWing.h"
#include "ProfilesDBManager.h"

namespace Ui
{
class ModificatorWingWidget;
}

class ModificatorWingWidget : public ModificatorWidget
{
    Q_OBJECT

public slots:

    // update the sweep according the displayed chord value,
    // used to keep display sweep value consitant with the displayed chord value
    // Remark the the double parameter is just to match the signature of the "valueChanged" signal of the chord spinbox.
    void updateSweepAccordingChordValue(double dummy = 0);
    void updateDihedralAccordingChordValue(double dummy = 0);

    void setAreaConstant(bool checked);
    void setSpanConstant(bool checked);
    void setARConstant(bool checked);

public:
    explicit ModificatorWingWidget(QWidget* parent = nullptr);
    ~ModificatorWingWidget();

    bool apply() override;
    void reset() override;

    void setWing(tigl::CCPACSWing& wing, ProfilesDBManager* profilesDB);

private:
    void init();

    Ui::ModificatorWingWidget* ui;
    tigl::CCPACSWing* tiglWing;
    ProfilesDBManager* profilesDB;

    // internal root LE is a CTiglPointWidget

    // internal rotation is a CTiglPointWidget

    // internal symmetry is a SymmetryComboBoxWidget

    // internal sweep
    double internalSweep;
    double internalSweepChord;

    // internal dihedral
    double internalDihedral;
    double internalDihedralChord;

    // internal area
    double internalArea;

    // internal span
    double internalSpan;

    // internal AR
    double internalAR;

    QString internalProfile;
};

#endif // MODIFICATORWINGWIDGET_H
