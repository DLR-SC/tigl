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
* This file is automatically created from tigl.h on 2015-06-01.
* If you experience any bugs please contact the authors
*/

package de.dlr.sc.tigl;

import java.util.ArrayList;

public enum TiglReturnCode {
    TIGL_SUCCESS(0),
    TIGL_ERROR(1),
    TIGL_NULL_POINTER(2),
    TIGL_NOT_FOUND(3),
    TIGL_XML_ERROR(4),
    TIGL_OPEN_FAILED(5),
    TIGL_CLOSE_FAILED(6),
    TIGL_INDEX_ERROR(7),
    TIGL_STRING_TRUNCATED(8),
    TIGL_WRONG_TIXI_VERSION(9),
    TIGL_UID_ERROR(10),
    TIGL_WRONG_CPACS_VERSION(11),
    TIGL_UNINITIALIZED(12),
    TIGL_MATH_ERROR(13),
    TIGL_WRITE_FAILED(14);

    private static ArrayList<TiglReturnCode> codes = new ArrayList<>();

    static {
        codes.add(TIGL_SUCCESS);
        codes.add(TIGL_ERROR);
        codes.add(TIGL_NULL_POINTER);
        codes.add(TIGL_NOT_FOUND);
        codes.add(TIGL_XML_ERROR);
        codes.add(TIGL_OPEN_FAILED);
        codes.add(TIGL_CLOSE_FAILED);
        codes.add(TIGL_INDEX_ERROR);
        codes.add(TIGL_STRING_TRUNCATED);
        codes.add(TIGL_WRONG_TIXI_VERSION);
        codes.add(TIGL_UID_ERROR);
        codes.add(TIGL_WRONG_CPACS_VERSION);
        codes.add(TIGL_UNINITIALIZED);
        codes.add(TIGL_MATH_ERROR);
        codes.add(TIGL_WRITE_FAILED);
    }

    private final int code;

    private TiglReturnCode(final int value) {
         code = value;
    }

    public static TiglReturnCode getEnum(final int value) {
        return codes.get(Integer.valueOf(value));
    }

    public int getValue() {
        return code;
    }
};