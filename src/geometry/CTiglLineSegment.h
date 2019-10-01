#ifndef CTIGLLINESEGMENT_H
#define CTIGLLINESEGMENT_H

#include "CTiglPoint.h"

namespace tigl
{

class CTiglLineSegment
{
public:
    CTiglLineSegment(const CTiglPoint& p1, const CTiglPoint& p2);
    
    CTiglPoint value(double u) const;
    
    double distance(const CTiglLineSegment& other) const;

private:
    CTiglPoint p;
    CTiglPoint d;
};

} // namespace tigl

#endif // CTIGLLINESEGMENT_H
