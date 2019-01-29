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

#ifndef TIGL_MODIFICATORWIDGET_H
#define TIGL_MODIFICATORWIDGET_H

#include <QWidget>

/**
 * Abstract class for modificator widget.
 * A Modificator widget set the interface for a particular cpacs element.
 * For example the wing modificator widget will hold the high level interface to
 * modify the wing. Each modificator widgets will hold a Tigl object as the
 * CCPACSWing, it retrieve its information and display it. Each modificator
 * widgets need to implement an "apply" function that will that the values from
 * its GUI and set it into the tigl object. Each modificator widgets need to
 * implement an "reset" function that will retrieve the data from its tigl
 * object and display it in its GUI.
 * @remark The function to set the tigl object need to be implemented in the
 * derivative class.
 *
 */
class ModificatorWidget : public QWidget
{

    Q_OBJECT

public:
    ModificatorWidget(QWidget* parent = 0);

    /**
   * Reset the display value from the value of tigl object (called when cancel
   * button is pressed)
   */
    virtual void reset();

    /**
   *  Should check if modification was performed , if yes -> apply it on the
   * tigl object. (called when commit button is pressed)
   */
    virtual void apply();

protected:
    double precision;
    /**
   * Helper function to evaluate if a change occurs.
   * The values stored by the GUI can have some rounding that will create false
   * positif if this function is not used.
   * @param a
   * @param b
   * @return true if the abs(a-b) < precision
   */
    bool isApprox(double a, double b);
};

#endif // TIGL_MODIFICATORWIDGET_H
