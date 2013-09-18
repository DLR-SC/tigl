#ifndef CCPACSFARFIELD_H
#define CCPACSFARFIELD_H

#include "CTiglAbstractGeometricComponent.h"
#include "tixi.h"
#include "tigl_config.h"

namespace tigl {

enum TiglFarFieldType {
    NONE,
    HALF_SPHERE,
    FULL_SPHERE,
    HALF_CUBE,
    FULL_CUBE
};

class CCPACSFarField : public CTiglAbstractGeometricComponent
{
public:
    CCPACSFarField();
    ~CCPACSFarField();

    void ReadCPACS(TixiDocumentHandle tixiHandle);

    // Returns the Geometric type of this component, e.g. Wing or Fuselage
    virtual TiglGeometricComponentType GetComponentType(void);

    TiglFarFieldType GetFieldType();

#ifdef TIGL_USE_XCAF
    // builds data structure for a TDocStd_Application
    // mostly used for export
    TDF_Label ExportDataStructure(class CCPACSConfiguration&, Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label);
#endif

protected:
    virtual TopoDS_Shape BuildLoft(void);

private:
    void init();

    TiglFarFieldType fieldType;

    /// depending on fieldtype either cross section of sphere or sidelength of cube (reference length in cpacs)
    double fieldSize;
};

} // namespace tigl

#endif // CCPACSFARFIELD_H
