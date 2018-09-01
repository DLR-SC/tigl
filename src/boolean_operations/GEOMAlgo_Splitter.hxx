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
//  File:   GEOMAlgo_Splitter.hxx
//
//  Author: Peter KURNEV
// Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE

#ifndef GEOMAlgo_Splitter_HeaderFile
#define GEOMAlgo_Splitter_HeaderFile

#include "tigl_internal.h"

#include <Standard.hxx>
#include <Standard_Version.hxx>
#include <Standard_Macro.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Integer.hxx>

#include <NCollection_BaseAllocator.hxx>

#include <TopAbs_ShapeEnum.hxx>

#include <TopoDS_Shape.hxx>

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#else
#include <BOPCol_ListOfShape.hxx>
#include <BOPCol_MapOfShape.hxx>
#endif

#include <BOPAlgo_Builder.hxx>

//=======================================================================
//class    : GEOMAlgo_Splitter
//purpose  :
//=======================================================================
class GEOMAlgo_Splitter : public BOPAlgo_Builder
{
public:
  
  TIGL_EXPORT
    GEOMAlgo_Splitter();

  TIGL_EXPORT
    GEOMAlgo_Splitter(const Handle(NCollection_BaseAllocator)& theAllocator);
  
  TIGL_EXPORT
    ~GEOMAlgo_Splitter() OVERRIDE;

  TIGL_EXPORT
    void AddTool(const TopoDS_Shape& theShape);

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
  TIGL_EXPORT
    const TopTools_ListOfShape& Tools()const;
#else
  TIGL_EXPORT
    const BOPCol_ListOfShape& Tools()const;
#endif

  TIGL_EXPORT
    void SetLimit(const TopAbs_ShapeEnum aLimit);

  TIGL_EXPORT
    TopAbs_ShapeEnum Limit()const;

  TIGL_EXPORT
    void SetLimitMode(const Standard_Integer aMode);

  TIGL_EXPORT
    Standard_Integer LimitMode()const;

  TIGL_EXPORT
    void Clear() OVERRIDE;

 protected:
  TIGL_EXPORT
    void BuildResult(const TopAbs_ShapeEnum theType) OVERRIDE;

  TIGL_EXPORT
    void PostTreat() OVERRIDE;
  
 protected:
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,3,0)
  TopTools_ListOfShape myTools;
  TopTools_MapOfShape myMapTools;
#else
  BOPCol_ListOfShape myTools;
  BOPCol_MapOfShape myMapTools;
#endif
  TopAbs_ShapeEnum myLimit;
  Standard_Integer myLimitMode;
};

#endif
