/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-02-05 Martin Siggel <martin.siggel@dlr.de>
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

#ifndef TIGL_ITIGLVIEWERSETTINGSCHANGEDLISTENER_H
#define TIGL_ITIGLVIEWERSETTINGSCHANGEDLISTENER_H


class ITIGLViewerSettingsChangedListener
{
public:
    // has default implementation so it is not mandatory to override
    virtual void backgroundColorHasChanged()
    {

    }

    // has default implementation so it is not mandatory to override
    virtual void defaultShapeColorHasChanged()
    {

    }

    // has default implementation so it is not mandatory to override
    virtual void defaultShapeSymmetryColorHasChanged()
    {

    }

    // has default implementation so it is not mandatory to override
    virtual void defaultMaterialHasChanged()
    {

    }

};
#endif //TIGL_ITIGLVIEWERSETTINGSCHANGEDLISTENER_H
