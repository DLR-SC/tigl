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
    CTiglEnginePylonBuilder(const CCPACSEnginePylon& pylon)
        : m_pylon(pylon)
    {}

    TIGL_EXPORT operator PNamedShape();

    TIGL_EXPORT PNamedShape BuildShape();

private:
    const CCPACSEnginePylon& m_pylon;
};

} // namespace tigl

#endif // CTIGLENGINEPYLONBUILDER_H
