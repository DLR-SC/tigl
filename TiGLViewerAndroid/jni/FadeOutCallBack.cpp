/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-06-11 Mahmoud Aly <Mahmoud.Aly@dlr.de>
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
#include "FadeOutCallBack.h"

FadeOutCallback::FadeOutCallback( osg::Material* mat )
    : _material(mat)
{
    _intialTime = osg::Timer::instance()->tick();
    _material->setAlpha(osg::Material::FRONT_AND_BACK, 1.);
}

void FadeOutCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    osg::Timer_t currentTime = osg::Timer::instance()->tick();
    double ms_elapsed = osg::Timer::instance()->delta_m(_intialTime, currentTime);
    updateDiffuse(ms_elapsed);
    traverse( node, nv );
    return;
}
void FadeOutCallback::updateDiffuse(double time_elapsed)
{
    double delay_to_fadeout = 3000;
    double fadeout_time     = 1000;
    double alpha_level = 1.;

    if (time_elapsed < delay_to_fadeout) {
        alpha_level = 1.;
    }
    else if (time_elapsed > delay_to_fadeout && time_elapsed < delay_to_fadeout + fadeout_time) {
        double time_faded = time_elapsed - delay_to_fadeout;
        alpha_level = 1. - time_faded/fadeout_time;
    }
    else {
        alpha_level = 0.;
    }
    _material->setAlpha(osg::Material::FRONT_AND_BACK, alpha_level);
}
