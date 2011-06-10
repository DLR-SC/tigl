/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id: TIGLInterfaceStatusCodes.java 3 2011-02-09 14:50:00Z markus.litz $ 
*
* Version: $Revision: 3 $
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
 
package de.dlr.sc.chameleon.rce.tigl;

import java.util.Hashtable;
import java.util.Map;


/**
 * 
 * Enum for the TIGL Return Codes.
 *
 * @version $LastChangedRevision: 1 $
 * @author Markus Litz
 */
public enum TIGLInterfaceStatusCodes {

    /**
     * The TIGL status codes - 2009-07-16.
     */
    TIGL_SUCCESS(0),                         /** !< No error occurred */
    TIGL_ERROR(1),                           /** !< Unspecified error */
    TIGL_NULL_POINTER(2),                    /** !< Null pointer argument */
    TIGL_NOT_FOUND(3),                       /** !< Configuration not found */
    TIGL_XML_ERROR(4),                       /** !< XML Error */
    TIGL_OPEN_FAILED(5),                     /** !< Open does fail */
    TIGL_CLOSE_FAILED(6),                    /** !< TIGL could not close configuration */
    TIGL_INDEX_ERROR(7),                     /** !< No or wrong Index */
    TIGL_STRING_TRUNCATED(8),                /** !< String truncated */
    TIGL_WRONG_TIXI_VERSION(9);              /** !< The TIXI Version does not fit to this tigl */

    
    private static final Map<Integer, TIGLInterfaceStatusCodes> ERRORCODES = new Hashtable<Integer, TIGLInterfaceStatusCodes>();

    static {    // quick access map statically initialized AFTER implicit enum constructor (!)
        ERRORCODES.put(Integer.valueOf(0), TIGL_SUCCESS);
        ERRORCODES.put(Integer.valueOf(1), TIGL_ERROR);
        ERRORCODES.put(Integer.valueOf(2), TIGL_NULL_POINTER);
        ERRORCODES.put(Integer.valueOf(3), TIGL_NOT_FOUND);
        ERRORCODES.put(Integer.valueOf(4), TIGL_XML_ERROR);
        ERRORCODES.put(Integer.valueOf(5), TIGL_OPEN_FAILED);
        ERRORCODES.put(Integer.valueOf(6), TIGL_CLOSE_FAILED);
        ERRORCODES.put(Integer.valueOf(7), TIGL_INDEX_ERROR);
        ERRORCODES.put(Integer.valueOf(8), TIGL_STRING_TRUNCATED);
        ERRORCODES.put(Integer.valueOf(9), TIGL_WRONG_TIXI_VERSION);
    }

    /**
     * Declaration of the enum property.
     */
    private final int errorCode;

    /**
     * Enum constructor.
     * @param returnValue The code to associate with the enum label
     */
    private TIGLInterfaceStatusCodes(final int returnValue) {
        errorCode = returnValue;
    }

    /**
     * Accessor for the enums .getError() method.
     * @param eCode the TIGL error code
     * @return The tigl errorcode enum
     */
    public static TIGLInterfaceStatusCodes getError(final int eCode) {
        return ERRORCODES.get(Integer.valueOf(eCode));
    }
    
    /**
     * To suppress warning.
     * @return The integer code
     */
    public int getCode() {
        return errorCode;
    }

}
