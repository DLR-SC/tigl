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

//
// Created by cfse on 7/9/18.
//

#include "TIGLViewerPositioningsWidget.h"
//#include "ModificatorManager.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

TIGLViewerPositioningsWidget::TIGLViewerPositioningsWidget(QWidget* parent)
    : ModificatorWidget(parent)
{
}
/*
void TIGLViewerPositioningsWidget::init(ModificatorManager *associate) {
    ModificatorWidget::init(associate);


    precision = 4; // 4 decimal after the ,

    // find the content of the scroll area
    content = this->findChild<QWidget*>("positioningsModificatorContent");
    globalLayout = new QVBoxLayout; // were the layout per positioning will be set

    // create the header and the spacer
    header.push_back(new QLabel("Tixi Idx"));
    header.push_back(new QLabel("Sweep"));
    header.push_back(new QLabel("Dihedral"));
    header.push_back(new QLabel("Length"));
    headerLayout = new QHBoxLayout();
    for( QLabel* h : header){
        headerLayout->addWidget(h);
    }
    spacer = new QSpacerItem(100,200,QSizePolicy::Expanding,QSizePolicy::Expanding);
    globalLayout->addSpacerItem(spacer);
    globalLayout->addLayout(headerLayout);

    this->content->setLayout(globalLayout);




}

void TIGLViewerPositioningsWidget::apply() {

    int numberOfChange = 0;
    int i = 0;
    bool hasChanged;
    for (std::pair<cpcr::CPACSTreeItem *, cpcr::CPACSPositioning> p: internals) {

        hasChanged = ( ! isApprox( p.second.getLength(), lengths[i]->value() ) )
                        || ( ! isApprox(p.second.getDihedralAngle(), dihedrals[i]->value() ))
                        || ( ! isApprox(p.second.getSweepAngle(), sweeps[i]->value() ));

        if(hasChanged){
            p.second.setDihedralAngle(dihedrals[i]->value() ); // set internals from boxes
            p.second.setSweepAngle(sweeps[i]->value() ); // set internals from boxes
            p.second.setLength(lengths[i]->value() ); // set internals from boxes
            associateManager->adapter->setPositioning(p.first, p.second ); // set in CPACS
            numberOfChange ++;
        }

        i++; // we parse in the same order than we have created
    }

    if(numberOfChange > 0){
        associateManager->adapter->writeToFile();
    }

}



void TIGLViewerPositioningsWidget::setPositionings(cpcr::CPACSTreeItem *item) {

    clean();

    // get all the positionings
    this->internals = associateManager->adapter->getPositionings(item);

    // creation of the spin boxes
    int count = 1;
    for(std::pair<cpcr::CPACSTreeItem*,cpcr::CPACSPositioning> p: internals){

        QDoubleSpinBox *tempSweep = new QDoubleSpinBox;
        tempSweep->setRange(-360, 360);
        tempSweep->setDecimals(precision);
        tempSweep->setValue(p.second.getSweepAngle());
        sweeps.push_back(tempSweep);

        QDoubleSpinBox *tempDihedral = new QDoubleSpinBox;
        tempDihedral->setRange(-360, 360);
        tempDihedral->setDecimals(precision);
        tempDihedral->setValue(p.second.getDihedralAngle());
        dihedrals.push_back(tempDihedral);

        QDoubleSpinBox *tempLength = new QDoubleSpinBox;
        tempLength->setRange(-1000, 1000);
        tempLength->setDecimals(precision);
        tempLength->setValue(p.second.getLength());
        lengths.push_back(tempLength);

        QLabel *tempLabel = new QLabel;
        tempLabel->setText( std::to_string(p.first->getTixiIndex()).c_str()  );
        labelsIndex.push_back(tempLabel);
        count++;
    }

    // creation of the layouts
    for(int i = 0; i < sweeps.size(); i++){
        QHBoxLayout* tempLayout = new QHBoxLayout();
        tempLayout->addWidget(labelsIndex[i]);
        tempLayout->addWidget(sweeps[i]);
        tempLayout->addWidget(dihedrals[i]);
        tempLayout->addWidget(lengths[i]);
        layoutPerPositioning.push_back(tempLayout);
        globalLayout->addLayout(tempLayout);
    }

}


void TIGLViewerPositioningsWidget::clean() {

    for(int i = 0; i < layoutPerPositioning.size(); i++){


        layoutPerPositioning[i]->removeWidget(sweeps[i]);
        delete sweeps[i];

        layoutPerPositioning[i]->removeWidget(dihedrals[i]);
        delete  dihedrals[i];

        layoutPerPositioning[i]->removeWidget(lengths[i]);
        delete lengths[i];

        layoutPerPositioning[i]->removeWidget(labelsIndex[i]);
        delete labelsIndex[i];

        delete  layoutPerPositioning[i];
    }

    layoutPerPositioning.clear();

    sweeps.clear();
    dihedrals.clear();
    lengths.clear();
    labelsIndex.clear();

    internals.clear();
}



*/
