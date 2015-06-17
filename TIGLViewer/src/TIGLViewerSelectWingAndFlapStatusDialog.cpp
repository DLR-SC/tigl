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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QToolTip>
#include <QSpacerItem>
#include <QPushButton>

// store deflections, values are from 0 ... 1
// TODO: this should not be static, or at least it should be cleared, if new dialog is created with
// new configuration
static std::map< std::string, double> _deflectionMap;

double sliderToRelativeDeflect(QSlider* slider) {
    return (slider->value() - slider->minimum())/(double)(slider->maximum()-slider->minimum());
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

TIGLViewerSelectWingAndFlapStatusDialog::~TIGLViewerSelectWingAndFlapStatusDialog()
{
    cleanup();
    delete ui;
}

int TIGLViewerSelectWingAndFlapStatusDialog::exec(QStringList wings)
{
    ui->comboBoxWings->addItems(wings);
    int res = QDialog::exec();
    return res;
}

std::string TIGLViewerSelectWingAndFlapStatusDialog::getSelectedWing()
{
    return ui->comboBoxWings->currentText().toStdString();
}

std::map<std::string,double> TIGLViewerSelectWingAndFlapStatusDialog::getControlSurfaceStatus()
{
    return _deflectionMap;
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_comboBoxWings_currentIndexChanged(int index)
{
    drawGUI(true);
}

void TIGLViewerSelectWingAndFlapStatusDialog::slider_value_changed(int k)
{
    QSlider* slider = dynamic_cast<QSlider*>(QObject::sender());
    std::string uid = slider->windowTitle().toStdString();
    updateLabels(uid, sliderToRelativeDeflect(slider));
    _document->updateControlSurfacesInteractiveObjects(getSelectedWing(),_deflectionMap,slider->windowTitle().toStdString());
}

void TIGLViewerSelectWingAndFlapStatusDialog::cleanup()
{
    _guiMap.clear();
    return;
}

double TIGLViewerSelectWingAndFlapStatusDialog::getTrailingEdgeFlapValue( std::string uid )
{
    return _deflectionMap[uid];
}

double TIGLViewerSelectWingAndFlapStatusDialog::linearInterpolation(std::vector<double> list1, std::vector<double> list2, double valueRelList1)
{
    double min = 0;
    double max = 0;
    int idefRem = 1;
    for ( std::vector<double>::size_type idef = 1; idef < list1.size(); idef++ ) {
        if ( list1[idef-1] <= valueRelList1 && list1[idef] >= valueRelList1 ) {
            min = list1[idef-1];
            max = list1[idef];
            idefRem = idef;
            break;
        }
    }
    double value = ( valueRelList1 - list1[idefRem-1] ) / ( list1[idefRem] - list1[idefRem-1] );
    double min2 = list2[idefRem-1];
    double max2 = list2[idefRem];
    return value * ( max2 - min2 ) + min2;
}

void TIGLViewerSelectWingAndFlapStatusDialog::drawGUI(bool redrawModel)
{
    cleanup();
    std::string wingUID = ui->comboBoxWings->currentText().toStdString();
    tigl::CCPACSConfiguration& config = _document->GetConfiguration();
    tigl::CCPACSWing &wing = config.GetWing(wingUID);

    QWidget* outerWidget = new QWidget;
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setAlignment(Qt::AlignTop);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::white);
    bool switcher = true;

    int noDevices = wing.GetComponentSegmentCount();
    for ( int i = 1; i <= wing.GetComponentSegmentCount(); i++ ) {
        tigl::CCPACSWingComponentSegment& componentSegment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(i);
        if ( componentSegment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount() < 1) {
            noDevices--;
            if (noDevices < 1) {
                QPushButton *okButton= ui->buttonBox->button(QDialogButtonBox::Ok);
                okButton->setEnabled(false);
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
        else {
            QPushButton *okButton= ui->buttonBox->button(QDialogButtonBox::Ok);
            okButton->setEnabled(true);
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

            QLabel* labelValue = new QLabel("Value: 0% ", this);
            labelValue->setFixedHeight(15);
            QString def = "Deflection: ";
            QString rot = "Rotation: ";

            // @todo: refactor, chained object calls are ugly
            if ( controlSurfaceDevice.getMovementPath().getSteps().GetStepCount() > 0 ) {
                def.append(QString::number(controlSurfaceDevice.getMovementPath().getSteps().GetStep(1).getRelDeflection()));
                rot.append(QString::number(controlSurfaceDevice.getMovementPath().getSteps().GetStep(1).getHingeLineRotation()));
            }

            QLabel* labelRotation   = new QLabel(rot, this);
            QLabel* labelDeflection = new QLabel(def, this);
            labelValue->setMargin(0);
            labelRotation->setMargin(0);
            labelRotation->setFixedHeight(15);

            labelValue->setFixedWidth(90);
            labelRotation->setFixedWidth(90);
            labelDeflection->setFixedWidth(90);

            double savedValue;
            if (_deflectionMap.find(uid.toStdString()) != _deflectionMap.end()) {
                savedValue = _deflectionMap[uid.toStdString()];
            }
            else {
                std::vector<double> relDeflections;
                std::vector<double> rotations;
                // @todo: refactor, chained object calls are ugly
                for ( int iStep = 1; iStep <= controlSurfaceDevice.getMovementPath().getSteps().GetStepCount(); iStep++ ) {
                    relDeflections.push_back(controlSurfaceDevice.getMovementPath().getSteps().GetStep(iStep).getRelDeflection());
                    rotations.push_back(controlSurfaceDevice.getMovementPath().getSteps().GetStep(iStep).getHingeLineRotation());
                }

                if (relDeflections.size() > 1) {
                    double deflection = linearInterpolation(rotations,relDeflections,0);
                    // map Deflection to Percentage.
                    double minDef = relDeflections[0];
                    double deflectionRange = relDeflections[relDeflections.size()-1] - minDef;
                    savedValue = ((deflection - minDef)/deflectionRange);

                }
                else {
                    savedValue = 0.5;
                }
            }


            labelValue->setText("Value: " + QString::number(savedValue) + "%");
            slider->setValue((int)(slider->minimum() + (slider->maximum() - slider->minimum()) * savedValue));

            DeviceLabels elements;
            elements.valueLabel = labelValue;
            elements.rotAngleLabel = labelRotation;
            elements.deflectionLabel = labelDeflection;
            _guiMap[uid.toStdString()] = elements;

            hLayout->addWidget(labelUID);
            hLayout->addWidget(slider);
            hLayout->addWidget(labelValue);
            hLayout->addWidget(labelRotation);
            hLayout->addWidget(labelDeflection);
            hLayout->addWidget(labelValue);

            innerWidget->setLayout(hLayout);

            slider->setWindowTitle( uid );
            vLayout->addWidget(innerWidget);

            if (_deflectionMap.find(uid.toStdString()) == _deflectionMap.end()) {
                _deflectionMap[uid.toStdString()] = 0;
            }

            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slider_value_changed(int)));
            _deviceMap[uid.toStdString()] = &controlSurfaceDevice;
            updateLabels(uid.toStdString(), savedValue);
        }
        outerWidget->setLayout(vLayout);
        ui->scrollArea->setWidget(outerWidget);
    }
    if (redrawModel) {
        _document->drawWingFlapsForInteractiveUse(getSelectedWing(), getControlSurfaceStatus());
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

void TIGLViewerSelectWingAndFlapStatusDialog::updateLabels(std::string controlSurfaceDeviceUID, double relDeflect)
{
    QString textVal = "Value: ";
    QString textRot = "Rotation: ";
    QString textDef = "Deflection: ";

    std::vector<double> relDeflections;
    std::vector<double> rotations;
    
    std::map< std::string, tigl::CCPACSControlSurfaceDevice*>::iterator it;
    it = _deviceMap.find(controlSurfaceDeviceUID);
    
    if (it == _deviceMap.end()) {
        return;
    }
    
    tigl::CCPACSControlSurfaceDevice* device = it->second;
    tigl::CCPACSControlSurfaceDeviceSteps steps = device->getMovementPath().getSteps();
    
    for ( int iStep = 1; iStep <= steps.GetStepCount(); iStep++ ) {
        relDeflections.push_back(steps.GetStep(iStep).getRelDeflection());
        rotations.push_back(steps.GetStep(iStep).getHingeLineRotation());
    }

    double inputDeflection = ( relDeflections.back() - relDeflections.front() ) * relDeflect  + relDeflections.front();
    double rotation = linearInterpolation( relDeflections, rotations, inputDeflection );

    textVal.append(QString::number(relDeflect*100.));
    textRot.append(QString::number(rotation));
    textDef.append(QString::number(inputDeflection));
    textVal.append("% ");

    DeviceLabels& elms = _guiMap.at(controlSurfaceDeviceUID);
    elms.valueLabel->setText(textVal);
    elms.deflectionLabel->setText(textDef);
    elms.rotAngleLabel->setText(textRot);

    _deflectionMap[controlSurfaceDeviceUID] = relDeflect;
}
