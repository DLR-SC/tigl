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

#include "TIGLViewerSelectWingAndFlapStatusDialog.h"
#include "ui_TIGLViewerSelectWingAndFlapStatusDialog.h"
#include "CCPACSConfigurationManager.h"
#include "CCPACSConfiguration.h"
#include "CCPACSTrailingEdgeDevice.h"
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
    SignalsBlocker(QObject* ptr):
    _ptr(ptr)
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
    
    return (maxControlParam - minControlParam)/(maxSlider-minSlider) * (valSlider - minSlider) + minControlParam;
}

} // namespace

TIGLViewerSelectWingAndFlapStatusDialog::TIGLViewerSelectWingAndFlapStatusDialog(TIGLViewerDocument* document, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerSelectWingAndFlapStatusDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Choose ControlSurface Control Parameters");
    _document = document;

}

TIGLViewerSelectWingAndFlapStatusDialog::~TIGLViewerSelectWingAndFlapStatusDialog()
{
    cleanup();
    delete ui;
}


void TIGLViewerSelectWingAndFlapStatusDialog::slider_value_changed(int /* k */)
{
    QSlider* slider = dynamic_cast<QSlider*>(QObject::sender());
    std::string uid = slider->windowTitle().toStdString();

    DeviceWidgets& elms = _guiMap.at(uid);
    double controlParm = sliderToControlParameter(elms.slider, elms.controlParamBox);
    
    SignalsBlocker block(elms.controlParamBox);
    updateWidgets(uid, controlParm);
    _document->updateFlapTransform(uid);
}

void TIGLViewerSelectWingAndFlapStatusDialog::spinBox_value_changed(double controlParam)
{
    QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
    std::string uid = spinBox->windowTitle().toStdString();

    DeviceWidgets& elms = _guiMap.at(uid);

    SignalsBlocker block(elms.slider);
    updateWidgets(uid, controlParam);
    _document->updateFlapTransform(uid);
}

void TIGLViewerSelectWingAndFlapStatusDialog::cleanup()
{
    _guiMap.clear();
    return;
}

double TIGLViewerSelectWingAndFlapStatusDialog::getTrailingEdgeFlapValue( std::string uid )
{
    try {
        std::map< std::string, tigl::CCPACSTrailingEdgeDevice*>::iterator it;
        it = _deviceMap.find(uid);
        if (it == _deviceMap.end()) {
            throw tigl::CTiglError("getTrailingEdgeFlapValue: UID not found", TIGL_UID_ERROR);
        }
        tigl::CCPACSTrailingEdgeDevice* device = it->second;
        return device->GetControlParameter();
    }
    catch(...) {
        return 0;
    }
}

void TIGLViewerSelectWingAndFlapStatusDialog::buildFlapRow(const tigl::CCPACSTrailingEdgeDevice& controlSurfaceDevice, int rowIdx, QTableWidget* gridLayout)
{

    QString uid = controlSurfaceDevice.GetUID().c_str();
    QLabel* labelUID = new QLabel(uid, this);
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

    if ( controlSurfaceDevice.GetPath().GetSteps().GetSteps().size() > 0 ) {
        const auto& step = controlSurfaceDevice.GetPath().GetSteps().GetSteps().front();
        if(step) {
            rot.append(QString::number(step->GetHingeLineRotation().value_or(0.)));
        }
    }

    QLabel* labelRotation   = new QLabel(rot, this);
    labelRotation->setStyleSheet(transparentBG);

    gridLayout->setCellWidget(rowIdx, 3, labelRotation);

    double savedValue = controlSurfaceDevice.GetControlParameter();

    double minControlParam = controlSurfaceDevice.GetMinControlParameter();
    double maxControlParam = controlSurfaceDevice.GetMaxControlParameter();
    
    int newSliderValue = static_cast<int>((slider->maximum() - slider->minimum()) / (maxControlParam-minControlParam) * (savedValue - minControlParam))
                        + slider->minimum();
    slider->setValue(newSliderValue);

    spinBox->setMinimum(minControlParam);
    spinBox->setValue(savedValue);
    spinBox->setMaximum(maxControlParam);
    
    DeviceWidgets elements;
    elements.slider = slider;
    elements.controlParamBox = spinBox;
    elements.rotAngleLabel = labelRotation;
    _guiMap[uid.toStdString()] = elements;

    slider->setWindowTitle( uid );
    spinBox->setWindowTitle( uid );

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slider_value_changed(int)));
    connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(spinBox_value_changed(double)));
}

void TIGLViewerSelectWingAndFlapStatusDialog::drawGUI()
{
    cleanup();    
    std::string wingUID = m_currentWing;

    if (wingUID.empty())
        return;

    tigl::CCPACSConfiguration& config = _document->GetConfiguration();
    tigl::CCPACSWing &wing = config.GetWing(wingUID);

    std::vector<tigl::CCPACSTrailingEdgeDevice*> devices;

    for ( int i = 1; i <= wing.GetComponentSegmentCount(); i++ ) {
        tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(i));
        const auto& controlSurfaces = componentSegment.GetControlSurfaces();
        if (!controlSurfaces.is_initialized() || controlSurfaces->ControlSurfaceCount() < 1) {
            continue;
        }

        // TODO: this logic needs to be adapted when there's support for LEDs and Spoilers
        if (controlSurfaces->GetTrailingEdgeDevices().is_initialized()) {
            for ( const auto&  controlSurfaceDevice : controlSurfaces->GetTrailingEdgeDevices()->GetTrailingEdgeDevices()) {
                if (!controlSurfaceDevice) {
                    continue;
                }

                if ((!ui->checkTED->isChecked() && controlSurfaceDevice->GetType() == TRAILING_EDGE_DEVICE) ||
                    (!ui->checkLED->isChecked() && controlSurfaceDevice->GetType() == LEADING_EDGE_DEVICE) ||
                    (!ui->checkSpoiler->isChecked() && controlSurfaceDevice->GetType() == SPOILER)) {

                    continue;
                }

                devices.push_back(controlSurfaceDevice.get());
            }
        }

    }

    auto* tableWidget = new QTableWidget(static_cast<int>(devices.size()), 4);

    int rowIdx = 0;
    for (auto* device : devices) {
        buildFlapRow(*device, rowIdx++, tableWidget);

        _deviceMap[device->GetUID()] = device;
        updateWidgets(device->GetUID(), device->GetControlParameter() );


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

void TIGLViewerSelectWingAndFlapStatusDialog::on_checkTED_stateChanged(int)
{
   drawGUI();
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_checkLED_stateChanged(int)
{
    drawGUI();
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_checkSpoiler_stateChanged(int)
{
    drawGUI();
}

void TIGLViewerSelectWingAndFlapStatusDialog::updateWidgets(std::string controlSurfaceDeviceUID,
                                                            double controlParam)
{
    DeviceWidgets& elms = _guiMap.at(controlSurfaceDeviceUID);

    QString textVal;

    std::map< std::string, tigl::CCPACSTrailingEdgeDevice*>::iterator it;
    it = _deviceMap.find(controlSurfaceDeviceUID);
    if (it == _deviceMap.end()) {
        return;
    }
    tigl::CCPACSTrailingEdgeDevice* device = it->second;

    // Get rotation for current control parameter value
    std::vector<double> controlParams;
    std::vector<double> rotations;
    const tigl::CCPACSControlSurfaceSteps& steps = device->GetPath().GetSteps();
    for ( const auto &step : steps.GetSteps()) {
        if (!step) continue;
        controlParams.push_back(step->GetControlParameter());
        rotations.push_back(step->GetHingeLineRotation().value_or(0.));
    }
    double rotation = tigl::Interpolate(controlParams, rotations, controlParam);
    QString textRot = QString::number(rotation);

    double factor = ( controlParam - device->GetMinControlParameter()  ) / ( device->GetMaxControlParameter() - device->GetMinControlParameter() );
    textVal.append(QString::number(100 * factor));
    textVal.append("% ");

    int sliderVal = static_cast<int>(Mix(elms.slider->minimum(), elms.slider->maximum(), factor));

    elms.slider->setValue(sliderVal);
    elms.controlParamBox->setValue(controlParam);
    elms.rotAngleLabel->setText(textRot);

    device->SetControlParameter(controlParam);
}
