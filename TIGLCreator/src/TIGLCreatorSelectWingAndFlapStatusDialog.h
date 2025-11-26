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

#ifndef TIGLCREATORSELECTWINGANDFLAPSTATUSDIALOG_H
#define TIGLCREATORSELECTWINGANDFLAPSTATUSDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <map>
#include <vector>
#include <variant>
#include "CCPACSConfigurationManager.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSLeadingEdgeDevice.h"
#include "TIGLCreatorDocument.h"

class QSlider;
class QLabel;
class QDoubleSpinBox;

namespace Ui
{
class TIGLCreatorSelectWingAndFlapStatusDialog;
}

class TIGLCreatorSelectWingAndFlapStatusDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TIGLCreatorSelectWingAndFlapStatusDialog(TIGLCreatorDocument* document, QWidget* parent = 0);

    void setWing(std::string wingUID)
    {
        m_currentWing = wingUID;
        drawGUI();
    }

    ~TIGLCreatorSelectWingAndFlapStatusDialog();
    double getFlapValue(std::string uid);
    std::map<std::string, double> getControlParameters();

private slots:
    void slider_value_changed(int k);
    void spinBox_value_changed(double controlParam);
    void on_checkTED_stateChanged(int arg1);
    void on_checkLED_stateChanged(int arg1);
    void on_checkSpoiler_stateChanged(int arg1);

private:
    Ui::TIGLCreatorSelectWingAndFlapStatusDialog* ui;

    struct DeviceWidgets {
        QSlider* slider;
        QDoubleSpinBox* controlParamBox;
        QLabel* rotAngleLabel;
    };

    struct RotationData {
        double controlParam;
        double rotation;
        double factor;
    };

    struct TED 
    {
        tigl::CCPACSTrailingEdgeDevice* device;
        std::string uid() const { return device->GetUID(); }
        double control_parameter() const { return device->GetControlParameter(); }
    };

    struct LED 
    {
        tigl::CCPACSLeadingEdgeDevice* device;
        std::string uid() const { return device->GetUID(); }
        double control_parameter() const { return device->GetControlParameter(); }
    };

    using ControlDevice = std::variant<TED, LED>;

    std::map<std::string, DeviceWidgets> _guiMap;
    std::map<std::string, tigl::CTiglAbstractGeometricComponent*> _deviceMap;
    std::string m_currentWing;
    
    TIGLCreatorDocument* _document;
    void updateWidgets(std::string controlSurfaceDeviceUID, double controlParam);
    template <typename DeviceType> RotationData getRotation(const DeviceType* device, double controlParam);
    auto& uidMgr() { return _document->GetConfiguration().GetUIDManager(); }
    
    void drawGUI();
    void cleanup();

    void buildFlapRow(const ControlDevice& device, int rowIdx,
                      class QTableWidget*);
    template <typename DeviceType>
    void buildFlapRow_helper(const DeviceType& controlSurfaceDevice, int rowIdx, QTableWidget* gridLayout);
};

#endif // TIGLCREATORSELECTWINGANDFLAPSTATUSDIALOG_H
