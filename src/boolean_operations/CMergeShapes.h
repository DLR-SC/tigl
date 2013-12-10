#ifndef CMERGESHAPES_H
#define CMERGESHAPES_H

#include "CNamedShape.h"

class CMergeShapes
{
public:
    // merges shapes with adjacent faces
    CMergeShapes(const PNamedShape shape, const PNamedShape tool);
    virtual ~CMergeShapes();

    operator PNamedShape ();

    void Perform();
    const PNamedShape NamedShape();

protected:
    bool _hasPerformed;

    PNamedShape _resultshape, _tool, _source;
};

#endif // CMERGESHAPES_H
