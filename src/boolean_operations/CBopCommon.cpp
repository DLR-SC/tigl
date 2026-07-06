/*
* Copyright (C) 2020 German Aerospace Center (DLR/SC)
*
* Created: 2015-06-15 Martin Siggel <Martin.Siggel@dlr.de>
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

#include "CBopCommon.h"

#include <cassert>

#include <BOPAlgo_PaveFiller.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <Standard_Version.hxx>
#include <Standard_Failure.hxx>
#include <Precision.hxx>

#include "CBooleanOperTools.h"
#include "CNamedShape.h"
#include "CTiglError.h"
#include "CTiglLogging.h"

CBopCommon::CBopCommon(const PNamedShape shape, const PNamedShape cuttingTool)
    :  _resultshape(), _tool(cuttingTool), _source(shape), _dsfiller(nullptr)
    , _fuzzyValue(Precision::Confusion())
{
    _fillerAllocated = false;
    _hasPerformed = false;
}

CBopCommon::CBopCommon(const PNamedShape shape, const PNamedShape cuttingTool, const BOPAlgo_PaveFiller & filler)
    :  _resultshape(), _tool(cuttingTool), _source(shape)
    , _fuzzyValue(Precision::Confusion())
{
    _fillerAllocated = false;
    _hasPerformed = false;
    _dsfiller = const_cast<BOPAlgo_PaveFiller*>(&filler);
}

CBopCommon::~CBopCommon()
{
    if (_fillerAllocated && _dsfiller) {
        delete _dsfiller;
        _dsfiller = nullptr;
    }
}

void CBopCommon::SetFuzzyValue(double fuzzyValue)
{
    _fuzzyValue = fuzzyValue;
}

CBopCommon::operator PNamedShape()
{
    return NamedShape();
}

void CBopCommon::PrepareFiller(double fuzzyValue)
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

void CBopCommon::Perform()
{
    if (!_hasPerformed) {
        if (!_tool || !_source) {
           _resultshape.reset();
            return;
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

            try {
                BRepAlgoAPI_Common commonTool(_source->Shape(), _tool->Shape(), *_dsfiller);

                TopoDS_Shape commonShape = commonTool.Shape();

#if OCC_VERSION_HEX >= VERSION_HEX_CODE(7,2,0)
                if (commonTool.HasErrors()) {
                    if (i < c_tries - 1) {
                        fuzzyValue *= 10;
                        LOG(WARNING) << "CBopCommon failed, retrying with fuzzyValue: " << fuzzyValue;
                        continue;
                    }

                    std::ostringstream oss;
                    commonTool.GetReport()->Dump(oss);
                    LOG(ERROR) << "CBopCommon failed after " << c_tries << " attempts: " << oss.str();
                    throw tigl::CTiglError("CBopCommon failed: " + oss.str());
                }
#elif OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
                if (commonTool.ErrorStatus() != 0) {
                    if (i < c_tries - 1) {
                        fuzzyValue *= 10;
                        LOG(WARNING) << "CBopCommon failed, retrying with fuzzyValue: " << fuzzyValue;
                        continue;
                    }

                    LOG(ERROR) << "CBopCommon failed after " << c_tries << " attempts!";
                    throw tigl::CTiglError("CBopCommon failed!");
                }
#endif

                if (commonShape.IsNull() || !BRepCheck_Analyzer(commonShape).IsValid()) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
                    if (i < c_tries - 1) {
                        fuzzyValue *= 10;
                        LOG(WARNING) << "CBopCommon produced invalid result, retrying with fuzzyValue: " << fuzzyValue;
                        continue;
                    }
#endif
                    LOG(ERROR) << "CBopCommon produced invalid result after all retries";
                    throw tigl::CTiglError("CBopCommon produced invalid result");
                }

                _resultshape = PNamedShape(new CNamedShape(commonShape, _source->Name()));
                CBooleanOperTools::MapFaceNamesAfterBOP(commonTool, _source, _resultshape);
                CBooleanOperTools::MapFaceNamesAfterBOP(commonTool, _tool,   _resultshape);
                _hasPerformed = true;
                return;
            }
            catch (const Standard_Failure& f) {
#if OCC_VERSION_HEX >= VERSION_HEX_CODE(6,9,0)
                if (i < c_tries - 1) {
                    fuzzyValue *= 10;
                    LOG(WARNING) << "CBopCommon threw exception, retrying with fuzzyValue: " << fuzzyValue << " — " << f.GetMessageString();
                    continue;
                }
#endif
                std::string msg = "CBopCommon exception: ";
                msg += f.GetMessageString();
                LOG(ERROR) << msg;
                throw tigl::CTiglError(msg);
            }
        }
    }
}

const PNamedShape CBopCommon::NamedShape()
{
    Perform();
    return _resultshape;
}