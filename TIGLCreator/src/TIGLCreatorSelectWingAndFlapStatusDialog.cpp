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

#include "TIGLCreatorSelectWingAndFlapStatusDialog.h"
#include "ui_TIGLCreatorSelectWingAndFlapStatusDialog.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSTrailingEdgeDevice.h"
#include "CCPACSLeadingEdgeDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"
#include "generated/CPACSControlSurfaceStep.h"
#include "tiglmathfunctions.h"
#include "tiglcommonfunctions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolTip>
#include <QSpacerItem>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpacerItem>
#include <QTableWidget>
#include <QHeaderView>
#include <QtGlobal>

namespace
{

class SignalsBlocker
{
public:
    SignalsBlocker(QObject* ptr)
        : _ptr(ptr)
    {
        _b = ptr->blockSignals(true);
    }
    ~SignalsBlocker()
    {
        _ptr->blockSignals(_b);
    }

private:
    QObject* _ptr;
    bool _b;
};

double sliderToControlParameter(const QSlider* slider, const QDoubleSpinBox* spinBox)
{

    double minSlider = static_cast<double>(slider->minimum());
    double maxSlider = static_cast<double>(slider->maximum());
    double valSlider = static_cast<double>(slider->value());

    double minControlParam = spinBox->minimum();
    double maxControlParam = spinBox->maximum();

    return (maxControlParam - minControlParam) / (maxSlider - minSlider) * (valSlider - minSlider) + minControlParam;
}

} // namespace

TIGLCreatorSelectWingAndFlapStatusDialog::TIGLCreatorSelectWingAndFlapStatusDialog(TIGLCreatorDocument* document,
                                                                                   QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::TIGLCreatorSelectWingAndFlapStatusDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Choose ControlSurface Control Parameters");
    _document = document;
}

TIGLCreatorSelectWingAndFlapStatusDialog::~TIGLCreatorSelectWingAndFlapStatusDialog()
{
    cleanup();
    delete ui;
}

void TIGLCreatorSelectWingAndFlapStatusDialog::slider_value_changed(int /* k */)
{
    QSlider* slider = dynamic_cast<QSlider*>(QObject::sender());
    std::string uid = slider->windowTitle().toStdString();

    DeviceWidgets& elms = _guiMap.at(uid);
    double controlParm  = sliderToControlParameter(elms.slider, elms.controlParamBox);

    SignalsBlocker block(elms.controlParamBox);
    updateWidgets(uid, controlParm);
    _document->updateFlapTransform(uid);
}

void TIGLCreatorSelectWingAndFlapStatusDialog::spinBox_value_changed(double controlParam)
{
    QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
    std::string uid         = spinBox->windowTitle().toStdString();

    DeviceWidgets& elms = _guiMap.at(uid);

    SignalsBlocker block(elms.slider);
    updateWidgets(uid, controlParam);
    _document->updateFlapTransform(uid);
}

void TIGLCreatorSelectWingAndFlapStatusDialog::cleanup()
{
    _guiMap.clear();
    return;
}

double TIGLCreatorSelectWingAndFlapStatusDialog::getFlapValue(std::string uid)
{
    if (uidMgr().IsType<tigl::CCPACSTrailingEdgeDevice>(uid)) {
        auto& ted = uidMgr().ResolveObject<tigl::CCPACSTrailingEdgeDevice>(uid);
        return ted.GetControlParameter();
    }
    else if (uidMgr().IsType<tigl::CCPACSLeadingEdgeDevice>(uid)) {
        auto& led = uidMgr().ResolveObject<tigl::CCPACSLeadingEdgeDevice>(uid);
        return led.GetControlParameter();
    }
    else {
        throw tigl::CTiglError("getFlapValue: UID not found", TIGL_UID_ERROR);
    }
    
}

void TIGLCreatorSelectWingAndFlapStatusDialog::buildFlapRow(
     const ControlDevice& device, int rowIdx, QTableWidget* gridLayout)
{
    std::visit([&](auto& device) {
        buildFlapRow_helper(device, rowIdx, gridLayout);
    }, device);
}

template <typename DeviceType>
void TIGLCreatorSelectWingAndFlapStatusDialog::buildFlapRow_helper(const DeviceType& controlSurfaceDevice, int rowIdx,
                                                                   QTableWidget* gridLayout)
{
    QString uid           = QString::fromStdString(controlSurfaceDevice.uid());
    QLabel* labelUID      = new QLabel(uid, this);
    QString transparentBG = "background-color: rgba(0, 0, 0, 0.0); padding-left: 6px; padding-right: 6px;";
    labelUID->setStyleSheet(transparentBG);

    gridLayout->setCellWidget(rowIdx, 0, labelUID);

    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setMaximum(1000);
    slider->setStyleSheet(transparentBG);

    gridLayout->setCellWidget(rowIdx, 1, slider);

    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setDecimals(3);
    spinBox->setSingleStep(0.005);
    spinBox->setStyleSheet(transparentBG);

    gridLayout->setCellWidget(rowIdx, 2, spinBox);

    QString rot;

    if (controlSurfaceDevice.device->GetPath().GetSteps().GetSteps().size() > 0) {
        const auto& step = controlSurfaceDevice.device->GetPath().GetSteps().GetSteps().front();
        if (step) {
            rot.append(QString::number(step->GetHingeLineRotation().value_or(0.)));
        }
    }

    QLabel* labelRotation = new QLabel(rot, this);
    labelRotation->setStyleSheet(transparentBG);

    gridLayout->setCellWidget(rowIdx, 3, labelRotation);

    double savedValue = controlSurfaceDevice.control_parameter();

    double minControlParam = controlSurfaceDevice.device->GetMinControlParameter();
    double maxControlParam = controlSurfaceDevice.device->GetMaxControlParameter();

    int newSliderValue = static_cast<int>((slider->maximum() - slider->minimum()) /
                                          (maxControlParam - minControlParam) * (savedValue - minControlParam)) +
                         slider->minimum();
    slider->setValue(newSliderValue);

    spinBox->setMinimum(minControlParam);
    spinBox->setValue(savedValue);
    spinBox->setMaximum(maxControlParam);

    DeviceWidgets elements;
    elements.slider            = slider;
    elements.controlParamBox   = spinBox;
    elements.rotAngleLabel     = labelRotation;
    _guiMap[uid.toStdString()] = elements;

    slider->setWindowTitle(uid);
    spinBox->setWindowTitle(uid);

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slider_value_changed(int)));
    connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(spinBox_value_changed(double)));
}

void TIGLCreatorSelectWingAndFlapStatusDialog::drawGUI()
{
    cleanup();

    std::string wingUID = m_currentWing;

    if (wingUID.empty())
        return;

    tigl::CCPACSConfiguration& config = _document->GetConfiguration();
    tigl::CCPACSWing& wing            = config.GetWing(wingUID);

    std::vector<ControlDevice> devices;

    for (int i = 1; i <= wing.GetComponentSegmentCount(); i++) {
        tigl::CCPACSWingComponentSegment& componentSegment =
            static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(i));
        const auto& controlSurfaces = componentSegment.GetControlSurfaces();
        if (!controlSurfaces.is_initialized() || controlSurfaces->ControlSurfaceCount() < 1) {
            continue;
        }

        if (controlSurfaces->GetTrailingEdgeDevices().is_initialized()) {
            for (auto& device : controlSurfaces->GetTrailingEdgeDevices()->GetTrailingEdgeDevices()) {
                if (!device || !ui->checkTED->isChecked())
                    continue;
                devices.emplace_back(TED{device.get()});
            }
        }
        if (controlSurfaces->GetLeadingEdgeDevices().is_initialized()) {
            for (auto& device : controlSurfaces->GetLeadingEdgeDevices()->GetLeadingEdgeDevices()) {
                if (!device || !ui->checkLED->isChecked())
                    continue;
                devices.emplace_back(LED{device.get()});
            }
        }
    }

    auto* tableWidget = new QTableWidget(static_cast<int>(devices.size()), 4);

    int rowIdx = 0;
    for (const ControlDevice& device : devices) {

        buildFlapRow(device, rowIdx++, tableWidget);

        std::visit([&](auto& control_device) {
            auto* device = control_device.device;  

            _deviceMap[control_device.uid()] = device;

            updateWidgets(control_device.uid(), control_device.control_parameter());
        }, device);
    }

    // set style
    tableWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    tableWidget->setAlternatingRowColors(true);
#if QT_VERSION >= 0x050000
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
    tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif
    tableWidget->setHorizontalHeaderLabels(QStringList({"", "", "Control Parameter", "Rotation"}));
    tableWidget->verticalHeader()->hide();
    tableWidget->setStyleSheet("QHeaderView::section { border: 0px solid black}");

    ui->scrollArea->setWidget(tableWidget);
}

void TIGLCreatorSelectWingAndFlapStatusDialog::on_checkTED_stateChanged(int)
{
    drawGUI();
}

void TIGLCreatorSelectWingAndFlapStatusDialog::on_checkLED_stateChanged(int)
{
    drawGUI();
}

void TIGLCreatorSelectWingAndFlapStatusDialog::on_checkSpoiler_stateChanged(int)
{
    drawGUI();
}

void TIGLCreatorSelectWingAndFlapStatusDialog::updateWidgets(std::string controlSurfaceDeviceUID, double controlParam)
{
    DeviceWidgets& elms = _guiMap.at(controlSurfaceDeviceUID);

    QString textVal;

    if (uidMgr().IsType<tigl::CCPACSTrailingEdgeDevice>(controlSurfaceDeviceUID)) {
        auto& ted = uidMgr().ResolveObject<tigl::CCPACSTrailingEdgeDevice>(controlSurfaceDeviceUID);
        auto rotationdata = getRotation(&ted, controlParam);
        int sliderVal     = static_cast<int>(Mix(elms.slider->minimum(), elms.slider->maximum(), rotationdata.factor));

        QString textRot = QString::number(rotationdata.rotation);

        elms.slider->setValue(sliderVal);
        elms.controlParamBox->setValue(rotationdata.controlParam);
        elms.rotAngleLabel->setText(textRot);
        ted.SetControlParameter(rotationdata.controlParam);
    }
    else if (uidMgr().IsType<tigl::CCPACSLeadingEdgeDevice>(controlSurfaceDeviceUID)) {
        auto& led = uidMgr().ResolveObject<tigl::CCPACSLeadingEdgeDevice>(controlSurfaceDeviceUID);
        auto rotationdata = getRotation(&led, controlParam);
        int sliderVal     = static_cast<int>(Mix(elms.slider->minimum(), elms.slider->maximum(), rotationdata.factor));

        QString textRot = QString::number(rotationdata.rotation);

        elms.slider->setValue(sliderVal);
        elms.controlParamBox->setValue(rotationdata.controlParam);
        elms.rotAngleLabel->setText(textRot);
        led.SetControlParameter(rotationdata.controlParam);
    }
}

template <typename DeviceType>
TIGLCreatorSelectWingAndFlapStatusDialog::RotationData
TIGLCreatorSelectWingAndFlapStatusDialog::getRotation(const DeviceType* device, double controlParam)
{
    QString textVal;
    // Get rotation for current control parameter value
    std::vector<double> controlParams;
    std::vector<double> rotations;
    const tigl::CCPACSControlSurfaceSteps& steps = device->GetPath().GetSteps();
    for (const auto& step : steps.GetSteps()) {
        if (!step)
            continue;
        controlParams.push_back(step->GetControlParameter());
        rotations.push_back(step->GetHingeLineRotation().value_or(0.));
    }
    double rotation = tigl::Interpolate(controlParams, rotations, controlParam);
    QString textRot = QString::number(rotation);

    double factor = (controlParam - device->GetMinControlParameter()) /
                    (device->GetMaxControlParameter() - device->GetMinControlParameter());
    textVal.append(QString::number(100 * factor));
    textVal.append("% ");

    return {controlParam, rotation, factor};
}
