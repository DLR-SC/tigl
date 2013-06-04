/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLViewer.h 74 2011-08-12 12:39:28Z markus.litz $
*
* Version: $Revision: 74 $
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/



#ifndef TIGLVIEWER_H
#define TIGLVIEWER_H

#include <AIS_StatusOfDetection.hxx>
#include <AIS_StatusOfPick.hxx>
#include <Aspect_Drawable.hxx>
#include <Aspect_GridDrawMode.hxx>
#include <Aspect_GridType.hxx>
#include <Aspect_GraphicCallbackProc.hxx>
#include <Handle_AIS_InteractiveContext.hxx>
#include <Handle_V3d_View.hxx>
#include <Handle_V3d_Viewer.hxx>
#include <Handle_Visual3d_Layer.hxx>
#include <Handle_TopTools_HSequenceOfShape.hxx>
#include <Standard_TypeDef.hxx>
#include <Quantity_Factor.hxx>
#include <Quantity_Length.hxx>
#include <Quantity_NameOfColor.hxx>
#include <V3d_Coordinate.hxx>

#ifdef QOCC_STATIC
#define QOCC_DECLSPEC
#else
#ifdef QOCC_MAKEDLL
#define QOCC_DECLSPEC Q_DECL_EXPORT
#else
#define QOCC_DECLSPEC Q_DECL_IMPORT
#endif
#endif

#define SIGN(X) ((X) < 0. ? -1 : ((X) > 0. ? 1 : 0.))

#endif // TIGLVIEWER_H

