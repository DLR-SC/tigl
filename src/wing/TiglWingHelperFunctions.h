#ifndef TIGLWINGHELPERFUNCTIONS_H
#define TIGLWINGHELPERFUNCTIONS_H

#include <CCPACSWing.h>

namespace tigl
{

namespace winghelper
{

 // Returns the deep direction of the wing
TiglAxis GetDeepDirection(const tigl::CCPACSWing& wing);

// Returns the major direction of the wing (correspond to the span direction)
// @Details: If a symmetry plan is set, the major direction is normal to the symmetry plan,
// otherwise, an heuristic is used to find out the best span axis candidate.
TiglAxis GetMajorDirection(const tigl::CCPACSWing& wing);

} // namespace winghelper

} // namespace tigl

#endif // TIGLWINGHELPERFUNCTIONS_H
