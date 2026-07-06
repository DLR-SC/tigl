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
#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CNamedShape.h"

#include <cassert>
#include <sstream>
#include <string>

#include <BOPAlgo_PaveFiller.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <Standard_Version.hxx>
#include <Standard_Failure.hxx>
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
#include <BRepBuilderAPI_MakeVertex.hxx>
#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,3,0)
#include <BOPCol_ListOfShape.hxx>
#endif

namespace
{

    static unsigned int itrim = 0;


    // Writes shape and its central face points into brep file (for debugging purposes)
    void WriteDebugShape(const TopoDS_Shape& shape, const char * name)
    {
        TopoDS_Compound c;
        BRep_Builder b;
        b.MakeCompound(c);

        b.Add(c, shape);

        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(shape,   TopAbs_FACE, map);
        for (int i = 1; i <= map.Extent(); ++i) {
            const TopoDS_Face& face = TopoDS::Face(map(i));
            gp_Pnt p = GetCentralFacePoint(face);
            b.Add(c, BRepBuilderAPI_MakeVertex(p));
        }

        std::stringstream str;
        str << "trim_" << itrim << "_" << name << ".brep";

        BRepTools::Write(c, str.str().c_str());
    }

    TopoDS_Shape GetFacesNotInShape(BRepBuilderAPI_MakeShape& bop, const TopoDS_Shape& originalShape, const TopoDS_Shape& splittedShape, const TopoDS_Shape& shapeToExInclude, TrimOperation op)
    {

        TopoDS_Compound compound;
        BRep_Builder compoundmaker;
        compoundmaker.MakeCompound(compound);

        // add splitted faces to compound
        TopTools_IndexedMapOfShape originMap;
        TopExp::MapShapes(originalShape,   TopAbs_FACE, originMap);
        for (int i = 1; i <= originMap.Extent(); ++i) {
            const TopoDS_Face& face = TopoDS::Face(originMap(i));
            const TopTools_ListOfShape& splits = bop.Modified(face);
            TopTools_ListIteratorOfListOfShape it;
            for (it.Initialize(splits); it.More(); it.Next()) {
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
                    if (classifier.State() == TopAbs_IN) {
                        compoundmaker.Add(compound, splitface);
                    }
                    break;
                default:
                    printf("illegal operation\n");
                }
            }
        }

        // add remaining faces of original shape
        TopTools_IndexedMapOfShape splitMap;
        TopExp::MapShapes(splittedShape,   TopAbs_FACE, splitMap);
        for (int i = 1; i <= splitMap.Extent(); ++i) {
            const TopoDS_Face& face = TopoDS::Face(splitMap(i));
            // check if faces is from original shape
            Standard_Integer index = originMap.FindIndex(face);
            if (index > 0) {
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
                    if (classifier.State() == TopAbs_IN) {
                        compoundmaker.Add(compound, originalFace);
                    }
                    break;
                default:
                    printf("illegal operation\n");
                }
            }
        }


        return compound;
    }
} // namespace

CTrimShape::CTrimShape(const PNamedShape shape, const PNamedShape trimmingTool, TrimOperation op)
    : _operation(op), _resultshape(), _tool(trimmingTool), _source(shape), _dsfiller(NULL)
    , _fuzzyValue(Precision::Confusion())
{
    _fillerAllocated = false;
    _hasPerformed = false;
}

CTrimShape::CTrimShape(const PNamedShape shape, const PNamedShape trimmingTool, const BOPAlgo_PaveFiller & filler, TrimOperation op)
    : _operation(op), _resultshape(), _tool(trimmingTool), _source(shape)
    , _fuzzyValue(Precision::Confusion())
{
    _fillerAllocated = false;
    _hasPerformed = false;
    _dsfiller = (BOPAlgo_PaveFiller*) &filler;
}

void CTrimShape::SetFuzzyValue(double fuzzyValue)
{
    _fuzzyValue = fuzzyValue;
}

CTrimShape::~CTrimShape()
{
    if (_fillerAllocated && _dsfiller) {
        delete _dsfiller;
        _dsfiller = NULL;
    }
}

CTrimShape::operator PNamedShape()
{
    return NamedShape();
}

void CTrimShape::PrepareFiller(double fuzzyValue)
{
    if (!_tool || !_source) {
        return;
    }

    if (_dsfiller) {
        if (_fillerAllocated) {
            delete _dsfiller;
            _dsfiller = nullptr;
        }
        _dsfiller = nullptr;
    }

#if OCC_VERSION_HEX < VERSION_HEX_CODE(7,3,0)
    BOPCol_ListOfShape aLS;
#else
    TopTools_ListOfShape aLS;
#endif
    aLS.Append(_tool->Shape());
    aLS.Append(_source->Shape());

    _dsfiller = new BOPAlgo_PaveFiller;
    _fillerAllocated = true;

    _dsfiller->SetArguments(aLS);
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
    _dsfiller->SetFuzzyValue(fuzzyValue);
#endif
    _dsfiller->Perform();
}

void CTrimShape::Perform()
{
    if (!_hasPerformed) {
        if (!_source) {
            throw tigl::CTiglError("Null pointer for source argument in CTrimShape", TIGL_NULL_POINTER);
        }

        if (!_tool) {
            throw tigl::CTiglError("Null pointer for tool argument in CTrimShape", TIGL_NULL_POINTER);
        }

        bool debug = (getenv("TIGL_DEBUG_BOP") != NULL);

        if (debug) {
            WriteDebugShape(_source->Shape(), "source");
            WriteDebugShape(_tool->Shape(), "tool");
        }

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
        const int c_tries = 3;
        double fuzzyValue = _fuzzyValue;
#endif

        for (int i = 0;; i++) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
            PrepareFiller(fuzzyValue);
#else
            PrepareFiller(_fuzzyValue);
#endif

            GEOMAlgo_Splitter splitter;
            BOPBuilderShapeToBRepBuilderShapeAdapter splitAdapter(splitter);
            splitter.AddArgument(_source->Shape());
            splitter.AddTool(_tool->Shape());
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
            splitter.SetFuzzyValue(fuzzyValue);
#endif

            try {
                splitter.PerformWithFiller(*_dsfiller);

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,2,0)
                if (splitter.HasErrors()) {
                    if (i < c_tries - 1) {
                        fuzzyValue *= 10;
                        LOG(WARNING) << "CTrimShape splitter failed, retrying with fuzzyValue: " << fuzzyValue;
                        continue;
                    }

                    std::ostringstream oss;
                    splitter.GetReport()->Dump(oss);
                    LOG(ERROR) << "CTrimShape splitter failed after " << c_tries << " attempts: " << oss.str();
                    throw tigl::CTiglError("CTrimShape splitter failed: " + oss.str());
                }
#elif OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
                if (splitter.ErrorStatus() != 0) {
                    if (i < c_tries - 1) {
                        fuzzyValue *= 10;
                        LOG(WARNING) << "CTrimShape splitter failed, retrying with fuzzyValue: " << fuzzyValue;
                        continue;
                    }

                    LOG(ERROR) << "CTrimShape splitter failed after " << c_tries << " attempts!";
                    throw tigl::CTiglError("CTrimShape splitter failed!");
                }
#endif

                if (debug) {
                    WriteDebugShape(splitter.Shape(), "split");
                }

                TopoDS_Shape trimmedShape = GetFacesNotInShape(splitAdapter, _source->Shape(), splitter.Shape(), _tool->Shape(), _operation);

                if (trimmedShape.IsNull() || !BRepCheck_Analyzer(trimmedShape).IsValid()) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
                    if (i < c_tries - 1) {
                        fuzzyValue *= 10;
                        LOG(WARNING) << "CTrimShape produced invalid result, retrying with fuzzyValue: " << fuzzyValue;
                        continue;
                    }
#endif
                    LOG(ERROR) << "CTrimShape produced invalid result after all retries";
                    throw tigl::CTiglError("CTrimShape produced invalid result");
                }

                _resultshape = PNamedShape(new CNamedShape(trimmedShape, _source->Name()));
                CBooleanOperTools::MapFaceNamesAfterBOP(splitAdapter, _source, _resultshape);
                CBooleanOperTools::MapFaceNamesAfterBOP(splitAdapter, _tool,   _resultshape);

                _resultshape = CBooleanOperTools::Shellify(_resultshape);

                if (debug) {
                    WriteDebugShape(_resultshape->Shape(), "result");
                }

                itrim++;
                _hasPerformed = true;
                return;
            }
            catch (const Standard_Failure& f) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
                if (i < c_tries - 1) {
                    fuzzyValue *= 10;
                    LOG(WARNING) << "CTrimShape threw exception, retrying with fuzzyValue: " << fuzzyValue << " — " << f.GetMessageString();
                    continue;
                }
#endif
                std::string msg = "CTrimShape exception: ";
                msg += f.GetMessageString();
                LOG(ERROR) << msg;
                throw tigl::CTiglError(msg);
            }
        }
    }
}

const PNamedShape CTrimShape::NamedShape()
{
    Perform();
    return _resultshape;
}


