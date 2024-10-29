/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2014-10-21 Martin Siggel <martin.siggel@dlr.de>
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

/* 
* This file is automatically created from tigl.h on 2020-03-20.
* If you experience any bugs please contact the authors
*/

package de.dlr.sc.tigl3;

import java.util.ArrayList;

public enum TiglGeometricComponentType {
    TIGL_COMPONENT_PLANE(0),
    TIGL_COMPONENT_FUSELAGE(1),
    TIGL_COMPONENT_WING(2),
    TIGL_COMPONENT_SEGMENT(3),
    TIGL_COMPONENT_WINGSEGMENT(4),
    TIGL_COMPONENT_FUSELSEGMENT(5),
    TIGL_COMPONENT_WINGCOMPSEGMENT(6),
    TIGL_COMPONENT_WINGSHELL(7),
    TIGL_COMPONENT_WINGRIB(8),
    TIGL_COMPONENT_WINGSPAR(9),
    TIGL_COMPONENT_WINGCELL(10),
    TIGL_COMPONENT_GENERICSYSTEM(11),
    TIGL_COMPONENT_ROTOR(12),
    TIGL_COMPONENT_ROTORBLADE(13),
    TIGL_COMPONENT_ATTACHED_ROTORBLADE(14),
    TIGL_COMPONENT_PRESSURE_BULKHEAD(15),
    TIGL_COMPONENT_CROSS_BEAM_STRUT(16),
    TIGL_COMPONENT_CARGO_DOOR(17),
    TIGL_COMPONENT_LONG_FLOOR_BEAM(18),
    TIGL_COMPONENT_EXTERNAL_OBJECT(19),
    TIGL_COMPONENT_FARFIELD(20),
    TIGL_COMPONENT_ENGINE_PYLON(21),
    TIGL_COMPONENT_ENGINE_NACELLE(22),
    TIGL_COMPONENT_FUSELAGE_WALL(23),
    TIGL_COMPONENT_OTHER(24);

    private static ArrayList<TiglGeometricComponentType> codes = new ArrayList<>();

    static {
        codes.add(TIGL_COMPONENT_PLANE);
        codes.add(TIGL_COMPONENT_FUSELAGE);
        codes.add(TIGL_COMPONENT_WING);
        codes.add(TIGL_COMPONENT_SEGMENT);
        codes.add(TIGL_COMPONENT_WINGSEGMENT);
        codes.add(TIGL_COMPONENT_FUSELSEGMENT);
        codes.add(TIGL_COMPONENT_WINGCOMPSEGMENT);
        codes.add(TIGL_COMPONENT_WINGSHELL);
        codes.add(TIGL_COMPONENT_WINGRIB);
        codes.add(TIGL_COMPONENT_WINGSPAR);
        codes.add(TIGL_COMPONENT_WINGCELL);
        codes.add(TIGL_COMPONENT_GENERICSYSTEM);
        codes.add(TIGL_COMPONENT_ROTOR);
        codes.add(TIGL_COMPONENT_ROTORBLADE);
        codes.add(TIGL_COMPONENT_ATTACHED_ROTORBLADE);
        codes.add(TIGL_COMPONENT_PRESSURE_BULKHEAD);
        codes.add(TIGL_COMPONENT_CROSS_BEAM_STRUT);
        codes.add(TIGL_COMPONENT_CARGO_DOOR);
        codes.add(TIGL_COMPONENT_LONG_FLOOR_BEAM);
        codes.add(TIGL_COMPONENT_EXTERNAL_OBJECT);
        codes.add(TIGL_COMPONENT_FARFIELD);
        codes.add(TIGL_COMPONENT_ENGINE_PYLON);
        codes.add(TIGL_COMPONENT_ENGINE_NACELLE);
        codes.add(TIGL_COMPONENT_FUSELAGE_WALL);
        codes.add(TIGL_COMPONENT_OTHER);
    }

    private final int code;

    private TiglGeometricComponentType(final int value) {
         code = value;
    }

    public static TiglGeometricComponentType getEnum(final int value) {
        return codes.get(Integer.valueOf(value));
    }

    public int getValue() {
        return code;
    }
};