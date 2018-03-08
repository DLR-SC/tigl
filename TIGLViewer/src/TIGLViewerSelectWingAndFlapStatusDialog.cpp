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
#include "CCPACSControlSurfaceDevice.h"
#include "CCPACSWingComponentSegment.h"
#include "CCPACSWing.h"
#include "tiglcommonfunctions.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolTip>
#include <QSpacerItem>
#include <QPushButton>
#include <QDoubleSpinBox>

double sliderToRelativeDeflect(const QSlider* slider, const QDoubleSpinBox* spinBox) {
    
    double minSlider = (double) slider->minimum();
    double maxSlider = (double) slider->maximum();
    double valSlider = (double) slider->value();

    double minDeflect = spinBox->minimum();
    double maxDeflect = spinBox->maximum();
    
    return (maxDeflect - minDeflect)/(maxSlider-minSlider) * (valSlider - minSlider) + minDeflect;
}

TIGLViewerSelectWingAndFlapStatusDialog::TIGLViewerSelectWingAndFlapStatusDialog(TIGLViewerDocument* document, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TIGLViewerSelectWingAndFlapStatusDialog)
{
    ui->setupUi(this);
    setFixedSize(size());
    this->setWindowTitle("Choose ControlSurface Deflections");
    _document = document;
    QPalette Pal(palette());
    ui->scrollArea->setPalette(Pal);

}

void TIGLViewerSelectWingAndFlapStatusDialog::setWings(QStringList list)
{
    bool wasBlocked = ui->comboBoxWings->blockSignals(true);
    ui->comboBoxWings->clear();
    ui->comboBoxWings->blockSignals(wasBlocked);
    ui->comboBoxWings->addItems(list);
}

TIGLViewerSelectWingAndFlapStatusDialog::~TIGLViewerSelectWingAndFlapStatusDialog()
{
    cleanup();
    delete ui;
}

std::string TIGLViewerSelectWingAndFlapStatusDialog::getSelectedWing()
{
    return ui->comboBoxWings->currentText().toStdString();
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_comboBoxWings_currentIndexChanged(int index)
{
    drawGUI(true);
}

void TIGLViewerSelectWingAndFlapStatusDialog::slider_value_changed(int k)
{
    QSlider* slider = dynamic_cast<QSlider*>(QObject::sender());
    std::string uid = slider->windowTitle().toStdString();

    DeviceWidgets& elms = _guiMap.at(uid);
    double inputDeflection = sliderToRelativeDeflect(elms.slider, elms.deflectionBox);
    
    SignalsBlocker block(elms.deflectionBox);
    updateWidgets(uid, inputDeflection);
    _document->updateControlSurfacesInteractiveObjects(getSelectedWing(), uid);
}

void TIGLViewerSelectWingAndFlapStatusDialog::spinBox_value_changed(double inputDeflection)
{
    QDoubleSpinBox* spinBox = dynamic_cast<QDoubleSpinBox*>(QObject::sender());
    std::string uid = spinBox->windowTitle().toStdString();

    DeviceWidgets& elms = _guiMap.at(uid);

    SignalsBlocker block(elms.slider);
    updateWidgets(uid, inputDeflection);
    _document->updateControlSurfacesInteractiveObjects(getSelectedWing(), uid);
}

void TIGLViewerSelectWingAndFlapStatusDialog::cleanup()
{
    _guiMap.clear();
    return;
}

double TIGLViewerSelectWingAndFlapStatusDialog::getTrailingEdgeFlapValue( std::string uid )
{
    try {
        std::map< std::string, tigl::CCPACSControlSurfaceDevice*>::iterator it;
        it = _deviceMap.find(uid);
        if (it == _deviceMap.end()) {
            throw tigl::CTiglError("getTrailingEdgeFlapValue: UID not found", TIGL_UID_ERROR);
        }
        tigl::CCPACSControlSurfaceDevice* device = it->second;
        return device->GetDeflection();
    }
    catch(...) {
        return 0;
    }
}

// @TODO: rewrite using MVC and table layout
void TIGLViewerSelectWingAndFlapStatusDialog::drawGUI(bool redrawModel)
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
    
    std::string wingUID = ui->comboBoxWings->currentText().toStdString();
    tigl::CCPACSConfiguration& config = _document->GetConfiguration();
    tigl::CCPACSWing &wing = config.GetWing(wingUID);

    int noDevices = wing.GetComponentSegmentCount();
    for ( int i = 1; i <= wing.GetComponentSegmentCount(); i++ ) {
        tigl::CCPACSWingComponentSegment& componentSegment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(i);
        if ( componentSegment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount() < 1) {
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
        
        for ( int j = 1; j <= componentSegment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount(); j++ ) {
            tigl::CCPACSControlSurfaceDevice& controlSurfaceDevice = componentSegment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceByID(j);

            if ((!ui->checkTED->isChecked() && controlSurfaceDevice.getType() == TRAILING_EDGE_DEVICE) ||
                (!ui->checkLED->isChecked() && controlSurfaceDevice.getType() == LEADING_EDGE_DEVICE) ||
                (!ui->checkSpoiler->isChecked() && controlSurfaceDevice.getType() == SPOILER)) {

                continue;
            }

            if (switcher) {
                Pal.setColor(QPalette::Background, Qt::white);
                switcher = false;
            }
            else {
                Pal.setColor(QPalette::Background, Qt::lightGray);
                switcher = true;
            }

            QHBoxLayout* hLayout = new QHBoxLayout;
            QWidget* innerWidget = new QWidget;
            QWidget* innerWidget2 = new QWidget;
            innerWidget->setAutoFillBackground(true);
            innerWidget->setPalette(Pal);
            innerWidget2->setAutoFillBackground(true);
            innerWidget2->setPalette(Pal);

            QString uid = controlSurfaceDevice.GetUID().c_str();
            QLabel* labelUID = new QLabel(uid, this);
            labelUID->setFixedSize(250,15);
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setFixedSize(90,15);
            slider->setMaximum(1000);

            QLabel* labelValue = new QLabel("0%   ", this);
            labelValue->setFixedHeight(15);
            QString def = "Deflection: ";
            QDoubleSpinBox* spinBox = new QDoubleSpinBox();
            spinBox->setDecimals(3);
            spinBox->setFixedWidth(60);
            spinBox->setSingleStep(0.005);
            QString rot = "Rotation: ";

            // @todo: refactor, chained object calls are ugly
            if ( controlSurfaceDevice.getMovementPath().getSteps().GetStepCount() > 0 ) {
                rot.append(QString::number(controlSurfaceDevice.getMovementPath().getSteps().GetStep(1).getHingeLineRotation()));
            }

            QLabel* labelRotation   = new QLabel(rot, this);
            QLabel* labelDeflection = new QLabel(def, this);
            labelValue->setMargin(0);
            labelRotation->setMargin(0);
            labelRotation->setFixedHeight(15);

            labelValue->setFixedWidth(40);
            labelRotation->setFixedWidth(90);
            labelDeflection->setFixedWidth(90);
            labelDeflection->setAlignment(Qt::AlignRight);

            double savedValue = controlSurfaceDevice.GetDeflection();
            labelValue->setText("Value: " + QString::number(savedValue) + "%");

            double minDeflect = controlSurfaceDevice.GetMinDeflection();
            double maxDeflect = controlSurfaceDevice.GetMaxDeflection();
            
            int newSliderValue = (slider->maximum() - slider->minimum())/ (maxDeflect-minDeflect) * (savedValue - minDeflect)
                    + slider->minimum();
            slider->setValue(newSliderValue);

            spinBox->setMinimum(minDeflect);
            spinBox->setValue(savedValue);
            spinBox->setMaximum(maxDeflect);
            
            DeviceWidgets elements;
            elements.slider = slider;
            elements.valueLabel = labelValue;
            elements.deflectionBox = spinBox;
            elements.rotAngleLabel = labelRotation;
            _guiMap[uid.toStdString()] = elements;

            hLayout->addWidget(labelUID);
            hLayout->addWidget(slider);
            hLayout->addWidget(labelValue);
            hLayout->addWidget(labelDeflection);
            hLayout->addWidget(spinBox);
            hLayout->addWidget(labelRotation);

            innerWidget->setLayout(hLayout);

            slider->setWindowTitle( uid );
            spinBox->setWindowTitle( uid );
            vLayout->addWidget(innerWidget);

            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slider_value_changed(int)));
            connect(spinBox, SIGNAL(valueChanged(double)), this, SLOT(spinBox_value_changed(double)));
            _deviceMap[uid.toStdString()] = &controlSurfaceDevice;
            updateWidgets(uid.toStdString(), savedValue );
        }
        outerWidget->setLayout(vLayout);
        ui->scrollArea->setWidget(outerWidget);
    }
    if (redrawModel) {
        _document->drawWingFlapsForInteractiveUse(getSelectedWing());
    }
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_checkTED_stateChanged(int arg1)
{
   drawGUI(false);
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_checkLED_stateChanged(int arg1)
{
    drawGUI(false);
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_checkSpoiler_stateChanged(int arg1)
{
    drawGUI(false);
}

void TIGLViewerSelectWingAndFlapStatusDialog::updateWidgets(std::string controlSurfaceDeviceUID,
                                                            double inputDeflection)
{
    DeviceWidgets& elms = _guiMap.at(controlSurfaceDeviceUID);

    QString textVal;
    QString textRot = "Rotation: ";

    std::map< std::string, tigl::CCPACSControlSurfaceDevice*>::iterator it;
    it = _deviceMap.find(controlSurfaceDeviceUID);
    if (it == _deviceMap.end()) {
        return;
    }
    tigl::CCPACSControlSurfaceDevice* device = it->second;

    // Get rotation for current deflection value
    std::vector<double> relDeflections;
    std::vector<double> rotations;
    tigl::CCPACSControlSurfaceDeviceSteps steps = device->getMovementPath().getSteps();
    for ( int iStep = 1; iStep <= steps.GetStepCount(); iStep++ ) {
        relDeflections.push_back(steps.GetStep(iStep).getRelDeflection());
        rotations.push_back(steps.GetStep(iStep).getHingeLineRotation());
    }
    double rotation = Interpolate(relDeflections, rotations, inputDeflection);
    textRot.append(QString::number(rotation));

    double factor = ( inputDeflection - device->GetMinDeflection()  ) / ( device->GetMaxDeflection() - device->GetMinDeflection() );
    textVal.append(QString::number(100 * factor));
    textVal.append("% ");

    int sliderVal = (int) ( factor*(elms.slider->maximum() - elms.slider->minimum()) + elms.slider->minimum() );

    elms.slider->setValue(sliderVal);
    elms.deflectionBox->setValue(inputDeflection);
    elms.valueLabel->setText(textVal);
    elms.rotAngleLabel->setText(textRot);

    device->SetDeflection(inputDeflection);
}
