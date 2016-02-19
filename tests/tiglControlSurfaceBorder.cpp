
#include "test.h"

#include "CControlSurfaceBoarderBuilder.h"

#include <TopoDS_Shape.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>

class TiglControlSurfaceBorder : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        BRep_Builder b;
        BRepTools::Read(wingShape, "TestData/simplewing.brep", b);
        ASSERT_FALSE(wingShape.IsNull());
    }

    virtual void TearDown()
    {
    }

protected:
    TopoDS_Shape wingShape;
};

TEST_F(TiglControlSurfaceBorder, boarderBuilder)
{
    double lex = 0.7;
    
    tigl::CSCoordSystem coordsystem(gp_Pnt(lex, 0.5, 0), gp_Pnt(1, 0.5, 0), gp_Vec(0, 0, 1.));
    tigl::CControlSurfaceBoarderBuilder builder(coordsystem, wingShape);
   
    builder.boarderSimple();

    double upperXsi = 0.5;
    double lowerXsi = 0.5;
    builder.boarderWithLEShape(0.6, upperXsi, lowerXsi);
    
    ASSERT_NEAR((1.-lex)*upperXsi, builder.upperPoint().X(), 1e-8);
    ASSERT_TRUE(builder.upperPoint().Y() > 0.);
    
    ASSERT_NEAR((1.-lex)*lowerXsi, builder.lowerPoint().X(), 1e-8);
    ASSERT_TRUE(builder.lowerPoint().Y() < 0.);
}
