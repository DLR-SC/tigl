#ifndef CTIGLENGINEPYLONBUILDER_H
#define CTIGLENGINEPYLONBUILDER_H

#include <CCPACSEnginePylon.h>
#include <PNamedShape.h>
#include "tigl_internal.h"

namespace tigl
{

class CTiglEnginePylonBuilder
{
public:
    CTiglEnginePylonBuilder(const CCPACSEnginePylon& pylon, bool enableProfileCutting = false)
        : m_pylon(pylon), _enableProfileCutting(enableProfileCutting)
    {}

    TIGL_EXPORT operator PNamedShape();

    TIGL_EXPORT PNamedShape BuildShape();

private:
    const CCPACSEnginePylon& m_pylon;
    bool _enableProfileCutting;
};

} // namespace tigl

#endif // CTIGLENGINEPYLONBUILDER_H
