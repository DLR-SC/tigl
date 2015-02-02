/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* This file was created by OpenCASCADE for the German Aerospace Center.
* It will reside in TiGL, until this Algorithm becomes part of
* OpenCASCADE.
*
* For now, the file will be licensed under the Apache License, Version 2.0.
*    http://www.apache.org/licenses/LICENSE-2.0
*/

#include <SurfTools.hxx>

#include <TColStd_Array1OfInteger.hxx>
#include <gp_Pnt.hxx>
#include <BSplCLib.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <GeomConvert_BSplineSurfaceToBezierSurface.hxx>
#include <TColGeom_Array2OfBezierSurface.hxx>
#include <GeomConvert_CompBezierSurfacesToBSplineSurface.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <TColStd_SequenceOfInteger.hxx>
#include <TColStd_SequenceOfReal.hxx>
#include <Geom_BSplineSurface.hxx>
#include <TColGeom_SequenceOfSurface.hxx>
//
static Standard_Integer UContinuity(const Handle(Geom_BSplineSurface)& theS)
{

    Standard_Integer aNbUKnots = theS->NbUKnots();

    if (aNbUKnots == 2) {
        return 2;
    }

    TColStd_Array1OfReal anUKnots(1, aNbUKnots);
    TColStd_Array1OfInteger anUMults(1, aNbUKnots);

    theS->UKnots(anUKnots);
    theS->UMultiplicities(anUMults);

    Standard_Integer Index = 2;
    Standard_Integer Upper = aNbUKnots - 1;
    Standard_Integer MaxMultiplicity = anUMults.Value (Index);
    while (Index <= Upper) {
        if (anUMults.Value (Index) > MaxMultiplicity) {
            MaxMultiplicity = anUMults.Value (Index);
        }
        Index++;
    }
    return theS->UDegree() - MaxMultiplicity;

}

static Standard_Integer VContinuity(const Handle(Geom_BSplineSurface)& theS)
{

    Standard_Integer aNbVKnots = theS->NbVKnots();

    if (aNbVKnots == 2) {
        return 2;
    }

    TColStd_Array1OfReal aVKnots(1, aNbVKnots);
    TColStd_Array1OfInteger aVMults(1, aNbVKnots);

    theS->VKnots(aVKnots);
    theS->VMultiplicities(aVMults);

    Standard_Integer Index = 2;
    Standard_Integer Upper = aNbVKnots - 1;
    Standard_Integer MaxMultiplicity = aVMults.Value (Index);
    while (Index <= Upper) {
        if (aVMults.Value (Index) > MaxMultiplicity) {
            MaxMultiplicity = aVMults.Value (Index);
        }
        Index++;
    }
    return theS->VDegree() - MaxMultiplicity;

}
//===================================================================
// SurfTools::ConcatenateSurfacesU
//===================================================================
Standard_Integer SurfTools::ConcatenateSurfacesU(const Handle(Geom_BSplineSurface)& theS1,
                                                 const Handle(Geom_BSplineSurface)& theS2,
                                                 const Standard_Real theTol,
                                                 Handle(Geom_BSplineSurface)& theS)
{
    Standard_Integer nbu1, nbv1, nbu2, nbv2, nbu, nbv, i, j;
    Standard_Real u1f, u1l, v1f, v1l, u2f, u2l, v2f, v2l;
    Standard_Real tol, aD1;//, aD2;
    gp_Pnt aP1[4], aP2[4];
    Handle(Geom_BSplineSurface) aS1R, aS2R;
    //
    theS1->Bounds(u1f, u1l, v1f, v1l);
    theS2->Bounds(u2f, u2l, v2f, v2l);
    //
    aP1[0]=theS1->Value(u1f, v1f);
    aP1[1]=theS1->Value(u1l, v1f);
    aP1[2]=theS1->Value(u1l, v1l);
    aP1[3]=theS1->Value(u1f, v1l);
    //
    aP2[0]=theS2->Value(u2f, v2f);
    aP2[1]=theS2->Value(u2l, v2f);
    aP2[2]=theS2->Value(u2l, v2l);
    aP2[3]=theS2->Value(u2f, v2l);
    //
    tol =theTol*theTol;
    //
    aS1R = Handle(Geom_BSplineSurface)::DownCast(theS1->Copy());
    aS2R = Handle(Geom_BSplineSurface)::DownCast(theS2->Copy());
    //

    Standard_Integer n = 0;
    //At least two boundary points of surfaces must coincide
    for (i=0; i<4; ++i) {
        for (j=0; j<4; ++j) {
            aD1=aP1[i].SquareDistance(aP2[j]);
            if (aD1<=tol) {
                ++n;
            }
        }
    }

    if (n < 2) {
        return 1;
    }

    // is aS1R to V reverse
    for (i=0; i<4; ++i) {
        aD1=aP1[0].SquareDistance(aP2[i]);
        if (aD1<=tol) {
            aS1R->VReverse();
            aS1R->Bounds(u1f, u1l, v1f, v1l);
            aP1[0]=aS1R->Value(u1f, v1f);
            aP1[1]=aS1R->Value(u1l, v1f);
            aP1[2]=aS1R->Value(u1l, v1l);
            aP1[3]=aS1R->Value(u1f, v1l);
            break;
        }
    }
    //
    // is aS2R to V reverse
    for (i=0; i<4; ++i) {
        aD1=aP2[3].SquareDistance(aP1[i]);
        if (aD1<=tol) {
            aS2R->VReverse();
            aS2R->Bounds(u2f, u2l, v2f, v2l);
            aP2[0]=aS2R->Value(u2f, v2f);
            aP2[1]=aS2R->Value(u2l, v2f);
            aP2[2]=aS2R->Value(u2l, v2l);
            aP2[3]=aS2R->Value(u2f, v2l);
            break;
        }
    }
    //
    //is aS2R to U reverse
    aD1=aP1[3].SquareDistance(aP2[1]);
    if (aD1<=tol) {
        aS2R->UReverse();
    }
    //
    Standard_Integer anUCont = Min(UContinuity(aS1R), UContinuity(aS2R));
    Standard_Integer udeg1, udeg2, uknts1, uknts2;
    Standard_Real ptol = 1.e-7;
    //
    udeg1 = aS1R->UDegree();
    udeg2 = aS2R->UDegree();
    udeg1 = Max(udeg1, udeg2);
    //
    aS1R->IncreaseDegree(udeg1, aS1R->VDegree());
    aS2R->IncreaseDegree(udeg1, aS2R->VDegree());
    //
    uknts1 = aS1R->NbUKnots();
    uknts2 = aS2R->NbUKnots();
    //
    if (uknts1 != uknts2) {
        if (Abs(u1f-u2f) > ptol || Abs(u1l-u2l) > ptol) {
            SurfTools::UReparametrize(aS2R, u1f, u1l);
        }

        TColStd_Array1OfReal Knots2(1, uknts2);
        aS2R->UKnots(Knots2);
        TColStd_Array1OfInteger Mults2(1, uknts2);
        aS2R->UMultiplicities(Mults2);

        aS1R->InsertUKnots(Knots2, Mults2, ptol, Standard_False);

        TColStd_Array1OfReal Knots(1, aS1R->NbUKnots());
        aS1R->UKnots(Knots);
        TColStd_Array1OfInteger Mults(1, aS1R->NbUKnots());
        aS1R->UMultiplicities(Mults);

        aS2R->InsertUKnots(Knots, Mults, ptol, Standard_False);
    }
    else {
        TColStd_Array1OfReal anUKnots(1, uknts1);
        for (i = 1; i <= uknts1; ++i) {
            anUKnots(i) = (aS1R->UKnot(i) + aS2R->UKnot(i))/2.;
        }
        aS1R->SetUKnots(anUKnots);
        aS2R->SetUKnots(anUKnots);
    }
    //
    GeomConvert_BSplineSurfaceToBezierSurface aDisass1(aS1R);
    nbu1 = aDisass1.NbUPatches();
    nbv1 = aDisass1.NbVPatches();
    TColGeom_Array2OfBezierSurface aBzrArr1(1, nbu1, 1, nbv1);
    aDisass1.Patches(aBzrArr1);

    GeomConvert_BSplineSurfaceToBezierSurface aDisass2(aS2R);
    nbu2 = aDisass2.NbUPatches();
    nbv2 = aDisass2.NbVPatches();
    //
    if (nbu1 != nbu2) {
        return 1;
    }
    //
    TColGeom_Array2OfBezierSurface aBzrArr2(1, nbu2, 1, nbv2);
    aDisass2.Patches(aBzrArr2);
    //
    nbv = nbv1+nbv2;
    nbu = nbu1;

    TColGeom_Array2OfBezierSurface aBzrArr(1, nbu, 1, nbv);
    //
    for (i = 1; i <= nbu; i++) {
        for (j = 1; j <= nbv1; j++) {
            aBzrArr(i, j) = aBzrArr1(i, j);
        }
    }

    for (i = 1; i <= nbu; i++) {
        for (j = 1; j <= nbv2; j++) {
            aBzrArr(i, j+nbv1) = aBzrArr2(i, j);
        }
    }
    //
    GeomConvert_CompBezierSurfacesToBSplineSurface anAss(aBzrArr, theTol);
    //
    if (!anAss.IsDone()) {
        return 1;
    }
    //
    theS = new Geom_BSplineSurface(anAss.Poles()->Array2(),
                                   anAss.UKnots()->Array1(),
                                   anAss.VKnots()->Array1(),
                                   anAss.UMultiplicities()->Array1(),
                                   anAss.VMultiplicities()->Array1(),
                                   anAss.UDegree(),
                                   anAss.VDegree());

    Standard_Real vf, vl, vbnd;
    //
    vf = theS->VKnot(1);
    vbnd = theS->VKnot(aS1R->NbVKnots());
    vl = theS->VKnot(theS->NbVKnots());

    v1l = v1f + (v1l-v1f)/(vbnd-vf)*(vl-vf);

    SurfTools::UReparametrize(theS, u1f, u1l);
    SurfTools::VReparametrize(theS, v1f, v1l);
    //
    if (UContinuity(theS) < anUCont) {
        USmoothing(theS, anUCont, theTol);
    }


    return 0;

}

//===================================================================
// SurfTools::ConcatenateSurfacesV
//===================================================================
Standard_Integer SurfTools::ConcatenateSurfacesV(const Handle(Geom_BSplineSurface)& theS1,
                                                 const Handle(Geom_BSplineSurface)& theS2,
                                                 const Standard_Real theTol,
                                                 Handle(Geom_BSplineSurface)& theS)
{
    Standard_Integer nbu1, nbv1, nbu2, nbv2, nbu, nbv, i, j;
    Standard_Real u1f, u1l, v1f, v1l, u2f, u2l, v2f, v2l;
    Standard_Real tol, aD1;
    gp_Pnt aP1[4], aP2[4];
    Handle(Geom_BSplineSurface) aS1R, aS2R;
    //
    theS1->Bounds(u1f, u1l, v1f, v1l);
    theS2->Bounds(u2f, u2l, v2f, v2l);
    //
    theS1->Bounds(u1f, u1l, v1f, v1l);
    theS2->Bounds(u2f, u2l, v2f, v2l);
    //
    aP1[0]=theS1->Value(u1f, v1f);
    aP1[1]=theS1->Value(u1l, v1f);
    aP1[2]=theS1->Value(u1l, v1l);
    aP1[3]=theS1->Value(u1f, v1l);
    //
    aP2[0]=theS2->Value(u2f, v2f);
    aP2[1]=theS2->Value(u2l, v2f);
    aP2[2]=theS2->Value(u2l, v2l);
    aP2[3]=theS2->Value(u2f, v2l);
    //
    tol =theTol*theTol;
    //
    aS1R = Handle(Geom_BSplineSurface)::DownCast(theS1->Copy());
    aS2R = Handle(Geom_BSplineSurface)::DownCast(theS2->Copy());
    //

    Standard_Integer n = 0;
    //At least two boundary points of surfaces must coincide
    for (i=0; i<4; ++i) {
        for (j=0; j<4; ++j) {
            aD1=aP1[i].SquareDistance(aP2[j]);
            if (aD1<=tol) {
                ++n;
            }
        }
    }

    if (n < 2) {
        return 1;
    }

    // is aS1R to U reverse
    for (i=0; i<4; ++i) {
        aD1=aP1[0].SquareDistance(aP2[i]);
        if (aD1<=tol) {
            aS1R->UReverse();
            aS1R->Bounds(u1f, u1l, v1f, v1l);
            aP1[0]=aS1R->Value(u1f, v1f);
            aP1[1]=aS1R->Value(u1l, v1f);
            aP1[2]=aS1R->Value(u1l, v1l);
            aP1[3]=aS1R->Value(u1f, v1l);
            break;
        }
    }
    //
    // is aS2R to U reverse
    for (i=0; i<4; ++i) {
        aD1=aP2[2].SquareDistance(aP1[i]);
        if (aD1<=tol) {
            aS2R->UReverse();
            aS2R->Bounds(u2f, u2l, v2f, v2l);
            aP2[0]=aS2R->Value(u2f, v2f);
            aP2[1]=aS2R->Value(u2l, v2f);
            aP2[2]=aS2R->Value(u2l, v2l);
            aP2[3]=aS2R->Value(u2f, v2l);
            break;
        }
    }
    //
    //is aS2R to V reverse
    aD1=aP1[2].SquareDistance(aP2[0]);
    if (aD1<=tol) {
        aS2R->VReverse();
    }
    //
    Standard_Integer anVCont = Min(VContinuity(aS1R), VContinuity(aS2R));
    Standard_Integer vdeg1, vdeg2, vknts1, vknts2 ;
    Standard_Real ptol = 1.e-7;
    //
    vdeg1 = aS1R->VDegree();
    vdeg2 = aS2R->VDegree();
    vdeg1 = Max(vdeg1, vdeg2);
    //
    aS1R->IncreaseDegree(aS1R->UDegree(), vdeg1);
    aS2R->IncreaseDegree(aS2R->UDegree(), vdeg1);
    //
    vknts1 = aS1R->NbVKnots();
    vknts2 = aS2R->NbVKnots();
    if (vknts1 != vknts2) {
        if (Abs(v1f-v2f) > ptol || Abs(v1l-v2l) > ptol) {
            SurfTools::VReparametrize(aS2R, v1f, v1l);
        }
        //
        TColStd_Array1OfReal Knots2(1, vknts2);
        aS2R->VKnots(Knots2);
        TColStd_Array1OfInteger Mults2(1, vknts2);
        aS2R->VMultiplicities(Mults2);

        aS1R->InsertVKnots(Knots2, Mults2, ptol, Standard_False);

        TColStd_Array1OfReal Knots(1, aS1R->NbVKnots());
        aS1R->VKnots(Knots);
        TColStd_Array1OfInteger Mults(1, aS1R->NbVKnots());
        aS1R->VMultiplicities(Mults);

        aS2R->InsertVKnots(Knots, Mults, ptol, Standard_False);
    }
    else {
        TColStd_Array1OfReal aVKnots(1, vknts1);
        for (i = 1; i <= vknts1; ++i) {
            aVKnots(i) = (aS1R->VKnot(i) + aS2R->VKnot(i))/2.;
        }
        aS1R->SetVKnots(aVKnots);
        aS2R->SetVKnots(aVKnots);
    }

    //
    //
    GeomConvert_BSplineSurfaceToBezierSurface aDisass1(aS1R);

    nbu1 = aDisass1.NbUPatches();
    nbv1 = aDisass1.NbVPatches();

    TColGeom_Array2OfBezierSurface aBzrArr1(1, nbu1, 1, nbv1);

    aDisass1.Patches(aBzrArr1);

    GeomConvert_BSplineSurfaceToBezierSurface aDisass2(aS2R);

    nbu2 = aDisass2.NbUPatches();
    nbv2 = aDisass2.NbVPatches();
    //
    if (nbv1 != nbv2) {
        return 1;
    }
    //
    TColGeom_Array2OfBezierSurface aBzrArr2(1, nbu2, 1, nbv2);

    aDisass2.Patches(aBzrArr2);
    //
    nbu = nbu1+nbu2;
    nbv = nbv1;

    TColGeom_Array2OfBezierSurface aBzrArr(1, nbu, 1, nbv);
    //
    for (i = 1; i <= nbu1; i++) {
        for (j = 1; j <= nbv; j++) {
            aBzrArr(i, j) = aBzrArr1(i, j);
        }
    }

    for (i = 1; i <= nbu2; i++) {
        for (j = 1; j <= nbv; j++) {
            aBzrArr(i+nbu1, j) = aBzrArr2(i, j);
        }
    }
    //
    GeomConvert_CompBezierSurfacesToBSplineSurface anAss(aBzrArr, theTol);
    //
    if (!anAss.IsDone()) {
        return 1;
    }
    //
    theS = new Geom_BSplineSurface(anAss.Poles()->Array2(),
                                   anAss.UKnots()->Array1(),
                                   anAss.VKnots()->Array1(),
                                   anAss.UMultiplicities()->Array1(),
                                   anAss.VMultiplicities()->Array1(),
                                   anAss.UDegree(),
                                   anAss.VDegree());

    Standard_Real uf, ul, ubnd;
    //
    uf = theS->UKnot(1);
    ubnd = theS->UKnot(aS1R->NbUKnots());
    ul = theS->UKnot(theS->NbUKnots());

    u1l = u1f + (u1l-u1f)/(ubnd-uf)*(ul-uf);

    SurfTools::UReparametrize(theS, u1f, u1l);
    SurfTools::VReparametrize(theS, v1f, v1l);
    //
    if (VContinuity(theS) < anVCont) {
        VSmoothing(theS, anVCont, theTol);
    }

    return 0;

}
//===================================================================
// SurfTools::ConcatenateSurfaces
//===================================================================
Standard_Integer SurfTools::ConcatenateSurfaces(
    const TColGeom_SequenceOfSurface& theSurfaces,
    const Standard_Real theTol,
    Handle(Geom_BSplineSurface)& theS)
{
    Standard_Integer i, err;
    Handle(Geom_BSplineSurface) aS1 =
        Handle(Geom_BSplineSurface)::DownCast(theSurfaces.Value(1));
    if (aS1.IsNull()) {
        return 1;
    }
    for (i = 2; i<= theSurfaces.Length(); ++i) {
        Handle(Geom_BSplineSurface) aS2 =
            Handle(Geom_BSplineSurface)::DownCast(theSurfaces.Value(i));
        if (aS2.IsNull()) {
            return 1;
        }
        err = ConcatenateSurfaces(aS1, aS2, theTol, theS);
        if (err != 0) {
            return err;
        }
        aS1 = theS;
    }

    Standard_Integer aCont = 2;
    if (UContinuity(theS) < aCont) {
        USmoothing(theS, aCont, theTol);
    }

    if (VContinuity(theS) < aCont) {
        VSmoothing(theS, aCont, theTol);
    }

    return err;
}
//===================================================================
// SurfTools::ConcatenateSurfaces
//===================================================================
Standard_Integer SurfTools::ConcatenateSurfaces(const Handle(Geom_BSplineSurface)& theS1,
                                                const Handle(Geom_BSplineSurface)& theS2,
                                                const Standard_Real theTol,
                                                Handle(Geom_BSplineSurface)& theS)
{
    Standard_Integer  i, j;
    Standard_Real u1f, u1l, v1f, v1l, u2f, u2l, v2f, v2l;
    gp_XYZ aP1[4], aP2[4];
    Handle(Geom_BSplineSurface) aS1R, aS2R;
    //
    theS1->Bounds(u1f, u1l, v1f, v1l);
    theS2->Bounds(u2f, u2l, v2f, v2l);
    //
    theS1->Bounds(u1f, u1l, v1f, v1l);
    theS2->Bounds(u2f, u2l, v2f, v2l);
    //
    aP1[0] = theS1->Value((u1f+u1l)/2., v1f).XYZ();
    aP1[1] = theS1->Value(u1l, (v1f+v1l)/2.).XYZ();
    aP1[2] = theS1->Value((u1l+u1f)/2., v1l).XYZ();
    aP1[3] = theS1->Value(u1f, (v1l+v1f)/2.).XYZ();
    //
    aP2[0] = theS2->Value((u2f+u2l)/2., v2f).XYZ();
    aP2[1] = theS2->Value(u2l, (v2f+v2l)/2.).XYZ();
    aP2[2] = theS2->Value((u2l+u2f)/2., v2l).XYZ();
    aP2[3] = theS2->Value(u2f, (v2l+v2f)/2.).XYZ();
    //
    aS1R = Handle(Geom_BSplineSurface)::DownCast(theS1->Copy());
    aS2R = Handle(Geom_BSplineSurface)::DownCast(theS2->Copy());
    //
    for (i=0; i<4; ++i) {
        for (j=0; j<4; ++j) {
            if (aP1[i].IsEqual(aP2[j], theTol)) {
                if (i == 0 || i == 2) {
                    if (j == 1 || j == 3) {
                        aS2R->ExchangeUV();
                    }
                    Standard_Integer ierr = ConcatenateSurfacesU(aS1R, aS2R, theTol, theS);
                    return ierr;
                }
                else {
                    if (j == 0 || j == 2) {
                        aS2R->ExchangeUV();
                    }
                    Standard_Integer ierr = ConcatenateSurfacesV(aS1R, aS2R, theTol, theS);
                    return ierr;
                }
            }
        }
    }

    return 1;
}
//===================================================================
// SurfTools::UReparametrize
//===================================================================
void SurfTools::UReparametrize(Handle(Geom_BSplineSurface)& theS,
                               const Standard_Real theUfirst,
                               const Standard_Real theUlast)
{

    Standard_Integer aNbUKnots = theS->NbUKnots();
    TColStd_Array1OfReal anUKnots(1, aNbUKnots);

    theS->UKnots(anUKnots);

    BSplCLib::Reparametrize(theUfirst, theUlast, anUKnots);

    theS->SetUKnots(anUKnots);

    return;
}

//===================================================================
// SurfTools::VReparametrize
//===================================================================
void SurfTools::VReparametrize(Handle(Geom_BSplineSurface)& theS,
                               const Standard_Real theVfirst,
                               const Standard_Real theVlast)
{

    Standard_Integer aNbVKnots = theS->NbVKnots();
    TColStd_Array1OfReal aVKnots(1, aNbVKnots);

    theS->VKnots(aVKnots);

    BSplCLib::Reparametrize(theVfirst, theVlast, aVKnots);

    theS->SetVKnots(aVKnots);

    return;
}

//===================================================================
// SurfTools::USmoothing
//===================================================================
void SurfTools::USmoothing(Handle(Geom_BSplineSurface)& theS,
                           const Standard_Integer theCont,
                           const Standard_Real theTol,
                           const Standard_Boolean theInsertKnots)
{

    Standard_Integer aNbIter = 10;
    Standard_Boolean bContinue = Standard_True;
    Standard_Integer iter = 1;
    TColStd_SequenceOfInteger aKnotIndex;
    TColStd_SequenceOfReal aKnotIns;

    while (bContinue && iter <= aNbIter) {

        Standard_Integer aNbUKnots = theS->NbUKnots();
        TColStd_Array1OfInteger aMults(1, aNbUKnots);
        TColStd_Array1OfReal aKnots(1, aNbUKnots);

        theS->UMultiplicities(aMults);
        theS->UKnots(aKnots);
        Standard_Integer i, m = theS->UDegree();
        m = m - theCont;

        if (m < 1) {
            return;
        }

        aKnotIndex.Clear();

        for (i = 2; i < aNbUKnots; i++) {
            if (aMults(i) > m) {
                if (!(theS->RemoveUKnot(i, m, theTol))) {
                    aKnotIndex.Append(i);
                }
            }
        }

        if (!theInsertKnots) {
            return;
        }

        //Prepare knots for inserting;

        Standard_Integer aNbAdd = aKnotIndex.Length();

        if (aNbAdd == 0) {
            return;
        }

        aKnotIns.Clear();

        Standard_Real aLastKnot = aKnots(1);
        for (i = 1; i <= aNbAdd; i++) {

            Standard_Integer anInd = aKnotIndex(i);

            Standard_Real aK1 = 0.5*(aKnots(anInd) + aKnots(anInd-1));
            if (Abs(aK1 - aLastKnot) > 1.e-3) {
                aKnotIns.Append(aK1);
                aLastKnot = aK1;
            }

            Standard_Real aK2 = 0.5*(aKnots(anInd+1) + aKnots(anInd));

            if (Abs(aK2 - aLastKnot) > 1.e-3) {
                aKnotIns.Append(aK2);
                aLastKnot = aK2;
            }

        }

        aNbAdd = aKnotIns.Length();

        for (i = 1; i <= aNbAdd; i++) {
            theS->InsertUKnot(aKnotIns(i), m, 1.e-9, Standard_False);
        }

        iter++;

    }

    if (iter > aNbIter) {
        SurfTools::USmoothing(theS, theCont, 1.e10);
    }

    return;
}
//===================================================================
// SurfTools::VSmoothing
//===================================================================
void SurfTools::VSmoothing(Handle(Geom_BSplineSurface)& theS,
                           const Standard_Integer theCont,
                           const Standard_Real theTol,
                           const Standard_Boolean theInsertKnots)
{

    Standard_Integer aNbIter = 10;
    Standard_Boolean bContinue = Standard_True;
    Standard_Integer iter = 1;
    TColStd_SequenceOfInteger aKnotIndex;
    TColStd_SequenceOfReal aKnotIns;

    while (bContinue && iter <= aNbIter) {

        Standard_Integer aNbVKnots = theS->NbVKnots();
        TColStd_Array1OfInteger aMults(1, aNbVKnots);
        TColStd_Array1OfReal aKnots(1, aNbVKnots);

        theS->VMultiplicities(aMults);
        theS->VKnots(aKnots);
        Standard_Integer i, m = theS->VDegree();
        m = m - theCont;

        if (m < 1) {
            return;
        }

        aKnotIndex.Clear();

        for (i = 2; i < aNbVKnots; i++) {
            if (aMults(i) > m) {
                if (!(theS->RemoveVKnot(i, m, theTol))) {
                    aKnotIndex.Append(i);
                }
            }
        }

        if (!theInsertKnots) {
            return;
        }

        //Prepare knots for inserting;

        Standard_Integer aNbAdd = aKnotIndex.Length();

        if (aNbAdd == 0) {
            return;
        }

        aKnotIns.Clear();

        Standard_Real aLastKnot = aKnots(1);
        for (i = 1; i <= aNbAdd; i++) {

            Standard_Integer anInd = aKnotIndex(i);

            Standard_Real aK1 = 0.5*(aKnots(anInd) + aKnots(anInd-1));
            if (Abs(aK1 - aLastKnot) > 1.e-3) {
                aKnotIns.Append(aK1);
                aLastKnot = aK1;
            }

            Standard_Real aK2 = 0.5*(aKnots(anInd+1) + aKnots(anInd));

            if (Abs(aK2 - aLastKnot) > 1.e-3) {
                aKnotIns.Append(aK2);
                aLastKnot = aK2;
            }

        }

        aNbAdd = aKnotIns.Length();

        for (i = 1; i <= aNbAdd; i++) {
            theS->InsertVKnot(aKnotIns(i), m, 1.e-9, Standard_False);
        }

        iter++;

    }

    if (iter > aNbIter) {
        SurfTools::VSmoothing(theS, theCont, 1.e10);
    }

    return;
}
