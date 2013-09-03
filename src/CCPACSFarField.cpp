#include "CCPACSFarField.h"
#include "CTiglError.h"
#include "CTiglLogger.h"

#include <string>
#include <cmath>

#include <gp_Ax2.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <TopExp_Explorer.hxx>

#ifdef TIGL_USE_XCAF
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <TDataStd_Name.hxx>
#include <TDataXtd_Shape.hxx>
#endif

namespace tigl {

CCPACSFarField::CCPACSFarField() {
    init();
}

CCPACSFarField::~CCPACSFarField() {
}

void CCPACSFarField::init() {
    fieldType = NONE;
    fieldSize = 0.;
    loft.Nullify();
}

TiglFarFieldType CCPACSFarField::GetFieldType(){
    return fieldType;
}

void CCPACSFarField::ReadCPACS(TixiDocumentHandle tixiHandle) {
    init();

    std::string prefix = "/cpacs/toolspecific/cFD/farField";
    if(tixiCheckElement(tixiHandle, prefix.c_str()) != SUCCESS) {
        LOG(INFO) << "No far-field defined.";
        fieldType = NONE;
        return;
    }

    // get field type
    std::string typePath = prefix + "/type";
    char * tmpstr = NULL;
    if (tixiGetTextElement(tixiHandle, typePath.c_str(), &tmpstr) != SUCCESS) {
        fieldType = NONE;
        return;
    }
    else {
        if (strcmp(tmpstr, "halfSphere") == 0){
            fieldType = HALF_SPHERE;
        }
        else if (strcmp(tmpstr, "fullSphere") == 0){
            fieldType = FULL_SPHERE;
        }
        else if (strcmp(tmpstr, "halfCube") == 0){
            fieldType = HALF_CUBE;
        }
        else if (strcmp(tmpstr, "fullCube") == 0){
            fieldType = FULL_CUBE;
        }
        else {
            fieldType = NONE;
            return;
        }
    }

    // get reference length
    std::string refLenPath = prefix + "/referenceLength";
    if (tixiGetDoubleElement(tixiHandle, refLenPath.c_str(), &fieldSize) != SUCCESS) {
        fieldSize = 0.;
        throw tigl::CTiglError("No reference length defined for far-field!");
    }

    // get multiplier
    std::string multiplierPath = prefix + "/multiplier";
    double multiplier = 1.;
    if (tixiGetDoubleElement(tixiHandle, multiplierPath.c_str(), &multiplier) != SUCCESS) {
        fieldSize = 0.;
        throw tigl::CTiglError("No multiplier defined for far-field!");
    }
    else {
        fieldSize *= multiplier;
    }
}

TopoDS_Shape CCPACSFarField::BuildLoft(void){
    TopoDS_Shape shape;
    shape.Nullify();
    gp_Pnt center(0,0,0);
    gp_Ax2 axis(center, gp_Dir(0,0,1));

    switch(fieldType){
    case NONE:
        return shape;
    case FULL_SPHERE:
        shape = BRepPrimAPI_MakeSphere(center, fieldSize).Shape();
        break;
    case FULL_CUBE:
        shape = BRepPrimAPI_MakeBox(gp_Pnt(center.X()-fieldSize, center.Y()-fieldSize, center.Z()-fieldSize),
                                    fieldSize*2., fieldSize*2., fieldSize*2.).Shape();
        break;
    case HALF_CUBE:
        shape = BRepPrimAPI_MakeBox(gp_Pnt(center.X()-fieldSize, center.Y(), center.Z()-fieldSize),
                                    fieldSize*2., fieldSize, fieldSize*2.).Shape();
        break;
    case HALF_SPHERE:
        shape = BRepPrimAPI_MakeSphere(axis, fieldSize, M_PI).Shape();
        break;
    default:
        shape.Nullify();
    }

    return shape;
}

TiglGeometricComponentType CCPACSFarField::GetComponentType(void) {
    return TIGL_COMPONENT_LOGICAL;
}

#ifdef TIGL_USE_XCAF
// builds data structure for a TDocStd_Application
// mostly used for export
TDF_Label CCPACSFarField::ExportDataStructure(Handle_XCAFDoc_ShapeTool &myAssembly, TDF_Label& label)
{
    TopExp_Explorer faceExplorer;

    Handle_XCAFDoc_ShapeTool newAssembly = XCAFDoc_DocumentTool::ShapeTool (label);
    TDF_Label subLabel = myAssembly->NewShape();

    int i = 0;
    for (faceExplorer.Init(GetLoft(), TopAbs_FACE); faceExplorer.More(); faceExplorer.Next()) {
        std::string numName = "Farfield_Face_" + i++;
        subLabel = newAssembly->AddSubShape (label, faceExplorer.Current());
        TDataStd_Name::Set(label, numName.c_str());
    }

    return subLabel;
}
#endif

} // namespace tigl

