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
* This file is automatically created from tigl.h on 2014-10-21.
* If you experience any bugs please contact the authors
*/

package de.dlr.sc.tigl;

import java.util.ArrayList;

public enum TiglLogLevel {
    TILOG_SILENT(0),
    TILOG_ERROR(1),
    TILOG_WARNING(2),
    TILOG_INFO(3),
    TILOG_DEBUG(4),
    TILOG_DEBUG1(5),
    TILOG_DEBUG2(6),
    TILOG_DEBUG3(7),
    TILOG_DEBUG4(8);

    private static ArrayList<TiglLogLevel> codes = new ArrayList<>();

    static {
        codes.add(TILOG_SILENT);
        codes.add(TILOG_ERROR);
        codes.add(TILOG_WARNING);
        codes.add(TILOG_INFO);
        codes.add(TILOG_DEBUG);
        codes.add(TILOG_DEBUG1);
        codes.add(TILOG_DEBUG2);
        codes.add(TILOG_DEBUG3);
        codes.add(TILOG_DEBUG4);
    }

    private final int code;

    private TiglLogLevel(final int value) {
         code = value;
    }

    public static TiglLogLevel getEnum(final int value) {
        return codes.get(Integer.valueOf(value));
    }

    public int getValue() {
        return code;
    }
};