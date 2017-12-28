/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-03-19 Volker Poddey <Volker.Poddey@dlr.de>
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

#ifndef CTIGLEXPORTCOLLADA_H
#define CTIGLEXPORTCOLLADA_H

#include "tigl.h"
#include "tigl_internal.h"
#include "PNamedShape.h"
#include "ListPNamedShape.h"
#include "CTiglCADExporter.h"

#include <string>


namespace tigl 
{

class CTiglExportCollada : public CTiglCADExporter
{
public:
    TIGL_EXPORT CTiglExportCollada();

private:
    bool WriteImpl(const std::string& filename) const OVERRIDE;

    /// Exports a polygon object to a collada file, the true export code
    TiglReturnCode writeToDisc(class CTiglPolyData &polyData, const char * id, const char * filename);
};


} // end namespace tigl

#endif // CTIGLEXPORTCOLLADA_H
