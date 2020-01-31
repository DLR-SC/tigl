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
#include "CCPACSControlSurfaceStep.h"
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

namespace
{

double sliderToRelativeDeflect(const QSlider* slider, const QDoubleSpinBox* spinBox)
{
    
    double minSlider = static_cast<double>(slider->minimum());
    double maxSlider = static_cast<double>(slider->maximum());
    double valSlider = static_cast<double>(slider->value());

    double minDeflect = spinBox->minimum();
    double maxDeflect = spinBox->maximum();
    
    return (maxDeflect - minDeflect)/(maxSlider-minSlider) * (valSlider - minSlider) + minDeflect;
}

} // namespace

TIGLViewerSelectWingAndFlapStatusDialog::TIGLViewerSelectWingAndFlapStatusDialog(TIGLViewerDocument* document, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerSelectWingAndFlapStatusDialog)
{
    ui->setupUi(this);
    setFixedSize(size());
    this->setWindowTitle("Choose ControlSurface Deflections");
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
    double inputDeflection = sliderToRelativeDeflect(elms.slider, elms.deflectionBox);
    
    SignalsBlocker block(elms.deflectionBox);
    updateWidgets(uid, inputDeflection);
    _document->updateControlSurfacesInteractiveObjects(uid);
}

void TIGLViewerSelectWingAndFlapStatusDialog::spinBox_value_changed(double inputDeflection)
{
    QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
    std::string uid = spinBox->windowTitle().toStdString();

    DeviceWidgets& elms = _guiMap.at(uid);

    SignalsBlocker block(elms.slider);
    updateWidgets(uid, inputDeflection);
    _document->updateControlSurfacesInteractiveObjects(uid);
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
        return device->GetDeflection();
    }
    catch(...) {
        return 0;
    }
}

QWidget* TIGLViewerSelectWingAndFlapStatusDialog::buildFlapRow(const tigl::CCPACSTrailingEdgeDevice& controlSurfaceDevice, const QPalette& Pal)
{
    QHBoxLayout* hLayout = new QHBoxLayout;
    QWidget* innerWidget = new QWidget;
    innerWidget->setAutoFillBackground(true);
    innerWidget->setPalette(Pal);


    QString uid = controlSurfaceDevice.GetUID().c_str();
    QLabel* labelUID = new QLabel(uid, this);
    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setMaximum(1000);

    QDoubleSpinBox* spinBox = new QDoubleSpinBox();
    spinBox->setDecimals(3);
    spinBox->setFixedWidth(60);
    spinBox->setSingleStep(0.005);
    QString rot = "Rotation: ";

    // @todo: refactor, chained object calls are ugly
    if ( controlSurfaceDevice.GetPath().GetSteps().GetSteps().size() > 0 ) {
        const auto& step = controlSurfaceDevice.GetPath().GetSteps().GetSteps().front();
        if(step) {
            rot.append(QString::number(step->GetHingeLineRotation().value_or(0.)));
        }
    }

    QLabel* labelRotation   = new QLabel(rot, this);
    labelRotation->setMargin(0);
    labelRotation->setFixedHeight(15);

    labelRotation->setFixedWidth(90);

    double savedValue = controlSurfaceDevice.GetDeflection();

    double minDeflect = controlSurfaceDevice.GetMinDeflection();
    double maxDeflect = controlSurfaceDevice.GetMaxDeflection();
    
    int newSliderValue = static_cast<int>((slider->maximum() - slider->minimum()) / (maxDeflect-minDeflect) * (savedValue - minDeflect))
                        + slider->minimum();
    slider->setValue(newSliderValue);

    spinBox->setMinimum(minDeflect);
    spinBox->setValue(savedValue);
    spinBox->setMaximum(maxDeflect);
    
    DeviceWidgets elements;
    elements.slider = slider;
    elements.deflectionBox = spinBox;
    elements.rotAngleLabel = labelRotation;
    _guiMap[uid.toStdString()] = elements;

    slider->setWindowTitle( uid );
    spinBox->setWindowTitle( uid );

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slider_value_changed(int)));
    connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(spinBox_value_changed(double)));

    hLayout->addWidget(labelUID);
    hLayout->addWidget(slider);
    hLayout->addWidget(spinBox);
    hLayout->addWidget(labelRotation);

    innerWidget->setLayout(hLayout);
    
    return innerWidget;
}

// @TODO: rewrite using MVC and table layout
void TIGLViewerSelectWingAndFlapStatusDialog::drawGUI()
{
    cleanup();
    
    QWidget* outerWidget = new QWidget;
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setAlignment(Qt::AlignTop);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::white);
    bool switcher = true;
    
    std::string wingUID = m_currentWing;

    if (wingUID.empty())
        return;

    tigl::CCPACSConfiguration& config = _document->GetConfiguration();
    tigl::CCPACSWing &wing = config.GetWing(wingUID);

    int noDevices = wing.GetComponentSegmentCount();
    for ( int i = 1; i <= wing.GetComponentSegmentCount(); i++ ) {
        tigl::CCPACSWingComponentSegment& componentSegment = static_cast<tigl::CCPACSWingComponentSegment&>(wing.GetComponentSegment(i));
        const auto& controlSurfaces = componentSegment.GetControlSurfaces();
        if (!controlSurfaces || controlSurfaces->ControlSurfaceCount() < 1) {
            noDevices--;
            if (noDevices < 1) {
                QLabel* error = new QLabel("This wing has no ControlSurfaces", this);
                error->setMargin(50);
                QWidget* innerWidgetE = new QWidget;
                innerWidgetE->setAutoFillBackground(true);
                innerWidgetE->setPalette(Pal);
                vLayout->addWidget( error );
                vLayout->addWidget(innerWidgetE);
                outerWidget->setLayout(vLayout);
                ui->scrollArea->setWidget(outerWidget);
                return;
            }
        }

        // @todo: what if no TEDS? fix this
        for (  const auto&  controlSurfaceDevice : controlSurfaces->GetTrailingEdgeDevices()->GetTrailingEdgeDevices()) {
            if (!controlSurfaceDevice) {
                continue;
            }

            if ((!ui->checkTED->isChecked() && controlSurfaceDevice->GetType() == TRAILING_EDGE_DEVICE) ||
                (!ui->checkLED->isChecked() && controlSurfaceDevice->GetType() == LEADING_EDGE_DEVICE) ||
                (!ui->checkSpoiler->isChecked() && controlSurfaceDevice->GetType() == SPOILER)) {

                continue;
            }

            QColor col = switcher ? Qt::white : Qt::lightGray;
            Pal.setColor(QPalette::Background, col);
            switcher = switcher ^ 1;

            vLayout->addWidget(buildFlapRow(*controlSurfaceDevice, Pal));

            _deviceMap[controlSurfaceDevice->GetUID()] = controlSurfaceDevice.get();
            updateWidgets(controlSurfaceDevice->GetUID(), controlSurfaceDevice->GetDeflection() );
        }

        outerWidget->setLayout(vLayout);
        ui->scrollArea->setWidget(outerWidget);
    }
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
                                                            double inputDeflection)
{
    DeviceWidgets& elms = _guiMap.at(controlSurfaceDeviceUID);

    QString textVal;
    QString textRot = "Rotation: ";

    std::map< std::string, tigl::CCPACSTrailingEdgeDevice*>::iterator it;
    it = _deviceMap.find(controlSurfaceDeviceUID);
    if (it == _deviceMap.end()) {
        return;
    }
    tigl::CCPACSTrailingEdgeDevice* device = it->second;

    // Get rotation for current deflection value
    std::vector<double> relDeflections;
    std::vector<double> rotations;
    const tigl::CCPACSControlSurfaceSteps& steps = device->GetPath().GetSteps();
    for ( const auto &step : steps.GetSteps()) {
        if (!step) continue;
        relDeflections.push_back(step->GetRelDeflection());
        rotations.push_back(step->GetHingeLineRotation().value_or(0.));
    }
    double rotation = tigl::Interpolate(relDeflections, rotations, inputDeflection);
    textRot.append(QString::number(rotation));

    double factor = ( inputDeflection - device->GetMinDeflection()  ) / ( device->GetMaxDeflection() - device->GetMinDeflection() );
    textVal.append(QString::number(100 * factor));
    textVal.append("% ");

    int sliderVal = static_cast<int>(Mix(elms.slider->minimum(), elms.slider->maximum(), factor));

    elms.slider->setValue(sliderVal);
    elms.deflectionBox->setValue(inputDeflection);
    elms.rotAngleLabel->setText(textRot);

    device->SetDeflection(inputDeflection);
}
