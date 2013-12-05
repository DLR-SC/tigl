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

#include "CTrimShape.h"

#include "CBooleanOperTools.h"
#include "GEOMAlgo_Splitter.hxx"
#include "BOPBuilderShapeToBRepBuilderShapeAdapter.h"
#include "tiglcommonfunctions.h"

#include <cassert>

#include <BOPAlgo_PaveFiller.hxx>
#include <BOPCol_ListOfShape.hxx>

#include <Precision.hxx>

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>

#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <BRepClass3d_SolidClassifier.hxx>

#include <Geom_Surface.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>

namespace {

TopoDS_Shape GetFacesNotInShape(BRepBuilderAPI_MakeShape& bop, const TopoDS_Shape& originalShape, const TopoDS_Shape& splittedShape, const TopoDS_Shape& shapeToExInclude, TrimOperation op){

    TopoDS_Compound compound;
    BRep_Builder compoundmaker;
    compoundmaker.MakeCompound(compound);

    // add splitted faces to compound
    TopTools_IndexedMapOfShape originMap;
    TopExp::MapShapes(originalShape,   TopAbs_FACE, originMap);
    for(int i = 1; i <= originMap.Extent(); ++i) {
        const TopoDS_Face& face = TopoDS::Face(originMap(i));
        const TopTools_ListOfShape& splits = bop.Modified(face);
        TopTools_ListIteratorOfListOfShape it;
        for(it.Initialize(splits); it.More(); it.Next()) {
            // check if face is inside
            TopoDS_Face splitface = TopoDS::Face(it.Value());
            gp_Pnt p = GetCentralFacePoint(splitface);

            // check if point is in shapeToExclude
            BRepClass3d_SolidClassifier classifier;
            classifier.Load(shapeToExInclude);
            classifier.Perform(p, Precision::Confusion());


            switch (op) {
            case EXCLUDE:
                if (classifier.State() != TopAbs_IN && classifier.State() != TopAbs_ON) {
                    compoundmaker.Add(compound, splitface);
                }
                break;
            case INCLUDE:
                if (classifier.State() == TopAbs_IN)
                    compoundmaker.Add(compound, splitface);
                break;
            default:
                printf("illegal operation\n");
            }
        }
    }

    // add remaining faces of original shape
    TopTools_IndexedMapOfShape splitMap;
    TopExp::MapShapes(splittedShape,   TopAbs_FACE, splitMap);
    for(int i = 1; i <= splitMap.Extent(); ++i) {
        const TopoDS_Face& face = TopoDS::Face(splitMap(i));
        // check if faces is from original shape
        Standard_Integer index = originMap.FindIndex(face);
        if(index > 0) {
            const TopoDS_Face& originalFace = TopoDS::Face(originMap.FindKey(index));
            gp_Pnt p = GetCentralFacePoint(originalFace);

            // check if point is in shapeToExclude
            BRepClass3d_SolidClassifier classifier;
            classifier.Load(shapeToExInclude);
            classifier.Perform(p, Precision::Confusion());


            switch (op) {
            case EXCLUDE:
                if (classifier.State() != TopAbs_IN && classifier.State() != TopAbs_ON) {
                    compoundmaker.Add(compound, originalFace);
                }
                break;
            case INCLUDE:
                if (classifier.State() == TopAbs_IN)
                    compoundmaker.Add(compound, originalFace);
                break;
            default:
                printf("illegal operation\n");
            }
        }
    }


    return compound;
}
}

CTrimShape::CTrimShape(const CNamedShape &shape, const CNamedShape &trimmingTool, TrimOperation op)
    : _dsfiller(NULL), _source(shape), _tool(trimmingTool), _operation(op)
{
    _fillerAllocated = false;
    _hasPerformed = false;
}

CTrimShape::CTrimShape(const CNamedShape &shape, const CNamedShape &trimmingTool, const BOPAlgo_PaveFiller & filler, TrimOperation op)
    : _source(shape), _tool(trimmingTool), _operation(op)
{
    _fillerAllocated = false;
    _hasPerformed = false;
    _dsfiller = (BOPAlgo_PaveFiller*) &filler;
}

CTrimShape::~CTrimShape()
{
    if(_fillerAllocated && _dsfiller) {
        delete _dsfiller;
        _dsfiller = NULL;
    }
}

CTrimShape::operator CNamedShape()
{
    return NamedShape();
}

void CTrimShape::PrepareFiller(){
    if(!_dsfiller) {
        BOPCol_ListOfShape aLS;
        aLS.Append(_tool.Shape());
        aLS.Append(_source.Shape());

        _dsfiller = new BOPAlgo_PaveFiller;
        _fillerAllocated = true;

        _dsfiller->SetArguments(aLS);
        _dsfiller->Perform();
    }
}

void CTrimShape::Perform()
{
    if(!_hasPerformed) {
        PrepareFiller();
        GEOMAlgo_Splitter splitter;
        BOPBuilderShapeToBRepBuilderShapeAdapter splitAdapter(splitter);
        splitter.AddArgument(_source.Shape());
        splitter.AddTool(_tool.Shape());
        splitter.PerformWithFiller(*_dsfiller);

        TopoDS_Shape trimmedShape = GetFacesNotInShape(splitAdapter, _source.Shape(), splitter.Shape(), _tool.Shape(), _operation);
        _resultshape = CNamedShape(trimmedShape, _source.Name());
        CBooleanOperTools::MapFaceNamesAfterBOP(splitAdapter, _source, _resultshape);
        CBooleanOperTools::MapFaceNamesAfterBOP(splitAdapter, _tool,   _resultshape);

        _hasPerformed = true;
    }
}

const CNamedShape &CTrimShape::NamedShape()
{
    Perform();
    return _resultshape;
}


