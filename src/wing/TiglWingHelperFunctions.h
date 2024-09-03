/*
 * Copyright (C) 2019 CFS Engineering
 *
 * Created: 2019 Malo Drougard <malo.drougard@protonmail.com>
 * Author: Malo Drougard
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


#ifndef TIGLWINGHELPERFUNCTIONS_H
#define TIGLWINGHELPERFUNCTIONS_H

#include <CCPACSWing.h>

namespace tigl
{

namespace winghelper
{

/**
 * Returns the depth direction of the wing
 *
 * Note: this is a heuristic that is determined by the wing shape
 * and its position in 3D space.
 *
 * TODO: In future we need do define on the cpacs basis the projection
 * plane of the wing
 */
TIGL_EXPORT TiglAxis GetWingDepthAxis(const tigl::CCPACSWing& wing);

/**
 * Returns the major direction of the wing (correspond to the span direction)
 *
 * @Details: If a symmetry plan is set, the major direction is normal to the symmetry plan,
 * otherwise, an heuristic is used to find out the best span axis candidate.
 *
 * Note: this is a heuristic that is determined by the wing shape
 * and its position in 3D space.
 *
 * TODO: In future we need do define on the cpacs basis the projection
 * plane of the wing
 */
TIGL_EXPORT TiglAxis GetWingSpanAxis(const tigl::CCPACSWing& wing);

} // namespace winghelper

} // namespace tigl

#endif // TIGLWINGHELPERFUNCTIONS_H
