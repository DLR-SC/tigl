/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-11-11 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CFuseShapes.h"

#include "CNamedShape.h"
#include "CTiglError.h"
#include "CBooleanOperTools.h"
#include "CTrimShape.h"
#include "BRepSewingToBRepBuilderShapeAdapter.h"

#include <cassert>

#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Solid.hxx>
#include <TopExp.hxx>

#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>

#include <BOPAlgo_PaveFiller.hxx>
#include <BOPCol_ListOfShape.hxx>
#include <BRepAlgoAPI_Section.hxx>

//#define DEBUG_BOP

CFuseShapes::CFuseShapes(const PNamedShape parent, const ListPNamedShape &childs)
    : _resultshape()
{
    Clear();
    _parent = parent;
    _childs = childs;
}

CFuseShapes::~CFuseShapes()
{
}

const PNamedShape CFuseShapes::NamedShape()
{
    Perform();
    return _resultshape;
}

CFuseShapes::operator PNamedShape()
{
    return NamedShape();
}

const ListPNamedShape &CFuseShapes::Intersections()
{
    Perform();
    return _intersections;
}

const ListPNamedShape &CFuseShapes::TrimmedChilds()
{
    Perform();
    return _trimmedChilds;
}

const PNamedShape CFuseShapes::TrimmedParent()
{
    Perform();
    return _trimmedParent;
}

void CFuseShapes::Perform()
{
    if (!_hasPerformed) {
        DoFuse();
        _hasPerformed = true;
    }
}

void CFuseShapes::Clear()
{
    _hasPerformed = false;
    _intersections.clear();
    _trimmedChilds.clear();
    _trimmedParent.reset();
    _parent.reset();
    _childs.clear();
    _resultshape.reset();

}

/// This is the actual fusing
void CFuseShapes::DoFuse()
{
    BRepBuilderAPI_Sewing shellMaker;

    _trimmedChilds.clear();
    ListPNamedShape::const_iterator childIter;

    TrimOperation childTrim  = EXCLUDE;
    TrimOperation parentTrim = EXCLUDE;

    // handle case when parent has no shape
    if (!_parent) {
        _trimmedParent.reset();
        for (childIter = _childs.begin(); childIter != _childs.end(); ++childIter) {
            const PNamedShape child = *childIter;
            if (!child) {
                continue;
            }
            _trimmedChilds.push_back(child);
        }
    }
    else {
        // trim the childs with the parent and vice versa
        _trimmedParent = _parent->DeepCopy();
        for (childIter = _childs.begin(); childIter != _childs.end(); ++childIter) {
            const PNamedShape child = *childIter;
            if (!child) {
                continue;
            }

#ifdef DEBUG_BOP
            clock_t start, stop;
            start = clock();
#endif
            BOPCol_ListOfShape aLS;
            aLS.Append(_trimmedParent->Shape());
            aLS.Append(child->Shape());
            BOPAlgo_PaveFiller DSFill;
            DSFill.SetArguments(aLS);
            DSFill.Perform();
#ifdef DEBUG_BOP
            stop = clock();
            printf("dsfiller [ms]: %f\n", (stop-start)/(double)CLOCKS_PER_SEC * 1000.);

            start = clock();
#endif
            // calculate intersection
            // Todo: make a new BOP out of this
            TopoDS_Shape intersection = BRepAlgoAPI_Section(_trimmedParent->Shape(), child->Shape(), DSFill);
            PNamedShape intersectionShape(new CNamedShape(intersection, std::string("INT" + std::string(_parent->Name()) + child->Name()).c_str()));
            intersectionShape->SetShortName(std::string("INT" + std::string(_parent->ShortName()) + child->ShortName()).c_str());
            _intersections.push_back(intersectionShape);

#ifdef DEBUG_BOP
            stop = clock();
            printf("intersection [ms]: %f\n", (stop-start)/(double)CLOCKS_PER_SEC * 1000.);

            start = clock();
#endif
            _trimmedParent = CTrimShape(_trimmedParent, child, DSFill, parentTrim);

#ifdef DEBUG_BOP
            stop = clock();
            printf("parent split [ms]: %f\n", (stop-start)/(double)CLOCKS_PER_SEC * 1000.);

            start = clock();
#endif
            PNamedShape trimmedChild = CTrimShape(child, _parent, DSFill, childTrim);
            _trimmedChilds.push_back(trimmedChild);

#ifdef DEBUG_BOP
            stop = clock();
            printf("child split [ms]: %f\n", (stop-start)/(double)CLOCKS_PER_SEC * 1000.);
#endif
        } // trimming
    }

    // add trimmed child faces to result
    for (childIter = _trimmedChilds.begin(); childIter != _trimmedChilds.end(); ++childIter) {
        shellMaker.Add((*childIter)->Shape());
    }

    // add trimmed parent faces to result
    if (_trimmedParent) {
        shellMaker.Add(_trimmedParent->Shape());
    }
    shellMaker.Perform();

    // make a solid out of the face collection
    TopoDS_Shape shell = shellMaker.SewedShape();


    BRepSewingToBRepBuilderShapeAdapter sewerAdapter(shellMaker);

    // map names to shell
    PNamedShape resultShell(new CNamedShape(shell, "BOP_FUSE"));
    for (childIter = _trimmedChilds.begin(); childIter != _trimmedChilds.end(); ++childIter) {
        const PNamedShape child = *childIter;
        PNamedShape tmpshape(new CNamedShape(*child));
        tmpshape->SetShape(shellMaker.ModifiedSubShape(child->Shape()));
        CBooleanOperTools::MapFaceNamesAfterBOP(sewerAdapter, tmpshape, resultShell);
    }

    if (_trimmedParent) {
        PNamedShape tmpshape(new CNamedShape(*_trimmedParent));
        tmpshape->SetShape(shellMaker.ModifiedSubShape(_trimmedParent->Shape()));
        CBooleanOperTools::MapFaceNamesAfterBOP(sewerAdapter, tmpshape, resultShell);
    }

    // map names to solid
    BRepBuilderAPI_MakeSolid solidmaker;
    TopTools_IndexedMapOfShape shellMap;
    TopExp::MapShapes(resultShell->Shape(), TopAbs_SHELL, shellMap);
    for (int ishell = 1; ishell <= shellMap.Extent(); ++ishell) {
        const TopoDS_Shell& shell = TopoDS::Shell(shellMap(ishell));
        solidmaker.Add(shell);
    }

    PNamedShape result(new CNamedShape(solidmaker.Solid(), resultShell->Name()));
    CBooleanOperTools::MapFaceNamesAfterBOP(solidmaker, resultShell, result);

    _resultshape = result;
}
