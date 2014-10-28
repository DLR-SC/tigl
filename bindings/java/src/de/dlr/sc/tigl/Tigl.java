/* 
 * Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
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

package de.dlr.sc.tigl;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.ptr.IntByReference;

/**
 * This class the high level access class to the TiGL library.

 */
public class Tigl {
	
    /**
     * Central logger instance.
     */
    protected static final Log LOGGER = LogFactory.getLog(Tigl.class);

    
    /**
     * 
     * Returns the Version number of TIGL as a Java String.
     * @return a String containing the version number of TIGL.
     */
    public static String getVersion() {
        final String version = TiglNativeInterface.tiglGetVersion();
        return version;
    }
    
    
    /**
     * Imports a CPACS configuration from a String with TIGL.
     * 
     * @param cpacsString
     *            The full CPACS configuration to open with TIGL in one string.
     * @param configurationUID
     *               The UID of the configuration to open with TIGL         
     *         
     * @return true if success.
     * @throws TiglException 
     */
    public static CpacsConfiguration openCPACSConfigurationFromString(final String cpacsString, final String configurationUID) throws TiglException {
        CpacsConfiguration config = new CpacsConfiguration();
    	config.setConfigurationUID(configurationUID);

        // open cpacs file with TIXI
        IntByReference tmpTixiHandle = new IntByReference();
        if (TixiNativeInterface.tixiImportFromString(cpacsString, tmpTixiHandle) != 0) {
            LOGGER.error("tixiImportFromString failed in TIGLInterface::tiglOpenCPACSConfigurationFromString");
            throw new TiglException("openCPACSConfigurationFromString", TiglReturnCode.TIGL_OPEN_FAILED);
        }    
        config.setTixiHandle(tmpTixiHandle.getValue());

        IntByReference tiglHandleTemp = new IntByReference();
        // now open cpacs configuration with tigl        
        int error = TiglNativeInterface.tiglOpenCPACSConfiguration(tmpTixiHandle.getValue(), configurationUID, tiglHandleTemp); 
        if (error != TiglReturnCode.TIGL_SUCCESS.getValue()) {
        	throw new TiglException("openCPACSConfiguration", TiglReturnCode.getEnum(error));
        }
        config.setCPACSHandle(tiglHandleTemp.getValue());
        LOGGER.info("Cpacs configuration " + configurationUID + "opened succesfully");
        
        
        return config;
    }

    /**
     * 
     * Open a CPACS configuration with TIGL.
     * 
     * @param cpacsFileName
     *            The full path to a XML file to open with tixi.
     * @param configurationUID
     *               The UID of the configuration to open with TIGL     
     * @return true if success.
     * @throws TiglException 
     */
    public static CpacsConfiguration openCPACSConfiguration(final String cpacsFileName, final String configurationUID) throws TiglException {
        CpacsConfiguration config = new CpacsConfiguration();
    	config.setConfigurationUID(configurationUID);

        // open cpacs file with TIXI
        IntByReference tmpTixiHandle = new IntByReference();
        if (TixiNativeInterface.tixiOpenDocument(cpacsFileName, tmpTixiHandle) != 0) {
            LOGGER.error("tixiOpenDocument failed in TIGLInterface::tiglOpenCPACSConfigurationFromDisk");
            throw new TiglException("openCPACSConfiguration", TiglReturnCode.TIGL_OPEN_FAILED);
        }    
        config.setTixiHandle(tmpTixiHandle.getValue());

        IntByReference tiglHandleTemp = new IntByReference();
        // now open cpacs configuration with tigl        
        int error = TiglNativeInterface.tiglOpenCPACSConfiguration(tmpTixiHandle.getValue(), configurationUID, tiglHandleTemp); 
        if (error != TiglReturnCode.TIGL_SUCCESS.getValue()) {
        	throw new TiglException("openCPACSConfiguration", TiglReturnCode.getEnum(error));
        }
        config.setCPACSHandle(tiglHandleTemp.getValue());
        LOGGER.info("Cpacs configuration " + configurationUID + "opened succesfully");

        return config;
    }
    
}
