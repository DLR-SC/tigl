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

TIGLViewerSelectWingAndFlapStatusDialog::TIGLViewerSelectWingAndFlapStatusDialog(QWidget *parent, TiglCPACSConfigurationHandle &handle, TIGLViewerDocument* document) :
    QDialog(parent),
    ui(new Ui::TIGLViewerSelectWingAndFlapStatusDialog)
{
    _handle = handle;
    ui->setupUi(this);
    switcher = true;
    setFixedSize(size());
    this->setWindowTitle("Choose ControlSurface Deflection");
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
    return _controlSurfaceDevices;
}

void TIGLViewerSelectWingAndFlapStatusDialog::on_comboBoxWings_currentIndexChanged(int index)
{
    drawGUI(true);
}

void TIGLViewerSelectWingAndFlapStatusDialog::slider_value_changed(int k)
{
    QSlider* slider = dynamic_cast<QSlider*>(QObject::sender());
    std::string uid = slider->windowTitle().toStdString();
    setSliderAndLabels(uid,k);
    _document->updateControlSurfacesInteractiveObjects(getSelectedWing(),_controlSurfaceDevices,slider->windowTitle().toStdString());
}

void TIGLViewerSelectWingAndFlapStatusDialog::cleanup()
{
    _displayer.clear();
    _displayer_deflection.clear();
    _displayer_rotation.clear();
    return;
}

double TIGLViewerSelectWingAndFlapStatusDialog::getTrailingEdgeFlapValue( std::string uid )
{
    return _controlSurfaceDevices[uid];
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
    tigl::CCPACSWing &wing = tigl::CCPACSConfigurationManager::GetInstance().GetConfiguration(_handle).GetWing(wingUID);

    QWidget* outerWidget = new QWidget;
    QVBoxLayout* vLayout = new QVBoxLayout;
    vLayout->setAlignment(Qt::AlignTop);
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::white);
    switcher = true;

    int noDevices = wing.GetComponentSegmentCount();
    for ( int i = 1; i <= wing.GetComponentSegmentCount(); i++ ) {
        tigl::CCPACSWingComponentSegment& componentSegment = (tigl::CCPACSWingComponentSegment&) wing.GetComponentSegment(i);
        if ( componentSegment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount() < 1) {
            noDevices--;
            if (noDevices < 1) {
                QPushButton *okButton= ui->buttonBox->button(QDialogButtonBox::Ok);
                okButton->setEnabled(false);
                QLabel* error = new QLabel("This wing has no ControlSurfaces");
                error->setMargin(50);
                vLayout->addWidget( error );
            }
        }
        else {
            QPushButton *okButton= ui->buttonBox->button(QDialogButtonBox::Ok);
            okButton->setEnabled(true);
        }

        for ( int j = 1; j <= componentSegment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceCount(); j++ ) {
            tigl::CCPACSControlSurfaceDevice& controlSurfaceDevice = componentSegment.getControlSurfaces().getControlSurfaceDevices()->getControlSurfaceDeviceByID(j);

            if ((!ui->checkTED->isChecked() && controlSurfaceDevice.getType() == TRAILING_EDGE_DEVICE)
                    || (!ui->checkLED->isChecked() && controlSurfaceDevice.getType() == LEADING_EDGE_DEVICE)
                    || (!ui->checkSpoiler->isChecked() && controlSurfaceDevice.getType() == SPOILER))
            {
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

            QString uid = controlSurfaceDevice.getUID().c_str();
            QLabel* label = new QLabel(uid);
            label->setFixedSize(250,15);
            QSlider* slider = new QSlider(Qt::Horizontal);
            slider->setFixedSize(90,15);
            slider->setMaximum(1000);

            QLabel* displayer = new QLabel("Value: 0% ");
            displayer->setFixedHeight(15);
            QString def = "Deflection: ";
            QString rot = "Rotation: ";

            if ( controlSurfaceDevice.getMovementPath().getSteps().getControlSurfaceDeviceStepCount() > 0 ) {
                def.append(QString::number(controlSurfaceDevice.getMovementPath().getSteps().getControlSurfaceDeviceStepByID(1).getRelDeflection()));
                rot.append(QString::number(controlSurfaceDevice.getMovementPath().getSteps().getControlSurfaceDeviceStepByID(1).getHingeLineRotation()));
            }

            QLabel* display_rotation = new QLabel(rot);
            QLabel* display_deflection = new QLabel(def);
            displayer->setMargin(0);
            display_rotation->setMargin(0);
            display_rotation->setFixedHeight(15);

            displayer->setFixedWidth(90);
            display_rotation->setFixedWidth(90);
            display_deflection->setFixedWidth(90);

            double savedValue;
            if (_controlSurfaceDevices.find(uid.toStdString()) != _controlSurfaceDevices.end()) {
                savedValue = _controlSurfaceDevices[uid.toStdString()];
            } else {
                std::vector<double> relDeflections;
                std::vector<double> rotations;
                for ( int steps = 0; steps < controlSurfaceDevice.getMovementPath().getSteps().getControlSurfaceDeviceStepCount(); steps++ )
                {
                    relDeflections.push_back(controlSurfaceDevice.getMovementPath().getSteps().getControlSurfaceDeviceStepByID(steps+1).getRelDeflection());
                    rotations.push_back(controlSurfaceDevice.getMovementPath().getSteps().getControlSurfaceDeviceStepByID(steps+1).getHingeLineRotation());
                }

                if (relDeflections.size() > 1) {
                    double deflection = linearInterpolation(rotations,relDeflections,0);
                    // map Deflection to Percentage.
                    double minDef = relDeflections[0];
                    double deflectionRange = relDeflections[relDeflections.size()-1] - minDef;
                    savedValue = ((deflection - minDef)/deflectionRange)*100;

                } else {
                    savedValue = 50;
                }
            }


            displayer->setText("Value: " + QString::number(savedValue) + "%");
            slider->setValue((int) savedValue*10);

            _displayer[uid.toStdString()] = displayer;
            _displayer_rotation[uid.toStdString()] = display_rotation;
            _displayer_deflection[uid.toStdString()] = display_deflection;

            hLayout->addWidget(label);
            hLayout->addWidget(slider);
            hLayout->addWidget(displayer);
            hLayout->addWidget(display_rotation);
            hLayout->addWidget(display_deflection);
            hLayout->addWidget(displayer);

            innerWidget->setLayout(hLayout);

            slider->setWindowTitle( uid );
            vLayout->addWidget(innerWidget);

            if ( _controlSurfaceDevices[uid.toStdString()] == NULL )
            {
               _controlSurfaceDevices[uid.toStdString()] = 0;
            }

            connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slider_value_changed(int)));
            _controlSurfaceDevicesPointer[uid.toStdString()] = &controlSurfaceDevice;
            setSliderAndLabels(uid.toStdString(),savedValue*10);
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

void TIGLViewerSelectWingAndFlapStatusDialog::setSliderAndLabels(std::string controlSurfaceDeviceUID, int k)
{
    QString textVal = "Value: ";
    QString textRot = "Rotation: ";
    QString textDef = "Deflection: ";

    std::vector<double> relDeflections;
    std::vector<double> rotations;
    for ( int steps = 0; steps < _controlSurfaceDevicesPointer[controlSurfaceDeviceUID]->getMovementPath().getSteps().getControlSurfaceDeviceStepCount(); steps++ )
    {
        relDeflections.push_back(_controlSurfaceDevicesPointer[controlSurfaceDeviceUID]->getMovementPath().getSteps().getControlSurfaceDeviceStepByID(steps+1).getRelDeflection());
        rotations.push_back(_controlSurfaceDevicesPointer[controlSurfaceDeviceUID]->getMovementPath().getSteps().getControlSurfaceDeviceStepByID(steps+1).getHingeLineRotation());
    }

    double value = k;
    double flapStatusInPercent = value/10;
    double inputDeflection = ( relDeflections[relDeflections.size()-1] - relDeflections[0] ) * ( flapStatusInPercent/100 ) + relDeflections[0];
    double rotation = linearInterpolation( relDeflections, rotations, inputDeflection );

    textVal.append(QString::number(value/10));
    textRot.append(QString::number(rotation));
    textDef.append(QString::number(inputDeflection));
    textVal.append("% ");
    _displayer.at(controlSurfaceDeviceUID)->setText(textVal);
    _displayer_deflection.at(controlSurfaceDeviceUID)->setText(textDef);
    _displayer_rotation.at(controlSurfaceDeviceUID)->setText(textRot);
    _controlSurfaceDevices[controlSurfaceDeviceUID] = value/10;
}
