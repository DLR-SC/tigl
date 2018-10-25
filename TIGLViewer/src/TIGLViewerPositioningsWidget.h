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

#ifndef TIGL_TIGLVIEWERPOSITIONINGSWIDGET_H
#define TIGL_TIGLVIEWERPOSITIONINGSWIDGET_H

//#include <CPACSCreatorLib/CPACSTreeItem.h>
#include "ModificatorWidget.h"
//#include <CPACSCreatorLib/CPACSPositioning.h>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QScrollArea>
#include <QSpacerItem>

class TIGLViewerPositioningsWidget : public ModificatorWidget
{

    Q_OBJECT

public:
    TIGLViewerPositioningsWidget(QWidget* parent = 0);
    /*
    void init(ModificatorManager * associate ) override ;
    void apply() override ;

    void setPositionings(cpcr::CPACSTreeItem* positionings);

protected:
    void clean();


private:

    // Here we stock the values of the cpacs files, we use them to detect changes
    std::list<std::pair<cpcr::CPACSTreeItem*, cpcr::CPACSPositioning >> internals;

    // These vectors contains the spinBoxes with the values that are displayed and can be modify
    std::vector<QDoubleSpinBox*> sweeps;
    std::vector<QDoubleSpinBox*> dihedrals;
    std::vector<QDoubleSpinBox*> lengths;
    std::vector<QLabel*> labelsIndex;

    // to manage the display we use a scroll area where the content is retrieve in the init function
    QWidget * content;
    std::vector<QLabel*> header;
    QHBoxLayout* headerLayout;
    QSpacerItem* spacer ; // to have header attached to layoutPerPositionings
    QVBoxLayout *globalLayout ;  // where the layout per positioning will be set
    std::vector<QHBoxLayout*> layoutPerPositioning;   // We create in  runtime one layout per positioning


    int precision; // set the precision of the qdoublespinbox and is use to detect false change
*/
};

#endif //TIGL_TIGLVIEWERPOSITIONINGSWIDGET_H
