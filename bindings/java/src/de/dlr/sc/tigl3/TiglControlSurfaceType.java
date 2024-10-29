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
* This file is automatically created from tigl.h on 2020-04-27.
* If you experience any bugs please contact the authors
*/

package de.dlr.sc.tigl3;

import java.util.ArrayList;

public enum TiglControlSurfaceType {
    TRAILING_EDGE_DEVICE(0),
    LEADING_EDGE_DEVICE(1),
    SPOILER(2);

    private static ArrayList<TiglControlSurfaceType> codes = new ArrayList<>();

    static {
        codes.add(TRAILING_EDGE_DEVICE);
        codes.add(LEADING_EDGE_DEVICE);
        codes.add(SPOILER);
    }

    private final int code;

    private TiglControlSurfaceType(final int value) {
         code = value;
    }

    public static TiglControlSurfaceType getEnum(final int value) {
        return codes.get(Integer.valueOf(value));
    }

    public int getValue() {
        return code;
    }
};