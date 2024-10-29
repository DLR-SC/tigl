#include "test.h"

#include "tiglcommonfunctions.h"
#include "CTiglRectGridSurface.h"

TEST(CTiglRectGridSurface, testErrors)
{
    /*This is not a rectangular grid:
     *   __ __
     *  |__|__|__
     *     |__|__|
     *
     */
    TopoDS_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    builder.Add(compound, BuildFace(gp_Pnt(0.,  0., 0.), gp_Pnt(1.,  0., 0.), gp_Pnt(0., 1., 0.), gp_Pnt(1., 1., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(1.,  0., 0.), gp_Pnt(2.,  0., 0.), gp_Pnt(1., 1., 0.), gp_Pnt(2., 1., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(1., -1., 0.), gp_Pnt(2., -1., 0.), gp_Pnt(1., 0., 0.), gp_Pnt(2., 0., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(2., -1., 0.), gp_Pnt(3., -1., 0.), gp_Pnt(2., 0., 0.), gp_Pnt(3., 0., 0.)));

    EXPECT_THROW(tigl::CTiglRectGridSurface<> rectgridsurf(compound), tigl::CTiglError);

    /*This is not a rectangular grid:
     *   __
     *  |__|__
     *  |__|__|
     *     |__|
     *
     */
    TopoDS_Compound compound2;
    builder.MakeCompound(compound2);
    builder.Add(compound2, BuildFace(gp_Pnt(0.,  0., 0.), gp_Pnt(1.,  0., 0.), gp_Pnt(0., 1., 0.), gp_Pnt(1., 1., 0.)));
    builder.Add(compound2, BuildFace(gp_Pnt(0., -1., 0.), gp_Pnt(1., -1., 0.), gp_Pnt(0., 0., 0.), gp_Pnt(1., 0., 0.)));
    builder.Add(compound2, BuildFace(gp_Pnt(1., -1., 0.), gp_Pnt(2., -1., 0.), gp_Pnt(1., 0., 0.), gp_Pnt(2., 0., 0.)));
    builder.Add(compound2, BuildFace(gp_Pnt(1., -2., 0.), gp_Pnt(2., -2., 0.), gp_Pnt(1.,-1., 0.), gp_Pnt(2.,-1., 0.)));

    EXPECT_THROW(tigl::CTiglRectGridSurface<> rectgridsurf2(compound2), tigl::CTiglError);
}

TEST(CTiglRectGridSurface, connectivity)
{
    /* A 2x3 grid
     *  __ __ __
     * |__|__|__|
     * |__|__|__|
     */
    TopoDS_Builder builder;
    TopoDS_Compound compound;
    builder.MakeCompound(compound);
    builder.Add(compound, BuildFace(gp_Pnt(0.,  0., 0.), gp_Pnt(1.,  0., 0.), gp_Pnt(0., 1., 0.), gp_Pnt(1., 1., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(1.,  0., 0.), gp_Pnt(2.,  0., 0.), gp_Pnt(1., 1., 0.), gp_Pnt(2., 1., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(2.,  0., 0.), gp_Pnt(3.,  0., 0.), gp_Pnt(2., 1., 0.), gp_Pnt(3., 1., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(0., -1., 0.), gp_Pnt(1., -1., 0.), gp_Pnt(0., 0., 0.), gp_Pnt(1., 0., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(1., -1., 0.), gp_Pnt(2., -1., 0.), gp_Pnt(1., 0., 0.), gp_Pnt(2., 0., 0.)));
    builder.Add(compound, BuildFace(gp_Pnt(2., -1., 0.), gp_Pnt(3., -1., 0.), gp_Pnt(2., 0., 0.), gp_Pnt(3., 0., 0.)));

    // check number of rows and columns
    tigl::CTiglRectGridSurface<> rectgridsurf(compound);
    EXPECT_EQ(rectgridsurf.NRows(), 2);
    EXPECT_EQ(rectgridsurf.NCols(), 3);

    // follow some paths in the grid
    auto f = rectgridsurf.Root();
    EXPECT_EQ(f->UNext()->UNext()->UNext(), nullptr);
    EXPECT_EQ(f->VNext()->VNext(), nullptr);
    EXPECT_EQ(f->UNext()->VNext()->VNext(), nullptr);
    EXPECT_EQ(f->VNext()->UNext()->UNext()->UNext(), nullptr);
}
