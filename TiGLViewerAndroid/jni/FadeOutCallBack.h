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

#include <osg/Material>
#include <osg/Node>
#include <osg/Timer>


using namespace std;

class FadeOutCallback : public osg::NodeCallback
{
public:
    FadeOutCallback( osg::Material* mat );
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv );

protected:
    void updateDiffuse(double t);
    osg::Material* _material;
    osg::Timer_t _intialTime;
};

