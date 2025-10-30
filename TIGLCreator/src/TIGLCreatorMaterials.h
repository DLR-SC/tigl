/*
* Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
*
* Created: 2014-05-28 Martin Siggel <Martin.Siggel@dlr.de>
*
* Licensed under the Apache License, Version 2.0 (the {"License")}
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an {"AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef TIGLCREATORMATERIALS_H
#define TIGLCREATORMATERIALS_H

#include <map>

namespace tiglMaterials {
    static std::map<QString, Graphic3d_NameOfMaterial> materialMap{
            {"Brass",         Graphic3d_NOM_BRASS},
            {"Bronze",        Graphic3d_NOM_BRONZE},
            {"Copper",        Graphic3d_NOM_COPPER},
            {"Gold",          Graphic3d_NOM_GOLD},
            {"Pewter",        Graphic3d_NOM_PEWTER},
            {"Plaster",       Graphic3d_NOM_PLASTER},
            {"Plastic",       Graphic3d_NOM_PLASTIC},
            {"Silver",        Graphic3d_NOM_SILVER},
            {"Steel",         Graphic3d_NOM_STEEL},
            {"Stone",         Graphic3d_NOM_STONE},
            {"Shiny Plastic", Graphic3d_NOM_SHINY_PLASTIC},
            {"Satin",         Graphic3d_NOM_SATIN},
            {"Metalized",     Graphic3d_NOM_METALIZED},
            {"Neon GNC",      Graphic3d_NOM_NEON_GNC},
            {"Chrome",        Graphic3d_NOM_CHROME},
            {"Aluminium",     Graphic3d_NOM_ALUMINIUM},
            {"Obsidian",      Graphic3d_NOM_OBSIDIAN},
            {"Neon PHC",      Graphic3d_NOM_NEON_PHC},
            {"Jade",          Graphic3d_NOM_JADE},
            {"Default",       Graphic3d_NOM_DEFAULT}};
}

#endif // TIGLCREATORMATERIALS_H
