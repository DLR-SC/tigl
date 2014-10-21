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

import java.util.ArrayList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.DoubleByReference;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;


/**
 * This class the high level access class to the TiGL library.

 */
public class TiglInterface {
	
	public class TiglException extends Exception{

		private static final long serialVersionUID = 1L;
		
		private int errorCode = 0;
		private String methodName = "";
		
		public TiglException(String method, int errorcode) {
			this.errorCode = errorcode;
			this.methodName = method;
		}
		
		public int getErrorCode() {
			return errorCode;
		}
		
		public String getLocation() {
			return methodName;
		}
		
		@Override
		public String getMessage() {
			return "TiglException in " + methodName +  
					": Return code=" + errorCode + ", " + TiglNativeInterface.tiglGetErrorString(errorCode);
		}
		
	}

    /**
     * Central logger instance.
     */
    protected static final Log LOGGER = LogFactory.getLog(TiglInterface.class);

    /** Internal TiGL handler. */
    private int cpacsHandle = -1;

    /** Internal TiXI handler. */
    private int tixiHandle = -1;

    /** The return value from TIGL. */
    private int errorCode = 0;

    /** UID of the CPACS configuration. */ 
    private String configUID = "";


    public TiglInterface() {
        // nothing special to initialize
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
    public boolean openCPACSConfigurationFromString(final String cpacsString, final String configurationUID) throws TiglException {

        // check if a configuration is already loaded
        if (cpacsHandle > 0) {
            LOGGER.error("TIGLInterface::tiglOpenCPACSConfigurationFromString - Could not open CPACS Configuration. There is already a loaded configuration.");
            return false;
        }

        configUID = configurationUID;

        // open cpacs file with TIXI
        IntByReference tmpTixiHandle = new IntByReference();
        if (TixiNativeInterface.tixiImportFromString(cpacsString, tmpTixiHandle) != 0) {
            LOGGER.error("tixiImportFromString failed in TIGLInterface::tiglOpenCPACSConfigurationFromString");
            tixiHandle = -1;
            return false;
        }    
        tixiHandle = tmpTixiHandle.getValue();

        // now open cpacs configuration with tigl
        IntByReference tiglHandleTemp = new IntByReference();
        errorCode = TiglNativeInterface.tiglOpenCPACSConfiguration(tixiHandle, configurationUID, tiglHandleTemp); 
        checkError("openCPACSConfigurationFromString", errorCode);
        cpacsHandle = tiglHandleTemp.getValue();

        return true;
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
    public boolean openCPACSConfiguration(final String cpacsFileName, final String configurationUID) throws TiglException {

        // check if a configuration is already loaded
        if (cpacsHandle >= 0) {
            LOGGER.error("TIGLInterface::tiglOpenCPACSConfigurationFromDisk - Could not open CPACS Configuration. There is already a loaded configuration.");
            return false;
        }

        configUID = configurationUID;

        // open cpacs file with TIXI
        IntByReference tmpTixiHandle = new IntByReference();
        if (TixiNativeInterface.tixiOpenDocument(cpacsFileName, tmpTixiHandle) != 0) {
            LOGGER.error("tixiOpenDocument failed in TIGLInterface::tiglOpenCPACSConfigurationFromDisk");
            throw new TiglException("openCPACSConfiguration", TiglReturnCode.TIGL_OPEN_FAILED.getValue());
        }    
        tixiHandle = tmpTixiHandle.getValue();

        IntByReference tiglHandleTemp = new IntByReference();
        // now open cpacs configuration with tigl        
        errorCode = TiglNativeInterface.tiglOpenCPACSConfiguration(tixiHandle, configurationUID, tiglHandleTemp); 
        checkError("openCPACSConfiguration", errorCode);
        cpacsHandle = tiglHandleTemp.getValue();

        return true;
    }

    /**
     * Cleanup TIGL stuff. Force TIGL to free memory. 
     */
    public void closeDocument() {

        if (!checkTiglConfiguration()) {
            return;
        }

        // close TIGL configuration
        errorCode = TiglNativeInterface.tiglCloseCPACSConfiguration(cpacsHandle);
        if (errorCode < 0) {
            LOGGER.error("tiglCloseDocument failed in TIGLInterface::tiglCloseDocument");
        } 
        cpacsHandle = -1;

        //close TIXI document
        int tixiError = TixiNativeInterface.tixiCloseDocument(tixiHandle);
        if (tixiError < 0) {
            LOGGER.error("tiglCloseDocument failed in TixiNativeInterface.tixiCloseDocument");
        } 
        tixiHandle = -1;
        return;
    }

    /**
     * Returns the number of wings defined for the current configuration
     * @throws TiglException 
     */
    public int getWingCount() throws TiglException {
        IntByReference wingCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglGetWingCount(cpacsHandle, wingCount);
        checkError("getWingCount", errorCode);
        
        return wingCount.getValue();
    }

    
    public String wingGetUID(final int wingIndex) {
        if (!checkTiglConfiguration()) {
            return "";
        }
        
        PointerByReference c_wuid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglWingGetUID(cpacsHandle, wingIndex, c_wuid);
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglWingGetUpperPoint");
            return "";
        }  
        
        String wingUID = c_wuid.getValue().getString(0);
        return wingUID;
    }
    
    /**
     * Returns the number of wing segments for the given wing
     * @param wingIndex Index of the wing (1 <= index <= numerOfWings)
     * 
     * @return Number of wing segments
     */
    public int wingGetSegmentCount(final int wingIndex) {
        if (!checkTiglConfiguration()) {
            return 0;
        }
        
        IntByReference segmentCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetSegmentCount(cpacsHandle, wingIndex, segmentCount);
        if (errorCode != 0) {
            LOGGER.error("wingGetSegmentCount failed in TiglNativeInterface.tiglWingGetSegmentCount");
            return 0;
        }
        
        return segmentCount.getValue();
    }
    
    /**
     * Returns the UID of a wing segment
     * @param wingIndex Index of the wing to query (1 <= index <= #wingCount)
     * @param segmentIndex Index of the segment to query (1 <= index <= #segmentCount)
     * 
     * @return UID of the segment as a String
     */
    public String wingGetSegmentUID(final int wingIndex, final int segmentIndex) {
        if (!checkTiglConfiguration()) {
            return "";
        }
        
        PointerByReference c_suid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglWingGetSegmentUID(cpacsHandle, wingIndex, segmentIndex, c_suid);
        if (errorCode != 0) {
            LOGGER.error("wingGetSegmentUID failed in native call of TiglNativeInterface.tiglWingGetSegmentUID");
            return "";
        }  
        
        String segmentUID = c_suid.getValue().getString(0);
        return segmentUID;
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
    public TiglPoint wingGetUpperPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) {
        TiglPoint point = new TiglPoint();

        if (!checkTiglConfiguration()) {
            return point;
        }

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get uppper Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetUpperPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
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
    public TiglPoint wingGetLowerPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) {
        TiglPoint point = new TiglPoint();

        if (!checkTiglConfiguration()) {
            return point;
        }

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetLowerPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
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
     * Returns an absolute point from a given relative coordinates eta, xsi on the wing chord surface,
     * 
     * @param wingIndex - The index number of the wing.
     * @param segmentIndex- the segment index where the relative coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @return - a Point object with x, y, z.
     */
    public TiglPoint wingGetChordPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) {
        TiglPoint point = new TiglPoint();

        if (!checkTiglConfiguration()) {
            return point;
        }

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetChordPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        if (errorCode != 0) {
            LOGGER.error("calling native function TiglNativeInterface.tiglWingGetChordPoint failed");
            return point;
        }  

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }
    
    /**
     * Returns the normal direction of the wing chord surface for a given eta, xsi coordinate
     * 
     * @param wingIndex - The index number of the wing.
     * @param segmentIndex- the segment index where the relative coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @return - a Point object with x, y, z.
     */
    public TiglPoint wingGetChordNormal(final int wingIndex, final int segmentIndex, final double eta, final double xsi) {
        TiglPoint point = new TiglPoint();

        if (!checkTiglConfiguration()) {
            return point;
        }

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetChordNormal(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        if (errorCode != 0) {
            LOGGER.error("calling native function TiglNativeInterface.tiglWingGetChordPoint failed");
            return point;
        }  

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }
    
    public double wingGetReferenceArea(final int wingIndex, final TiglSymmetryAxis projectionPlane) {
        if (!checkTiglConfiguration()) {
            return 0.;
        }
        
        DoubleByReference referenceArea = new DoubleByReference();
        errorCode = TiglNativeInterface.tiglWingGetReferenceArea(wingIndex, wingIndex, projectionPlane.getValue(), referenceArea);
        if (errorCode != 0) {
            LOGGER.error("calling native function TiglNativeInterface.tiglWingGetReferenceArea failed");
            return 0.;
        }  
        
        return referenceArea.getValue();
    }
    
    
    /**
     * Returns a points on the wing component segment
     * @param componentSegmentUID CPACS UID of the wing component segment
     * @param eta Eta (span wise) coordinate, with 0<=eta<=1
     * @param xsi Xsi (profile depth) coordinate, with 0<=xsi<=1
     * 
     * @return Point on the wing component segment
     */
    public TiglPoint wingComponentSegmentGetPoint(String componentSegmentUID, double eta, double xsi) {
        TiglPoint point = new TiglPoint();

        if (!checkTiglConfiguration()) {
            return point;
        }
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();
        
        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingComponentSegmentGetPoint(cpacsHandle, componentSegmentUID, eta, xsi, pointX, pointY, pointZ);
        if (errorCode != 0) {
            LOGGER.error("calling native function TiglNativeInterface.tiglWingComponentSegmentGetPoint failed");
            return point;
        }  

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }

    /**
     * Returns the number of fuselages defined for the current configuration
     */
    public int getFuselageCount() {
        if (!checkTiglConfiguration()) {
            return 0;
        }
        
        IntByReference fuselageCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglGetFuselageCount(cpacsHandle, fuselageCount);
        if (errorCode != 0) {
            LOGGER.error("getFuselageCount failed in TiglNativeInterface.tiglGetFuselageCount");
            return 0;
        }
        
        return fuselageCount.getValue();
    }
    
    
    public String fuselageGetUID(final int fuselageIndex) {
        if (!checkTiglConfiguration()) {
            return "";
        }
        
        PointerByReference c_fuid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetUID(cpacsHandle, fuselageIndex, c_fuid);
        if (errorCode != 0) {
            LOGGER.error("fuselageGetUID failed in TiglNativeInterface.tiglFuselageGetUID");
            return "";
        }  
        
        String fuselageUID = c_fuid.getValue().getString(0);
        return fuselageUID;
    }

    
    /**
     * Returns the number of fuselage segments for the given fuselage
     * @param fuselageIndex Index of the wing (1 <= index <= numerOfFuselages)
     * 
     * @return Number of fuselage segments
     */
    public int fuselageGetSegmentCount(int fuselageIndex) {
        if (!checkTiglConfiguration()) {
            return 0;
        }
        
        IntByReference segmentCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, segmentCount);
        if (errorCode != 0) {
            LOGGER.error("fuselageGetSegmentCount failed in TiglNativeInterface.tiglFuselageGetSegmentCount");
            return 0;
        }
        
        return segmentCount.getValue();
    }
    
    
    /**
     * Returns the UID of a fuselage segment
     * @param fuselageIndex Index of the fuselage to query (1 <= index <= #fuselageCount)
     * @param segmentIndex Index of the segment to query (1 <= index <= #segmentCount)
     * 
     * @return UID of the segment as a String
     */
    public String fuselageGetSegmentUID(final int fuselageIndex, final int segmentIndex) {
        if (!checkTiglConfiguration()) {
            return "";
        }
        
        PointerByReference c_suid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSegmentUID(cpacsHandle, fuselageIndex, segmentIndex, c_suid);
        if (errorCode != 0) {
            LOGGER.error("fuselageGetSegmentUID failed in native call of TiglNativeInterface.tiglFuselageGetSegmentUID");
            return "";
        }  
        
        String segmentUID = c_suid.getValue().getString(0);
        return segmentUID;
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
    public TiglPoint fuselageGetPoint(final int fuselageIndex, final int segmentIndex, final double eta, final double xsi) {
        TiglPoint point = new TiglPoint();

        if (!checkTiglConfiguration()) {
            return point;
        }

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglFuselageGetPoint(cpacsHandle, fuselageIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
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
     * Returns the B-Spline parameterization of a CPACS profile (wing or fuselage currently)
     * @param uid CPACS UID of the profile
     * @return List of B-Spline the profile is constructed of
     */
    public ArrayList<TiglBSpline> getProfileSplines(String uid) {
        ArrayList<TiglBSpline> list = new ArrayList<>();
        
        IntByReference splineCount = new IntByReference(0);
        int tiglRet = TiglNativeInterface.tiglProfileGetBSplineCount(cpacsHandle, uid, splineCount);
        
        if (tiglRet != 0 || splineCount.getValue() < 1) {
            return list;
        }
        
        for (int ispl = 1; ispl <= splineCount.getValue(); ++ispl) {
            IntByReference degree = new IntByReference();
            IntByReference ncp    = new IntByReference();
            IntByReference nk     = new IntByReference();
            // get data sizes
            TiglNativeInterface.tiglProfileGetBSplineDataSizes(cpacsHandle, uid, ispl, degree, ncp, nk);
            
            // allocate memory
            Pointer cpx   = new Memory(ncp.getValue() * Native.getNativeSize(Double.TYPE));
            Pointer cpy   = new Memory(ncp.getValue() * Native.getNativeSize(Double.TYPE));
            Pointer cpz   = new Memory(ncp.getValue() * Native.getNativeSize(Double.TYPE));
            Pointer knots = new Memory(nk.getValue()  * Native.getNativeSize(Double.TYPE));
            
            // get data
            TiglNativeInterface.tiglProfileGetBSplineData(cpacsHandle, uid, ispl, ncp.getValue(), cpx, cpy, cpz, nk.getValue(), knots);
            
            TiglBSpline spline = new TiglBSpline();
            spline.degree = degree.getValue();
            
            // convert data to java types
            for (int iknot = 0; iknot < nk.getValue(); ++iknot) {
                spline.knots.add(knots.getDouble(iknot * Native.getNativeSize(Double.TYPE)));
            }
            
            for (int icp = 0; icp < ncp.getValue(); ++icp) {
                TiglPoint p = new TiglPoint();
                p.setX(cpx.getDouble(icp * Native.getNativeSize(Double.TYPE)));
                p.setY(cpx.getDouble(icp * Native.getNativeSize(Double.TYPE)));
                p.setZ(cpx.getDouble(icp * Native.getNativeSize(Double.TYPE)));
                
                spline.controlPoints.add(p);
            }
            list.add(spline);
        }
        
        
        return list;
    }
    
    
    /**
     * Exports a CPACS Geometry in IGES format to a local file. 
     * 
     * @param exportFileName
     *            The full filename of the file to be exported.
     * @return true if success.
     */    
    public boolean exportIGES(final String exportFileName) { 
        if (!checkTiglConfiguration()) {
            return false;
        }

        // export to the file
        errorCode = TiglNativeInterface.tiglExportIGES(cpacsHandle, exportFileName); 
        if (errorCode != 0) {
            LOGGER.error("exportIGES failed in TiglNativeInterface.tiglExportIGES");
            return false;
        }          
        return true;
    }
    
    /**
     * Exports a CPACS Geometry in STEP format to a local file. 
     * 
     * @param exportFileName
     *            The full filename of the file to be exported.
     * @return true if success.
     */    
    public boolean exportSTEP(final String exportFileName) { 
        if (!checkTiglConfiguration()) {
            return false;
        }

        // export to the file
        errorCode = TiglNativeInterface.tiglExportSTEP(cpacsHandle, exportFileName);
        if (errorCode != 0) {
            LOGGER.error("exportSTEP failed in TiglNativeInterface.tiglExportSTEP");
            return false;
        }          
        return true;
    }
    
    
    /**
     * Exports a CPACS wing geometry in VTK format to a local file.
     * 
     * @param wingUID - UID of the wing to export
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @return true if success.
     */
    public boolean exportMeshedWingVTK(final String wingUID, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedWingVTKByUID(cpacsHandle, wingUID, fileName, deflection); 
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
    public boolean exportMeshedWingVTKSimple(final String wingUID, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedWingVTKSimpleByUID(cpacsHandle, wingUID, fileName, deflection); 
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
    public boolean exportMeshedFuselageVTKSimple(final String fuselageUID, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedFuselageVTKSimpleByUID(cpacsHandle, fuselageUID, fileName, deflection); 
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglExportMeshedFuselageVTK");
            return false;
        }        
        return true;
    }    

    /**
     * 
     * Exports a CPACS fuselage geometry in VTK format to a local file.
     * This method adds metadata to the VTK file and is a bit slower than
     * exportMeshedFuselageVTKSimple.
     * 
     * @param fuselageUID - The UID of the fuselage to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @return true if success..
     */
    public boolean exportMeshedFuselageVTK(final String fuselageUID, final String fileName, final double deflection) {
        if (!checkTiglConfiguration()) {
            return false;
        }

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedFuselageVTKByUID(cpacsHandle, fuselageUID, fileName, deflection); 
        if (errorCode != 0) {
            LOGGER.error("tiglExportIGES failed in TIGLInterface::tiglExportMeshedFuselageVTK");
            return false;
        }        
        return true;
    }
    
    
    /**
     * 
     * Returns the Version number of TIGL as a Java String.
     * @return a String containing the version number of TIGL.
     */
    public String getVersion() {
        final String version = TiglNativeInterface.tiglGetVersion();
        return version;
    }

    /**
     * Returns the CPACS handle. This can be used to directly access the TiglNativeInterface class
     * for low level TiGL access.
     * 
     * @return the CPACS handle.
     */
    public int getCPACSHandle() {
        return cpacsHandle;
    }
    
    public String getConfigurationUID() {
        return configUID; 
    }


    /**
     * Checks if this instance has a cpacs configuration loaded.
     * 
     * @return true if success
     */
    private boolean checkTiglConfiguration() 
    {
        // check if configuration is loaded
        if (cpacsHandle <= 0) {
            LOGGER.error("checkTiglConfiguration::Tried to work with a cpacs configuration whose handle is invalid (not loaded/imported before)");
            return false;
        }
        return true;
    }
    
    private void checkError(String methodname, int errorcode) throws TiglException {
        if (errorcode != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            throw new TiglException(methodname, errorcode);
        }
    }
}
