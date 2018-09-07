/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2018-08-22 Jan Kleinert <Jan.Kleinert@dlr.de>
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

#include "CTiglPatchShell.h"
#include "CTiglError.h"


#include <cassert>

#include <TopoDS.hxx>
#include <BRepLib.hxx>
#include <BRep_Builder.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <Precision.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <StdFail_NotDone.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <Geom_Plane.hxx>

namespace
{
    TopoDS_Shell MakeShells(TopoDS_Shape const& shell, const Standard_Real tol);
} // namespace

namespace tigl {

CTiglPatchShell::CTiglPatchShell(TopoDS_Shape const& shell, double tol)
 : _inputShell(shell)
 , _tolerance(tol)
{}

void CTiglPatchShell::AddSideCap(TopoDS_Wire const& boundaryWire)
{

    _hasPerformed = false;
    bool Ok = false;

    TopoDS_Face cap;

    bool isDegen = Standard_True;
    TopoDS_Iterator iter(boundaryWire);
    for (; iter.More(); iter.Next())
    {
        const TopoDS_Edge& anEdge = TopoDS::Edge(iter.Value());
        if (!BRep_Tool::Degenerated(anEdge)) {
            isDegen = Standard_False;
        }
    }
    if (isDegen)
        Ok = true;


    if (!boundaryWire.IsNull()) {
        BRepBuilderAPI_FindPlane Searcher( boundaryWire, _tolerance );
        if (Searcher.Found()) {
            cap = BRepBuilderAPI_MakeFace(Searcher.Plane(), boundaryWire);
            Ok = true;
        }
        else {
            // try to find another surface
            BRepBuilderAPI_MakeFace MF( boundaryWire );
            if (MF.IsDone())
            {
                cap = MF.Face();
                Ok = true;
            }
        }
    }

    if (Ok) {
        _sidecaps.push_back(cap);
    }
}

TopoDS_Shape CTiglPatchShell::PatchedShape()
{
    if( !_hasPerformed ) {
        Perform();
    }
    return _result;
}

void CTiglPatchShell::Perform()
{
    TopoDS_Shape shell = MakeShells(_inputShell, _tolerance);

    if (_sidecaps.size()>0) {
        // close holes using side caps
        Standard_Boolean B = shell.Closed();

        BRepBuilderAPI_Sewing sewingAlgo;
        sewingAlgo.Add(shell);


        if (!B) {
            for(int i = 0; i < _sidecaps.size(); ++i ) {
                sewingAlgo.Add(_sidecaps[i]);
            }
        }

        sewingAlgo.Perform();
        shell  = sewingAlgo.SewedShape();
        shell.Closed(Standard_True);
    }

    if (_makeSolid) {
        BRep_Builder B;
        TopoDS_Solid solid;
        B.MakeSolid(solid);
        try {
            B.Add(solid, shell);
        }
        catch ( ... ) {
            throw CTiglError("Cannot make a solid out of the shell. Is the base type correct?", TIGL_ERROR);
        }

        // verify the orientation of the solid
        BRepClass3d_SolidClassifier clas3d(solid);
        clas3d.PerformInfinitePoint(Precision::Confusion());
        if (clas3d.State() == TopAbs_IN) {
            B.MakeSolid(solid);
            TopoDS_Shape aLocalShape = shell.Reversed();
            B.Add(solid, TopoDS::Shell(aLocalShape));
        }
        _result = solid;
    }
    else {
        _result = MakeShells(shell, _tolerance);
    }

    BRepLib::EncodeRegularity(_result);
    _hasPerformed = true;
}

} // namespace tigl

namespace {

TopoDS_Shell MakeShells(TopoDS_Shape const& shell, const Standard_Real tol)
{
    if (shell.IsNull()) {
        StdFail_NotDone::Raise("Loft is not build");
    }

    BRepBuilderAPI_Sewing BB(tol);
    BB.Add(shell);
    BB.Perform();

    TopoDS_Shape shellClosed  = BB.SewedShape();

    if ( shellClosed.ShapeType() != TopAbs_SHELL ) {

        assert(shellClosed.ShapeType() == TopAbs_FACE);

        BRep_Builder B;
        TopoDS_Shell shellFinal;
        B.MakeShell(shellFinal);
        B.Add(shellFinal, shellClosed);
        return shellFinal;
    }
    else {
        return TopoDS::Shell(shellClosed);
    }
}

}
