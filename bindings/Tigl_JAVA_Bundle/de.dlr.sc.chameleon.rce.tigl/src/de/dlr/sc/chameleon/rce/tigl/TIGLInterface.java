/* 
* Copyright (C) 2007-2011 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$ 
*
* Version: $Revision$
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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Native;
import com.sun.jna.ptr.DoubleByReference;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.win32.StdCallLibrary;

import de.dlr.sc.chameleon.rce.tixi.TIXIInterface;


/**
 * This class is a JNA-Wrapper around the TIGL library.
 *
 * @version $LastChangedRevision$
 * @author Markus Litz
 */
public class TIGLInterface {
    
    /**
     * Central logger instance.
     */
    protected static final Log LOGGER = LogFactory.getLog(TIGLInterface.class);
    
    /**
     * Little work around to call tixi loadLibrary before tigl loadLibrary.
     */
    private static final TIXIInterface TIXI = new TIXIInterface();
    
    static {
    	System.loadLibrary("msvcp71d");
		System.loadLibrary("tk84");
		System.loadLibrary("TKernel");
		System.loadLibrary("PTKernel");
		System.loadLibrary("TKAdvTools");
		System.loadLibrary("TKWOK");
		System.loadLibrary("wokdfltsteps");
		System.loadLibrary("wokdeliverysteps");
		System.loadLibrary("TKIDLFront");
		System.loadLibrary("TKCPPClient");
		System.loadLibrary("wokobjssteps");
		System.loadLibrary("wokorbixsteps");
		System.loadLibrary("TKCDLFront");
		System.loadLibrary("TKCSFDBSchema");
		System.loadLibrary("TKTCPPExt");
		System.loadLibrary("woksteps");
		System.loadLibrary("TKMath");
		System.loadLibrary("TKG2d");
		System.loadLibrary("TKCPPIntExt");
		System.loadLibrary("TKjcas");
		System.loadLibrary("TKG3d");
		System.loadLibrary("TKGeomBase");
		System.loadLibrary("TKBRep");
		System.loadLibrary("TKPShape");
		System.loadLibrary("TKGeomAlgo");
		System.loadLibrary("TKTopAlgo");
		System.loadLibrary("TKHLR");
		System.loadLibrary("TKBO");
		System.loadLibrary("TKMesh");
		System.loadLibrary("TKSTL");
		System.loadLibrary("TKXMesh");
		System.loadLibrary("TKShHealing");
		System.loadLibrary("TKXSBase");
		System.loadLibrary("TKSTEPBase");
		System.loadLibrary("TKSTEP209");
		System.loadLibrary("TKPrim");
		System.loadLibrary("TKBool");
		System.loadLibrary("TKOffset");
		System.loadLibrary("TKFeat");
		System.loadLibrary("TKIGES");
		System.loadLibrary("TKFillet");
		System.loadLibrary("TKSTEPAttr");
		System.loadLibrary("TKSTEP");
		System.loadLibrary("TKCPPJini");
		System.loadLibrary("TKCPPExt");
		System.loadLibrary("TKCDF");
		System.loadLibrary("TKShapeSchema");
		System.loadLibrary("TKLCAF");
		System.loadLibrary("TKTObj");
		System.loadLibrary("TKBinL");
		System.loadLibrary("TKPLCAF");
		System.loadLibrary("TKStdLSchema");
		System.loadLibrary("BinLPlugin");
		System.loadLibrary("FWOSPlugin");
		System.loadLibrary("TKXmlL");
		System.loadLibrary("XmlLPlugin");
		System.loadLibrary("TKXmlTObj");
		System.loadLibrary("tcl84");
		System.loadLibrary("TKDraw");
		System.loadLibrary("TKWOKTcl");
		System.loadLibrary("wokutilscmd");
		System.loadLibrary("woktoolscmd");
		System.loadLibrary("wokcmd");
		System.loadLibrary("mscmd");
		System.loadLibrary("StdLPlugin");
		System.loadLibrary("TKService");
		System.loadLibrary("TKV2d");
		System.loadLibrary("TKV3d");
		System.loadLibrary("TKVRML");
		System.loadLibrary("TKTopTest");
		System.loadLibrary("TKMeshVS");
		System.loadLibrary("TKCAF");
		System.loadLibrary("TKBin");
		System.loadLibrary("BinPlugin");
		System.loadLibrary("TKXml");
		System.loadLibrary("XmlPlugin");
		System.loadLibrary("TKNIS");
		System.loadLibrary("TKOpenGl");
		System.loadLibrary("TKViewerTest");
		System.loadLibrary("TKXSDRAW");
		System.loadLibrary("TKDCAF");
		System.loadLibrary("TKTObjDRAW");
		System.loadLibrary("TKXCAF");
		System.loadLibrary("TKXmlXCAF");
		System.loadLibrary("TKBinXCAF");
		System.loadLibrary("BinXCAFPlugin");
		System.loadLibrary("TKXDESTEP");
		System.loadLibrary("TKXDEIGES");
		System.loadLibrary("TKXDEDRAW");
		System.loadLibrary("TKPCAF");
		System.loadLibrary("TKStdSchema");
		System.loadLibrary("TKXCAFSchema");
		System.loadLibrary("XCAFPlugin");
		System.loadLibrary("StdPlugin");
		System.loadLibrary("XmlXCAFPlugin");
		System.loadLibrary("TKBinTObj");
		System.loadLibrary("TIGL");
		LOGGER.debug("TIGL Libraries loaded");
	}

    /** The internal TIXI Interface. */
    public TIXIInterface tixi = null;
    
    /** Internal CPACS handler. */
    private IntByReference tiglHandle = null;
    
    /** Internal tixi handler. */
    private IntByReference tixiHandle = null;
    
    /** The return value from TIGL. */
    private int errorCode = 0;
    
    /** UID of the CPACS configuration. */ 
    private String configUID;
    
    
    /**
     * Constructor.
     */
    public TIGLInterface() {
        // nothing special to initialize
    } 
    
        
    /**
     * Exports a CPACS Geometry in IGES format to a local file. 
     * 
     * @param exportFileName
     *            The full filename of the file to be exported.
     * @return true if success.
     */    
    public boolean tiglExportNonFusedIGES(final String exportFileName) { 
        if (!checkTiglConfiguration()) {
            return false;
        }
                
        // export to the file
        errorCode = TIGL.INSTANCE.tiglExportIGES(tiglHandle.getValue(), exportFileName); 
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglExportNonFusedIGES");
            return false;
        }          
        return true;
    }
    
    /**
     * Exports a CPACS wing geometry in VTK format to a local file.
     * Uses Index to specify the wing. 
     * 
     * @param wingIndex - The index of the wing to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @return true if success.
     */
    public boolean tiglExportMeshedWingVTK(final int wingIndex, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }
        
        // export to the file
        errorCode = TIGL.INSTANCE.tiglExportMeshedWingVTKByIndex(tiglHandle.getValue(), wingIndex, fileName, deflection); 
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglExportMeshedWingVTKByIndex");
            return false;
        }        
        return true;
    }
    
    /**
     * Exports a wing of a CPACS Geometry (stored in a string) in IGES format to a local file.
     * Uses simple and fast method to export quick results!
     * 
     * @param wingUID - The UID of the wing to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @return true if success.
     */
    public boolean tiglExportMeshedWingVTK(final String wingUID, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }
        
        // export to the file
        errorCode = TIGL.INSTANCE.tiglExportMeshedWingVTKSimpleByUID(tiglHandle.getValue(), wingUID, fileName, deflection); 
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglExportMeshedWingVTKByUID");
            return false;
        }        
        return true;
    }
    
    /**
     * 
     * Exports a CPACS fuselage geometry in VTK format to a local file.
     * Uses simple and fast method to export quick results!
     * 
     * @param fuselageUID - The UID of the fuselage to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @return true if success.
     */
    public boolean tiglExportMeshedFuselageVTK(final String fuselageUID, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }
        
        // export to the file
        errorCode = TIGL.INSTANCE.tiglExportMeshedFuselageVTKSimpleByUID(tiglHandle.getValue(), fuselageUID, fileName, deflection); 
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglExportMeshedFuselageVTK");
            return false;
        }        
        return true;
    }    

    /**
     * 
     * Exports a CPACS fuselage geometry in VTK format to a local file.
     * Uses Index to specify the fuselage. 
     * 
     * @param fuselageIndex - The index number of the fuselage to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @return true if success.
     */
    public boolean tiglExportMeshedFuselageVTK(final int fuselageIndex, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }
        
        // export to the file
        errorCode = TIGL.INSTANCE.tiglExportMeshedFuselageVTKByIndex(tiglHandle.getValue(), fuselageIndex, fileName, deflection); 
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglExportMeshedFuselageVTK");
            return false;
        }        
        return true;
    }

    
    /**
     * 
     * Returns an absolute point from a given relative coordinates eta, xsi on the upper-
     * side of a wing.
     * 
     * @param wingIndex - The index number of the wing.
     * @param segmentIndex- the segment index where the realtiv coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @return - a Point object with x, y, z.
     */
    public TIGLPoint tiglWingGetUpperPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) {
    	TIGLPoint point = new TIGLPoint();
    	
        if (!checkTiglConfiguration()) {
            return point;
        }
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get uppper Point from TIGL
        errorCode = TIGL.INSTANCE.tiglWingGetUpperPoint(tiglHandle.getValue(), wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglWingGetUpperPoint");
            return point;
        }  
        
        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());
        
        return point;
    }

    
    /**
     * 
     * Returns an absolute point from a given relative coordinates eta, xsi on the lower-
     * side of a wing.
     * 
     * @param wingIndex - The index number of the wing.
     * @param segmentIndex- the segment index where the relative coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @return - a Point object with x, y, z.
     */
    public TIGLPoint tiglWingGetLowerPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) {
    	TIGLPoint point = new TIGLPoint();
    	
        if (!checkTiglConfiguration()) {
            return point;
        }
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TIGL.INSTANCE.tiglWingGetUpperPoint(tiglHandle.getValue(), wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglWingGetLowerPoint");
            return point;
        }  
        
        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());
        
        return point;
    }
    
    
    /**
     * 
     * Returns an absolute point from a given relative coordinates eta, xsi on 
     * a fuselage
     * 
     * @param fuselageIndex - The index number of the fuselage.
     * @param segmentIndex- the segment index where the relative coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @return - a Point object with x, y, z.
     */
    public TIGLPoint tiglFuselageGetPoint(final int fuselageIndex, final int segmentIndex, final double eta, final double xsi) {
    	TIGLPoint point = new TIGLPoint();
    	
        if (!checkTiglConfiguration()) {
            return point;
        }
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TIGL.INSTANCE.tiglFuselageGetPoint(tiglHandle.getValue(), fuselageIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglFuselageGetPoint");
            return point;
        }  
        
        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());
        
        return point;
    }
    
    /**
     * Cleanup TIGL stuff. Force TIGL to free memory. 
     */
    public void tiglCloseDocument() {

        if (!checkTiglConfiguration()) {
            return;
        }
        
        // close TIGL configuration
        errorCode = TIGL.INSTANCE.tiglCloseCPACSConfiguration(tiglHandle.getValue());
        if (errorCode != 0) {
            LOGGER.error("tiglCloseDocument failed in TIGLInterface::tiglCloseDocument");
        } 
        tiglHandle = null;
        
        //close TIXI document
        tixi.tixiCloseDocument();
        return;
    }
    
    /**
     * Imports a CPACS configuration from a String with TIGL.
     * 
     * @param cpacsString
     *            The full CPACS configuration to open with TIGL in one string.
     * @param configurationUID
     * 			  The UID of the configuration to open with TIGL 		
     * 		
     * @return true if success.
     */
    public boolean tiglOpenCPACSConfigurationFromString(final String cpacsString, final String configurationUID) {
    	
        // check if a configuration is already loaded
        if (tiglHandle != null) {
            LOGGER.error("TIGLInterface::tiglOpenCPACSConfigurationFromString - Could not open CPACS Configuration. There is already a loaded configuration.");
            return false;
        }
        
        tixiHandle = new IntByReference();
        configUID = configurationUID;
        
        // open cpacs file with TIXI
        tixi = new TIXIInterface();
        if (!tixi.tixiImportFromString(cpacsString)) {
            LOGGER.error("tixiImportFromString failed in TIGLInterface::tiglOpenCPACSConfigurationFromString");
            tixiHandle = null;
            return false;
        }    
        tixiHandle = tixi.getTixiHandle();
        tiglHandle = new IntByReference();       
        // now open cpacs configuration with tigl        
        errorCode = TIGL.INSTANCE.tiglOpenCPACSConfiguration(tixiHandle.getValue(), configurationUID, tiglHandle); 
        if (errorCode != 0) {
            LOGGER.error("tiglOpenCPACSConfiguration failed in TIGLInterface::tiglOpenCPACSConfigurationFromString");
            return false;
        }

        return true;
    }
    
    /**
     * 
     * Open a CPACS configuration with TIGL.
     * 
     * @param cpacsFileName
     *            The full path to a XML file to open with tixi.
     * @param configurationUID
     * 			  The UID of the configuration to open with TIGL 	
     * @return true if success.
     */
    public boolean tiglOpenCPACSConfigurationFromDisk(final String cpacsFileName, final String configurationUID) {
        
        // check if a configuration is already loaded
        if (tiglHandle != null) {
            LOGGER.error("TIGLInterface::tiglOpenCPACSConfigurationFromDisk - Could not open CPACS Configuration. There is already a loaded configuration.");
            return false;
        }
        
        tixiHandle = new IntByReference();
        configUID = configurationUID;
        
        // open cpacs file with TIXI
        tixi = new TIXIInterface();
        if (!tixi.tixiOpenDocument(cpacsFileName, 1)) {
            LOGGER.error("tixiOpenDocument failed in TIGLInterface::tiglOpenCPACSConfigurationFromDisk");
            tixiHandle = null;
            return false;
        }    
        tixiHandle = tixi.getTixiHandle();
        tiglHandle = new IntByReference();       
        // now open cpacs configuration with tigl        
        errorCode = TIGL.INSTANCE.tiglOpenCPACSConfiguration(tixiHandle.getValue(), configurationUID, tiglHandle); 
        if (errorCode != 0) {
            LOGGER.error("tiglOpenCPACSConfiguration failed in TIGLInterface::tiglOpenCPACSConfigurationFromDisk");
            return false;
        }

        return true;
    }
    
    /**
     * 
     * Returns the Version number of TIGL as a Java String.
     * @return a String containing the version number of TIGL.
     */
    public String tiglGetVersion() {
        final String version = TIGL.INSTANCE.tiglGetVersion();
        return version;
    }

    /**
     * Returns the TIGL handle.
     * @return the tigl handle.
     */
    public IntByReference getTiglHandle() {
        return tiglHandle;
    }
    
    
    /**
     * Checks if this instance has a cpacs configuration loaded.
     * 
     * @return true if success
     */
    private boolean checkTiglConfiguration() 
    {
        // check if configuration is loaded
        if (tiglHandle == null) {
            LOGGER.error("checkTiglConfiguration::Tried to work with a cpacs configuration whose handle is null (not loaded/imported before)");
            return false;
        }
        return true;
    }
    

    /**
     * Interface to communicate with TIGL. For detailed documentation see "tigl.h" c++-header file! 
     */
    public interface TIGL extends StdCallLibrary {
        final TIGL INSTANCE = (TIGL) Native.loadLibrary("tigl", TIGL.class);
        int tiglExportNonFusedIGES(final String fileName, final String configurationUID, final String exportFileName);
        int tiglExportIGES(final int cpacsHandle, final String fileName);
        int tiglExportMeshedWingVTKByIndex(final int cpacsHandle, final int wingIndex, final String fileName, final double deflection);
        int tiglExportMeshedWingVTKSimpleByUID(final int cpacsHandle, final String wingUID, final String fileName, final double deflection);
        int tiglExportMeshedFuselageVTKByIndex(final int cpacsHandle, final int fuselageIndex, final String fileName, final double deflection);
        int tiglExportMeshedFuselageVTKSimpleByUID(final int cpacsHandle, final String fuselageUID, final String fileName, final double deflection);
        int tiglWingGetUpperPoint(final int cpacsHandle, final int wingIndex, final int segmentIndex, final double eta, final double xsi, final DoubleByReference pointX, final DoubleByReference pointY, final DoubleByReference pointZ);
        int tiglWingGetLowerPoint(final int cpacsHandle, final int wingIndex, final int segmentIndex, final double eta, final double xsi, final DoubleByReference pointX, final DoubleByReference pointY, final DoubleByReference pointZ);
        int tiglFuselageGetPoint(final int cpacsHandle, final int fuselageIndex, final int segmentIndex, final double eta, final double xsi, final DoubleByReference pointX, final DoubleByReference pointY, final DoubleByReference pointZ);
        int tiglOpenCPACSConfiguration(final int tixiHandle, final String configurationUID, final IntByReference cpacsHandle);
        int tiglCloseCPACSConfiguration(final int cpacsHandle);
        String tiglGetVersion(); 
    }
    
}
