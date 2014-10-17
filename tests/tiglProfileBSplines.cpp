#include "test.h"
#include "tigl.h"


class BSplineSimpleGeom : public ::testing::Test 
{
protected:
    static void SetUpTestCase() 
    {
        const char* filename = "TestData/simpletest.cpacs.xml";
        ReturnCode tixiRet;
        TiglReturnCode tiglRet;

        tiglSimpleHandle = -1;
        tixiSimpleHandle = -1;

        tixiRet = tixiOpenDocument(filename, &tixiSimpleHandle);
        ASSERT_TRUE (tixiRet == SUCCESS);

        tiglRet = tiglOpenCPACSConfiguration(tixiSimpleHandle, "Cpacs2Test", &tiglSimpleHandle);
        ASSERT_TRUE(tiglRet == TIGL_SUCCESS);
    }

    static void TearDownTestCase() 
    {
        ASSERT_TRUE(tiglCloseCPACSConfiguration(tiglSimpleHandle) == TIGL_SUCCESS);
        ASSERT_TRUE(tixiCloseDocument(tixiSimpleHandle) == SUCCESS);
        tiglSimpleHandle = -1;
        tixiSimpleHandle = -1;
    }

    virtual void SetUp() {}
    virtual void TearDown() {}


    static TixiDocumentHandle           tixiSimpleHandle;
    static TiglCPACSConfigurationHandle tiglSimpleHandle;
};

TixiDocumentHandle BSplineSimpleGeom::tixiSimpleHandle = 0;
TiglCPACSConfigurationHandle BSplineSimpleGeom::tiglSimpleHandle = 0;

TEST_F(BSplineSimpleGeom, curveCount)
{
    int count;
    ASSERT_EQ(TIGL_SUCCESS, tiglProfileGetBSplineCount(tiglSimpleHandle, "NACA0012", &count));
    ASSERT_EQ(2, count);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglProfileGetBSplineCount(tiglSimpleHandle, "fuselageCircleProfileuID", &count));
    ASSERT_EQ(1, count);
}

TEST_F(BSplineSimpleGeom, curveCount_Errors)
{
    int count;
    ASSERT_EQ(TIGL_NOT_FOUND, tiglProfileGetBSplineCount(-1, "NACA0012", &count));
    ASSERT_EQ(TIGL_UID_ERROR, tiglProfileGetBSplineCount(tiglSimpleHandle, "invaliduid", &count));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineCount(tiglSimpleHandle, NULL, &count));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineCount(tiglSimpleHandle,"NACA0012", NULL));
}

TEST_F(BSplineSimpleGeom, splineSizes)
{
    int n, ncp, nk;
    ASSERT_EQ(TIGL_SUCCESS, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 1, &n, &ncp, &nk));
    ASSERT_EQ(3, n);
    ASSERT_EQ(nk, n + ncp + 1);
    
    ASSERT_EQ(TIGL_SUCCESS, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "fuselageCircleProfileuID", 1, &n, &ncp, &nk));
    ASSERT_EQ(3, n);
    ASSERT_EQ(nk, n + ncp + 1);

}

TEST_F(BSplineSimpleGeom, splineSizes_Errors)
{
    int n, ncp, nk;
    ASSERT_EQ(TIGL_NOT_FOUND, tiglProfileGetBSplineDataSizes(-1, "NACA0012", 1, &n, &ncp, &nk));
    ASSERT_EQ(TIGL_UID_ERROR, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "invaliduid", 1, &n, &ncp, &nk));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, NULL, 1, &n, &ncp, &nk));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 1, NULL, &ncp, &nk));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 1, &n, NULL, &nk));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 1, &n, &ncp, NULL));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 0, &n, &ncp, &nk));
    ASSERT_EQ(TIGL_INDEX_ERROR, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 3, &n, &ncp, &nk));
}

TEST_F(BSplineSimpleGeom, splineData)
{
    int n, ncp, nk;
    ASSERT_EQ(TIGL_SUCCESS, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 1, &n, &ncp, &nk));

    double* cpx = new double[ncp];
    double* cpy = new double[ncp];
    double* cpz = new double[ncp];
    double* knots = new double[nk];

    ASSERT_EQ(TIGL_SUCCESS, tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 1, ncp, cpx, cpy, cpz, nk, knots));
    ASSERT_LT(1, nk);
    
    double lastKnot = knots[0];
    for (int ik = 0; ik < nk; ++ik) {
        ASSERT_LE(lastKnot, knots[ik]);
        lastKnot = knots[ik];
    }
    
    delete[] cpx;
    delete[] cpy;
    delete[] cpz;
    delete[] knots;
}

TEST_F(BSplineSimpleGeom, splineData_Errors)
{
    int n, ncp, nk;
    ASSERT_EQ(TIGL_SUCCESS, tiglProfileGetBSplineDataSizes(tiglSimpleHandle, "NACA0012", 1, &n, &ncp, &nk));

    double* cpx = new double[ncp];
    double* cpy = new double[ncp];
    double* cpz = new double[ncp];
    double* knots = new double[nk];

    ASSERT_EQ(TIGL_NOT_FOUND,    tiglProfileGetBSplineData(-1, "NACA0012", 1, ncp, cpx, cpy, cpz, nk, knots));
    ASSERT_EQ(TIGL_UID_ERROR,    tiglProfileGetBSplineData(tiglSimpleHandle, "invaliduid", 1, ncp, cpx, cpy, cpz, nk, knots));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 1, ncp, NULL, cpy, cpz, nk, knots));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 1, ncp, cpx, NULL, cpz, nk, knots));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 1, ncp, cpx, cpy, NULL, nk, knots));
    ASSERT_EQ(TIGL_NULL_POINTER, tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 1, ncp, cpx, cpy, cpz, nk, NULL));
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 0, ncp, cpx, cpy, cpz, nk, knots));
    ASSERT_EQ(TIGL_INDEX_ERROR,  tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 3, ncp, cpx, cpy, cpz, nk, knots));
    ASSERT_EQ(TIGL_ERROR, tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 1, 1, cpx, cpy, cpz, nk, knots));
    ASSERT_EQ(TIGL_ERROR, tiglProfileGetBSplineData(tiglSimpleHandle, "NACA0012", 1, ncp, cpx, cpy, cpz, 1, knots));
    
    delete[] cpx;
    delete[] cpy;
    delete[] cpz;
    delete[] knots;
}
