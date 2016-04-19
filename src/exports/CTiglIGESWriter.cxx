/* 
* Copyright (C) 2012 Cassidian Air Systems
*
* Created: 2012-11-01 Roland Landertshamer
*
* Changed: $Id $ 
*
* Version: $Revision $
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

#include "CTiglIGESWriter.h"

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepToIGES_BREntity.hxx>
#include <errno.h>
#include <gp_XYZ.hxx>
#include <IGESBasic_Group.hxx>
#include <IGESData_IGESModel.hxx>
#include <IGESData_IGESEntity.hxx>
#include <IGESData_IGESWriter.hxx>
#include <IGESSelect_WorkLibrary.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Static.hxx>
#include <Message_ProgressIndicator.hxx>
#include <ShapeAnalysis_ShapeTolerance.hxx>
#include <Standard_Stream.hxx>
#include <TopExp_Explorer.hxx>
#include <Transfer_FinderProcess.hxx>
#include <XSAlgo.hxx>
#include <XSAlgo_AlgoContainer.hxx>
#include <IGESGeom_TrimmedSurface.hxx>
#include <IGESGeom_CurveOnSurface.hxx>
#include <IGESGeom_CompositeCurve.hxx>
#include <IGESData_IGESType.hxx>

CTiglIGESWriter::CTiglIGESWriter ()
	: transferFinderProcess (new Transfer_FinderProcess(10000)) ,
	  editor (IGESSelect_WorkLibrary::DefineProtocol()) ,
	  writeMode (0) , isValid (Standard_False)
{
	IGESControl_Controller::Init();
	editor.SetUnitName(Interface_Static::CVal ("write.iges.unit"));
	editor.ApplyUnit();
	igesModel = editor.Model();
}

Standard_Boolean CTiglIGESWriter::AddShape (const TopoDS_Shape& theShape, int level)
{
	if (theShape.IsNull()) {
		return Standard_False;
	}

	// TODO: MAYBE REMOVE THIS, WE MAY NOT NEED A PROGRESS INDICATOR
	Handle(Message_ProgressIndicator) progress = transferFinderProcess->GetProgress();
	if (! progress.IsNull()) {
		Standard_Integer nbfaces=0;
		for( TopExp_Explorer exp(theShape,TopAbs_FACE); exp.More(); exp.Next() ) {
			nbfaces++;
		}
		progress->SetScale ( "Faces", 0, nbfaces, 1 );
	}
  
	XSAlgo::AlgoContainer()->PrepareForTransfer();
  
	//  modified by NIZHNY-EAP Tue Aug 29 11:16:54 2000 ___BEGIN___
	Handle(Standard_Transient) info;
	Standard_Real Tol = Interface_Static::RVal("write.precision.val");
	Standard_Real maxTol = Interface_Static::RVal("read.maxprecision.val");
	TopoDS_Shape Shape = XSAlgo::AlgoContainer()->ProcessShape( theShape, Tol, maxTol, 
															  "write.iges.resource.name", 
															  "write.iges.sequence", info );
	//  modified by NIZHNY-EAP Tue Aug 29 11:17:01 2000 ___END___
	BRepToIGES_BREntity B0;
	B0.SetTransferProcess(transferFinderProcess);
	B0.SetModel(igesModel);
	Handle(IGESData_IGESEntity) ent = B0.TransferShape(Shape);
	if (ent.IsNull()) {
		return Standard_False;
	}
	// store level information in all entities
	assignLevelToAllEntities(ent, level);

//  modified by NIZHNY-EAP Tue Aug 29 11:37:18 2000 ___BEGIN___
	XSAlgo::AlgoContainer()->MergeTransferInfo(transferFinderProcess, info);
//  modified by NIZHNY-EAP Tue Aug 29 11:37:25 2000 ___END___
  
  //22.10.98 gka BUC60080

	Standard_Integer oldnb = igesModel->NbEntities();
	igesModel->AddWithRefs(ent,IGESSelect_WorkLibrary::DefineProtocol());
	isValid = Standard_False;
	Standard_Integer newnb = igesModel->NbEntities();

	Standard_Real oldtol = igesModel->GlobalSection().Resolution();
	Standard_Real newtol;

	Standard_Integer tolmod = Interface_Static::IVal("write.precision.mode");
	if (tolmod == 2) {
		newtol = Interface_Static::RVal("write.precision.val");
	} else {
		ShapeAnalysis_ShapeTolerance stu; 
		Standard_Real Tolv = stu.Tolerance (Shape, tolmod, TopAbs_VERTEX);
		Standard_Real Tole = stu.Tolerance (Shape, tolmod, TopAbs_EDGE); 

		if (tolmod == 0 ) {   //Average
			Standard_Real Tol1 = (Tolv + Tole) / 2;
			newtol = (oldtol * oldnb + Tol1 * (newnb - oldnb)) / newnb;
		}
		else if (tolmod < 0) {//Least
			newtol = Min (Tolv, Tole);
			if (oldnb > 0) newtol = Min (oldtol, newtol);
		}
		else {                //Greatest
			newtol = Max (Tolv, Tole);
			if (oldnb > 0) newtol = Max (oldtol, newtol);
		}
	}

	IGESData_GlobalSection gs = igesModel->GlobalSection();
	gs.SetResolution (newtol / gs.UnitValue());//rln 28.12.98 CCI60005

	//#34 22.10.98 rln BUC60081
	Bnd_Box box;
	BRepBndLib::Add (Shape, box);
	Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
	box.Get (aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
	gs.MaxMaxCoords (gp_XYZ (aXmax / gs.UnitValue(),
		aYmax / gs.UnitValue(),
		aZmax / gs.UnitValue()));
	gs.MaxMaxCoords (gp_XYZ (aXmin / gs.UnitValue(),
		aYmin / gs.UnitValue(),
		aZmin / gs.UnitValue()));

	igesModel->SetGlobalSection(gs);

	return Standard_True;
}

void CTiglIGESWriter::assignLevelToTrimmedSurfaces(Handle(IGESData_IGESEntity) ent, int level) {
	if (ent->TypeNumber() == 144) {
		// assign level in case of trimmed surfaces type
		ent->InitLevel(0, level);
	} else if (ent->IsInstance(STANDARD_TYPE(IGESBasic_Group))) {
		// recurse down into single groups until trimmed surface is found
		Handle(IGESBasic_Group) group = Handle(IGESBasic_Group)::DownCast(ent);
		int numEntities = group->NbEntities();
		for (int i=1; i <= numEntities; i++) {
			assignLevelToTrimmedSurfaces(group->Entity(i), level);
		}
	}
}

void CTiglIGESWriter::assignLevelToAllEntities(Handle(IGESData_IGESEntity) ent, int level) {
	// assign level in case of trimmed surfaces type
	ent->InitLevel(0, level);
	if (ent->IsInstance(STANDARD_TYPE(IGESBasic_Group))) {
		// recurse down into single groups until trimmed surface is found
		Handle(IGESBasic_Group) group = Handle(IGESBasic_Group)::DownCast(ent);
		int numEntities = group->NbEntities();
		for (int i=1; i <= numEntities; i++) {
			assignLevelToAllEntities(group->Entity(i), level);
		}
	} else if (ent->IsInstance(STANDARD_TYPE(IGESGeom_TrimmedSurface))) {
		Handle(IGESGeom_TrimmedSurface) trimmedSurf = Handle(IGESGeom_TrimmedSurface)::DownCast(ent);
		assignLevelToAllEntities(trimmedSurf->Surface(), level);
		if (trimmedSurf->HasOuterContour()) {
			assignLevelToAllEntities(trimmedSurf->OuterContour(), level);
		}
		for (int i=1; i <= trimmedSurf->NbInnerContours(); i++) {
			assignLevelToAllEntities(trimmedSurf->InnerContour(i), level);
		}
	} else if (ent->IsInstance(STANDARD_TYPE(IGESGeom_CurveOnSurface))) {
		Handle(IGESGeom_CurveOnSurface) curveOnSurf = Handle(IGESGeom_CurveOnSurface)::DownCast(ent);
		if (!curveOnSurf->CurveUV().IsNull()) {
			assignLevelToAllEntities(curveOnSurf->CurveUV(), level);
		}
		if (!curveOnSurf->Curve3D().IsNull()) {
			assignLevelToAllEntities(curveOnSurf->Curve3D(), level);
		}
	} else if (ent->IsInstance(STANDARD_TYPE(IGESGeom_CompositeCurve))) {
		Handle(IGESGeom_CompositeCurve) compositeCurve = Handle(IGESGeom_CompositeCurve)::DownCast(ent);
		for (int i=1; i <= compositeCurve->NbCurves(); i++) {
			assignLevelToAllEntities(compositeCurve->Curve(i), level);
		}
	}
}

void CTiglIGESWriter::ComputeModel ()
{
	if (isValid) {
		return;
	}
	editor.ComputeStatus();
	editor.AutoCorrectModel();
	isValid = Standard_True;
}

Standard_Boolean CTiglIGESWriter::Write (const Standard_CString file)
{
	ofstream fout(file,ios::out);
	if (!fout) {
		return Standard_False;
	}

	ComputeModel();
	IGESData_IGESWriter igesWriter (igesModel);
	igesWriter.SendModel (IGESSelect_WorkLibrary::DefineProtocol());
	Standard_Boolean res = igesWriter.Print(fout);

	errno = 0;
	fout.close();
	res = fout.good() && res && !errno;

	return res;
}
