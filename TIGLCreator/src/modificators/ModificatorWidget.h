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
 * A Modificator widget sets the interface for a particular cpacs element.
 * For example, the wing modificator widget will hold the high level interface to
 * modify the wing. Each modificator widget will hold a Tigl object as the
 * CCPACSWing, retrieve its information and display it. Each modificator
 * widget needs to implement an "apply" function that retrieves the values from
 * its GUI and sets it into the tigl object. Each modificator widget also needs to
 * implement a "reset" function that retrieves the data from its tigl
 * object and displays it in its GUI.
 * @remark The function to set the tigl object needs to be implemented in the
 * derived class.
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
    * Check if some modification was performed:
    * if yes -> apply it on the tigl object (need to write in tixi memory)
    * if no -> the value are reset from the tigl object
    * @return true if some modification was done -> need to write in tixi memory, otherwise return false
    */
    virtual bool apply();

protected:
    double precision;
    /**
   * Helper function to evaluate if a change occurs.
   * The values stored by the GUI may have some rounding errors that will create false
   * positives if this function is not used.
   * @param a
   * @param b
   * @return true if the abs(a-b) < precision
   */
    bool isApprox(double a, double b);
};

#endif // TIGL_MODIFICATORWIDGET_H
