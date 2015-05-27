/*
 * Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-01-28 Mark Geiger <Mark.Geiger@dlr.de>
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

#ifndef TIGLVIEWERSELECTWINGANDFLAPSTATUSDIALOG_H
#define TIGLVIEWERSELECTWINGANDFLAPSTATUSDIALOG_H

#include <QDialog>
#include <map>
#include <vector>
#include <QLabel>
#include "CCPACSConfigurationManager.h"
#include "CCPACSControlSurfaceDevice.h"
#include "TIGLViewerDocument.h"

namespace Ui
{
    class TIGLViewerSelectWingAndFlapStatusDialog;
}

static std::map< std::string, double> _controlSurfaceDevices;

class TIGLViewerSelectWingAndFlapStatusDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TIGLViewerSelectWingAndFlapStatusDialog(QWidget *parent, TiglCPACSConfigurationHandle &handle, TIGLViewerDocument* document);
    ~TIGLViewerSelectWingAndFlapStatusDialog();
    int exec(QStringList wings);
    std::string getSelectedWing();
    double getTrailingEdgeFlapValue( std::string uid );
    std::map<std::string,double> getControlSurfaceStatus();


private slots:
    void on_comboBoxWings_currentIndexChanged(int index);
    void slider_value_changed(int k);
    void on_checkTED_stateChanged(int arg1);
    void on_checkLED_stateChanged(int arg1);
    void on_checkSpoiler_stateChanged(int arg1);

private:
    bool switcher;
    double linearInterpolation(std::vector<double> list1, std::vector<double> list2, double valueRelList1);
    Ui::TIGLViewerSelectWingAndFlapStatusDialog *ui;
    TiglCPACSConfigurationHandle _handle;
    std::map< std::string, QLabel*> _displayer;
    std::map< std::string, QLabel*> _displayer_deflection;
    std::map< std::string, QLabel*> _displayer_rotation;
    std::map< std::string, tigl::CCPACSControlSurfaceDevice*> _controlSurfaceDevicesPointer;
    TIGLViewerDocument* _document;
    void setSliderAndLabels(std::string controlSurfaceDeviceUID, int k);
    void drawGUI(bool redrawModel);
    void cleanup();


};

#endif // TIGLVIEWERSELECTWINGANDFLAPSTATUSDIALOG_H
