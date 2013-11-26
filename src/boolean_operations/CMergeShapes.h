#ifndef CMERGESHAPES_H
#define CMERGESHAPES_H

#include "CNamedShape.h"

class CMergeShapes
{
public:
    // merges shapes with adjacent faces
    CMergeShapes(const CNamedShape& shape, const CNamedShape& tool);
    virtual ~CMergeShapes();

    operator CNamedShape ();

    void Perform();
    const CNamedShape& NamedShape();

protected:
    bool _hasPerformed;

    CNamedShape _resultshape, _tool, _source;
};

#endif // CMERGESHAPES_H
