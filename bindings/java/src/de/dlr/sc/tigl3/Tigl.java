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

package de.dlr.sc.tigl3;

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
            LOGGER.error("TiXI: tixiImportFromString failed in TIGLInterface::tiglOpenCPACSConfigurationFromString");
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
        LOGGER.info("TiGL: Cpacs configuration " + configurationUID + "opened succesfully");
        
        
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
        LOGGER.info("TiGL: Cpacs configuration " + configurationUID + "opened succesfully");

        return config;
    }
    
    /**
     * Sets up the tigl logging mechanism to send all log messages into a file.
     * Typically this function has to be called before opening any cpacs configuration.
     *  
     * @param filePrefix - Prefix of the filename to be created. The filename consists 
     *                     of the prefix, a date and time string and the ending ".log".
     * @throws TiglException
     */
    public void logToFileEnabled(final String filePrefix) throws TiglException {
        int error = TiglNativeInterface.tiglLogToFileEnabled(filePrefix);
        if (error != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            throw new TiglException("tiglLogToFileEnabled", TiglReturnCode.getEnum(error));
        }
        LOGGER.info("TiGL: Enabled logging into file!");
    }
    
    /**
     * Disables the file logging mechanism
     * @throws TiglException
     */
    public void logToFileDisabled() throws TiglException {
        int error = TiglNativeInterface.tiglLogToFileDisabled();
        if (error != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            throw new TiglException("tiglLogToFileDisabled", TiglReturnCode.getEnum(error));
        }
        LOGGER.info("TiGL: Disabled logging into file!");
    }
    
    /**
     * Sets the file suffix of the log file. Default is "log".
     *  
     * @param suffix The file suffix.
     * @throws TiglException
     */
    public void logSetFileEnding(final String suffix) throws TiglException {
        int error = TiglNativeInterface.tiglLogSetFileEnding(suffix);
        if (error != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            throw new TiglException("tiglLogSetFileEnding", TiglReturnCode.getEnum(error));
        }
    }
    
    /**
     * Enables or disables appending a unique date/time identifier inside the log file name 
     * (behind the file prefix). By default, the time identifier is enabled.
     *
     * This function has to be called before tiglLogToFileEnabled to have the desired effect.
     * 
     * @param enabled - True if time-code should be used for the log file name
     * @throws TiglException
     */
    public void logSetTimeInFilenameEnabled(final boolean enabled) throws TiglException {
        int error = TiglNativeInterface.tiglLogSetTimeInFilenameEnabled(enabled ? 1 : 0);
        if (error != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            throw new TiglException("tiglLogSetTimeInFilenameEnabled", TiglReturnCode.getEnum(error));
        }
    }

    /**
     * Set the console verbosity level.
     *
     * This function shall be used change, what kind of logging information is displayed
     * on the console. By default, only errors and warnings are printed on console
     * 
     * @param level - Maximum verbosity level for log messages. 
     * @throws TiglException
     */
    public void logSetVerbosity(final TiglLogLevel level) throws TiglException {
        int error = TiglNativeInterface.tiglLogSetVerbosity(level.getValue());
        if (error != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            throw new TiglException("tiglLogSetVerbosity", TiglReturnCode.getEnum(error));
        }
    }
    
    /**
     * Returns the string associated with a TiGL error code
     * 
     * @param errorCode - The TiGL error code.
     * 
     * @return String describing the error.
     * @throws TiglException
     */
    public String getErrorString(final TiglReturnCode errorCode) throws TiglException {
        return TiglNativeInterface.tiglGetErrorString(errorCode.getValue());
    }
    
    public static class WingCoordinates {
        public WingCoordinates(double eta, double xsi) {
            this.eta = eta;
            this.xsi = xsi;
        }
        
        public double eta;
        public double xsi;
    }
    
    public static class WGetPointDirectionResult {
        public WGetPointDirectionResult(TiglPoint p , double errorDistance) {
            this.point = p;
            this.errorDistance = errorDistance;
        }
        
        public TiglPoint point;
        public double errorDistance;
    }
    
    public static class GetSegmentIndexResult {
        public GetSegmentIndexResult(int parentIndex , int segmentIndex) {
            this.parentIndex = parentIndex;
            this.segmentIndex = segmentIndex;
        }
        
        //Index of the wing or the fuselage
        public int parentIndex;
        public int segmentIndex;
    }
    
    public static class WSProjectionResult {
        public WSProjectionResult(WingCoordinates c, boolean isOnTop, int segmentIndex) {
            this.point = c;
            this.isOnTop = isOnTop;
            this.segmentIndex = segmentIndex;
        }
        
        public boolean isOnTop;
        public int segmentIndex;
        public WingCoordinates point;
    }
    
    public static class WCSFindSegmentResult {
        public WCSFindSegmentResult(String winguid, String segmentuid) {
            this.wingUID = winguid;
            this.segmentUID = segmentuid;
        }
        
        public String wingUID, segmentUID;
    }
    
    public static class WCSGetSegmentEtaXsiResult {
        public WCSGetSegmentEtaXsiResult(String wingUID, String segmentUID, double eta, double xsi, double errorDistance) {
            this.wingUID = wingUID;
            this.segmentUID = segmentUID;
            this.eta = eta;
            this.xsi = xsi;
            this.errorDistance = errorDistance;
        }
        
        public String wingUID, segmentUID;
        public double  eta, xsi, errorDistance;
    }
    
    public static class SurfaceMaterial {
        public SurfaceMaterial(String materialUID, double thickness) {
            this.materialUID = materialUID;
            this.thickness = thickness;
        }
        
        public String materialUID = "";
        public double thickness = 0 ;
    }
    
    public static class SectionAndElementIndex {
        public SectionAndElementIndex(int sectionIndex, int elementIndex) {
            this.sectionIndex = sectionIndex;
            this.elementIndex = elementIndex;
        }
        
        public int sectionIndex = 0;
        public int elementIndex = 0;
    }
    
    public static class SectionAndElementUID {
        public SectionAndElementUID(String sectionUID, String elementUID) {
            this.sectionUID = sectionUID;
            this.elementUID = elementUID;
        }
        
        public String sectionUID;
        public String elementUID;
    }
    
    /**
     * Helper class to store the mean aerodynamic chord
     * and position
     */
    public static class WingMAC {
        public WingMAC(double mac, TiglPoint point) {
            this.mac = mac;
            this.macPoint = point;
        }
        
        public TiglPoint macPoint;
        public double mac;
    }
    
    public static class SegmentXsiAndWarning {
        public SegmentXsiAndWarning(double segmentXsi, boolean hasWarning) {
            this.segmentXsi = segmentXsi;
            this.hasWarning = hasWarning;
        }
        
        public double segmentXsi;
        public boolean hasWarning; 
    }
}
