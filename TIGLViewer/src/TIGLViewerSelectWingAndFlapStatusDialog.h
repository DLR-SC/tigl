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
#include <QGridLayout>
#include <map>
#include <vector>
#include "CCPACSConfigurationManager.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "TIGLViewerDocument.h"

class QSlider;
class QLabel;
class QDoubleSpinBox;

namespace Ui
{
    class TIGLViewerSelectWingAndFlapStatusDialog;
}

class TIGLViewerSelectWingAndFlapStatusDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TIGLViewerSelectWingAndFlapStatusDialog(TIGLViewerDocument* document, QWidget* parent=0);

    void setWing(std::string wingUID) {
        m_currentWing = wingUID;
        drawGUI();
    }

    ~TIGLViewerSelectWingAndFlapStatusDialog();
    double getTrailingEdgeFlapValue( std::string uid );
    std::map<std::string,double> getDeflections();


private slots:
    void slider_value_changed(int k);
    void spinBox_value_changed(double inputDeflection);
    void on_checkTED_stateChanged(int arg1);
    void on_checkLED_stateChanged(int arg1);
    void on_checkSpoiler_stateChanged(int arg1);

private:
    Ui::TIGLViewerSelectWingAndFlapStatusDialog *ui;

    struct DeviceWidgets {
        QSlider* slider;
        QDoubleSpinBox* deflectionBox;
        QLabel* rotAngleLabel;
    };

    std::map< std::string, DeviceWidgets> _guiMap;
    std::map< std::string, tigl::CCPACSTrailingEdgeDevice*> _deviceMap;
    std::string m_currentWing;

    TIGLViewerDocument* _document;
    void updateWidgets(std::string controlSurfaceDeviceUID,
                       double inputDeflection);
    void drawGUI();
    void cleanup();


    void buildFlapRow(const tigl::CCPACSTrailingEdgeDevice& controlSurfaceDevice, int rowIdx, class QTableWidget *);
};

#endif // TIGLVIEWERSELECTWINGANDFLAPSTATUSDIALOG_H
