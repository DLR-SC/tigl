// Copyright (C) 2007-2013  CEA/DEN, EDF R&D, OPEN CASCADE
//
// Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
// CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File:        GEOMAlgo_Splitter.cxx
// Created:     Thu Sep 06 10:54:04 2012
// Author:      Peter KURNEV
//              <pkv@irinox>
//

#include <GEOMAlgo_Splitter.hxx>

#include <TopAbs_ShapeEnum.hxx>

#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>

#include <BRep_Builder.hxx>
#include <Standard_Version.hxx>

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
#include <TopTools_MapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#else
#include <BOPCol_MapOfShape.hxx>
#include <BOPCol_ListOfShape.hxx>
#include <BOPCol_IndexedMapOfShape.hxx>
#include <BOPTools.hxx>
#endif


static 
  void TreatCompound(const TopoDS_Shape& aC, 
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
                     TopTools_ListOfShape& aLSX);
#else
                     BOPCol_ListOfShape& aLSX);
#endif

//=======================================================================
//function : 
//purpose  : 
//=======================================================================
GEOMAlgo_Splitter::GEOMAlgo_Splitter()
:
  BOPAlgo_Builder(),
  myTools(myAllocator),
  myMapTools(100, myAllocator)
{
  myLimit=TopAbs_SHAPE;
  myLimitMode=0;
}
//=======================================================================
//function : 
//purpose  : 
//=======================================================================
GEOMAlgo_Splitter::GEOMAlgo_Splitter
  (const Handle(NCollection_BaseAllocator)& theAllocator)
:
  BOPAlgo_Builder(theAllocator),
  myTools(myAllocator),
  myMapTools(100, myAllocator)
{
  myLimit=TopAbs_SHAPE;
  myLimitMode=0;
}
//=======================================================================
//function : ~
//purpose  : 
//=======================================================================
GEOMAlgo_Splitter::~GEOMAlgo_Splitter()
{
}
//=======================================================================
//function : AddTool
//purpose  : 
//=======================================================================
void GEOMAlgo_Splitter::AddTool(const TopoDS_Shape& theShape)
{
  if (myMapTools.Add(theShape)) {
    myTools.Append(theShape);
    //
    AddArgument(theShape);
  }
}
//=======================================================================
//function : Tools
//purpose  : 
//=======================================================================
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
const TopTools_ListOfShape& GEOMAlgo_Splitter::Tools()const
#else
const BOPCol_ListOfShape& GEOMAlgo_Splitter::Tools()const
#endif
{
  return myTools;
}
//=======================================================================
//function : SetLimit
//purpose  : 
//=======================================================================
void GEOMAlgo_Splitter::SetLimit(const TopAbs_ShapeEnum aLimit) 
{
  myLimit=aLimit;
}
//=======================================================================
//function : Limit
//purpose  : 
//=======================================================================
TopAbs_ShapeEnum GEOMAlgo_Splitter::Limit()const
{
  return myLimit;
}
//=======================================================================
//function : SetLimitMode
//purpose  : 
//=======================================================================
void GEOMAlgo_Splitter::SetLimitMode(const Standard_Integer aMode)
{
  myLimitMode=aMode;
}
//=======================================================================
//function : LimitMode
//purpose  : 
//=======================================================================
Standard_Integer GEOMAlgo_Splitter::LimitMode()const
{
  return myLimitMode;
}
//=======================================================================
//function : Clear
//purpose  : 
//=======================================================================
void GEOMAlgo_Splitter::Clear()
{
  myTools.Clear();
  myMapTools.Clear();
  myLimit=TopAbs_SHAPE;
  BOPAlgo_Builder::Clear();
}
//=======================================================================
//function : BuildResult
//purpose  : 
//=======================================================================
void GEOMAlgo_Splitter::BuildResult(const TopAbs_ShapeEnum theType)
{
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,2,0)
  myReport->Clear();
#else
  myErrorStatus=0;
#endif
  //
  TopAbs_ShapeEnum aType;
  BRep_Builder aBB;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
  TopTools_MapOfShape aM;
  TopTools_ListIteratorOfListOfShape aIt, aItIm;
#else
  BOPCol_MapOfShape aM;
  BOPCol_ListIteratorOfListOfShape aIt, aItIm;
#endif
  //
  aIt.Initialize(myArguments);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aS=aIt.Value();
    aType=aS.ShapeType();
    if (aType==theType && !myMapTools.Contains(aS)) {
      if (myImages.IsBound(aS)) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
        const TopTools_ListOfShape& aLSIm=myImages.Find(aS);
#else
        const BOPCol_ListOfShape& aLSIm=myImages.Find(aS);
#endif
        aItIm.Initialize(aLSIm);
        for (; aItIm.More(); aItIm.Next()) {
          const TopoDS_Shape& aSIm=aItIm.Value();
          if (aM.Add(aSIm)) {
            aBB.Add(myShape, aSIm);
          }
        }
      }
      else {
        if (aM.Add(aS)) {
          aBB.Add(myShape, aS);
        }
      }
    }
  }
}
//=======================================================================
//function : PostTreat
//purpose  : 
//=======================================================================
void GEOMAlgo_Splitter::PostTreat()
{
  if (myLimit!=TopAbs_SHAPE) {
    Standard_Integer i, aNbS;
    BRep_Builder aBB;
    TopoDS_Compound aC;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
    TopTools_IndexedMapOfShape aMx;
#else
    BOPCol_IndexedMapOfShape aMx;
#endif
    //
    aBB.MakeCompound(aC);
    //
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
    TopExp::MapShapes(myShape, myLimit, aMx);
#else
    BOPTools::MapShapes(myShape, myLimit, aMx);
#endif
    aNbS=aMx.Extent();
    for (i=1; i<=aNbS; ++i) {
      const TopoDS_Shape& aS=aMx(i);
      aBB.Add(aC, aS);
    }
    if (myLimitMode) {
      Standard_Integer iType, iLimit, iTypeX;
      TopAbs_ShapeEnum aType, aTypeX;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
      TopTools_ListOfShape aLSP, aLSX;
      TopTools_ListIteratorOfListOfShape aIt, aItX, aItIm;
      TopTools_MapOfShape  aM;
#else
      BOPCol_ListOfShape aLSP, aLSX;
      BOPCol_ListIteratorOfListOfShape aIt, aItX, aItIm;
      BOPCol_MapOfShape  aM;
#endif
      //
      iLimit=(Standard_Integer)myLimit; 
      //
      // 1. Collect the shapes to process aLSP
      aIt.Initialize(myArguments);
      for (; aIt.More(); aIt.Next()) {
        const TopoDS_Shape& aS=aIt.Value();
        if (myMapTools.Contains(aS)) {
          continue;
        }
        //
        aType=aS.ShapeType();
        iType=(Standard_Integer)aType;
        //
        if (iType>iLimit) {
          aLSP.Append(aS);
        }
        //
        else if (aType==TopAbs_COMPOUND) {
          aLSX.Clear();
          //
          TreatCompound(aS, aLSX);
          //
          aItX.Initialize(aLSX);
          for (; aItX.More(); aItX.Next()) {
            const TopoDS_Shape& aSX=aItX.Value();
            aTypeX=aSX.ShapeType();
            iTypeX=(Standard_Integer)aTypeX;
            //
            if (iTypeX>iLimit) {
              aLSP.Append(aSX);
            }
          }
        }
      }// for (; aIt.More(); aIt.Next()) {
      //
      aMx.Clear();
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
      TopExp::MapShapes(aC, aMx);
#else
      BOPTools::MapShapes(aC, aMx);
#endif
       // 2. Add them to aC
      aIt.Initialize(aLSP);
      for (; aIt.More(); aIt.Next()) {
        const TopoDS_Shape& aS=aIt.Value();
        if (myImages.IsBound(aS)) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
          const TopTools_ListOfShape& aLSIm=myImages.Find(aS);
#else
          const BOPCol_ListOfShape& aLSIm=myImages.Find(aS);
#endif
          aItIm.Initialize(aLSIm);
          for (; aItIm.More(); aItIm.Next()) {
            const TopoDS_Shape& aSIm=aItIm.Value();
            if (aM.Add(aSIm)) {
              if (!aMx.Contains(aSIm)) {
                aBB.Add(aC, aSIm);
              }
            }
          }
        }
        else {
          if (aM.Add(aS)) {
            if (!aMx.Contains(aS)) {
              aBB.Add(aC, aS);
            }
          }
        }
      }
    }// if (myLimitMode) {
    myShape=aC;
  }//if (myLimit!=TopAbs_SHAPE) {
  //
  Standard_Integer aNbS;
  TopoDS_Iterator aIt;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
  TopTools_ListOfShape aLS;
#else
  BOPCol_ListOfShape aLS;
#endif
  //
  aIt.Initialize(myShape);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aS=aIt.Value();
    aLS.Append(aS);
  }
  aNbS=aLS.Extent();
  if (aNbS==1) {
    myShape=aLS.First();
  }
  //
  BOPAlgo_Builder::PostTreat();
}
//=======================================================================
//function : TreatCompound
//purpose  : 
//=======================================================================
void TreatCompound(const TopoDS_Shape& aC1, 
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
                   TopTools_ListOfShape& aLSX)
#else
                   BOPCol_ListOfShape& aLSX)
#endif
{
  Standard_Integer aNbC1;
  TopAbs_ShapeEnum aType;
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
  TopTools_ListOfShape aLC, aLC1;
  TopTools_ListIteratorOfListOfShape aIt, aIt1;
#else
  BOPCol_ListOfShape aLC, aLC1;
  BOPCol_ListIteratorOfListOfShape aIt, aIt1;
#endif
  TopoDS_Iterator aItC;
  //
  aLC.Append (aC1);
  while(1) {
    aLC1.Clear();
    aIt.Initialize(aLC);
    for (; aIt.More(); aIt.Next()) {
      const TopoDS_Shape& aC=aIt.Value(); //C is compound
      //
      aItC.Initialize(aC);
      for (; aItC.More(); aItC.Next()) {
        const TopoDS_Shape& aS=aItC.Value();
        aType=aS.ShapeType();
        if (aType==TopAbs_COMPOUND) {
          aLC1.Append(aS);
        }
        else {
          aLSX.Append(aS);
        }
      }
    }
    //
    aNbC1=aLC1.Extent();
    if (!aNbC1) {
      break;
    }
    //
    aLC.Clear();
    aIt.Initialize(aLC1);
    for (; aIt.More(); aIt.Next()) {
      const TopoDS_Shape& aSC=aIt.Value();
      aLC.Append(aSC);
    }
  }// while(1)
}
//
// myErrorStatus
// 
// 0  - Ok
// 1  - The object is just initialized
// 2  - PaveFiller is failed
// 10 - No shapes to process
// 30 - SolidBuilder failed
