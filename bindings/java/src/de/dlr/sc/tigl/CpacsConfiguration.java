/* 
 * Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
 *
 * Created: 2014-10-23 Martin Siggel <martin.siggel@dlr.de>
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

import de.dlr.sc.tigl.Tigl.GetPointDirectionResult;

public class CpacsConfiguration implements AutoCloseable {
    
    /**
     * Central logger instance.
     */
    protected static final Log LOGGER = LogFactory.getLog(CpacsConfiguration.class);
    
    /** Internal TiGL handler. */
    private int cpacsHandle = -1;

    /** Internal TiXI handler. */
    private int tixiHandle = -1;

    /** The return value from TIGL. */
    private int errorCode = 0;

    /** UID of the CPACS configuration. */ 
    private String configUID = "";
    
    /**
     * Returns the number of wings defined for the current configuration
     * @throws TiglException 
     */
    public int getWingCount() throws TiglException {
        IntByReference wingCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglGetWingCount(cpacsHandle, wingCount);
        throwIfError("getWingCount", errorCode);
        
        return wingCount.getValue();
    }

    
    public String wingGetUID(final int wingIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_wuid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglWingGetUID(cpacsHandle, wingIndex, c_wuid);
        throwIfError("tiglWingGetUID", errorCode);
        
        String wingUID = c_wuid.getValue().getString(0);
        return wingUID;
    }
    
    /**
     * Returns the number of sections of the wing
     * 
     * @param wingIndex Index of the wing
     * @return Number of wing sections
     * @throws TiglException
     */
    public int wingGetSectionCount(final int wingIndex) throws TiglException {
        checkTiglConfiguration();

        IntByReference sectionCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetSectionCount(cpacsHandle, wingIndex, sectionCount);
        throwIfError("tiglWingGetSectionCount", errorCode);
        
        return sectionCount.getValue();
    }
    
    
    /**
     * Returns the UID of the i-th wing section
     * 
     * @param wingIndex Index of the wing 
     * @param sectionIndex Index of the section
     * @return
     * @throws TiglException
     */
    public String wingGetSectionUID(final int wingIndex, final int sectionIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglWingGetSectionUID(cpacsHandle, wingIndex, sectionIndex, c_suid);
        throwIfError("tiglWingGetSectionUID", errorCode);
        
        String sectionUID = c_suid.getValue().getString(0);
        return sectionUID;
    }
    
    /**
     * Returns the number of wing segments for the given wing
     * @param wingIndex Index of the wing (1 <= index <= numerOfWings)
     * 
     * @return Number of wing segments
     * @throws TiglException 
     */
    public int wingGetSegmentCount(final int wingIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference segmentCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetSegmentCount(cpacsHandle, wingIndex, segmentCount);
        throwIfError("tiglWingGetSegmentCount", errorCode);
        
        return segmentCount.getValue();
    }
    
    /**
     * Returns the UID of a wing segment
     * @param wingIndex Index of the wing to query (1 <= index <= #wingCount)
     * @param segmentIndex Index of the segment to query (1 <= index <= #segmentCount)
     * 
     * @return UID of the segment as a String
     * @throws TiglException 
     */
    public String wingGetSegmentUID(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglWingGetSegmentUID(cpacsHandle, wingIndex, segmentIndex, c_suid);
        throwIfError("tiglWingGetSegmentUID", errorCode);
        
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
     * @throws TiglException 
     */
    public TiglPoint wingGetUpperPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get uppper Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetUpperPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        throwIfError("tiglWingGetUpperPoint", errorCode);

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
     * @throws TiglException 
     */
    public TiglPoint wingGetLowerPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetLowerPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        throwIfError("tiglWingGetLowerPoint", errorCode);

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }
    
    
    /**
     * Returns a point on the upper wing surface for a
     * a given wing and segment index. This function is different from ::tiglWingGetUpperPoint: 
     * First, a point on the wing chord surface is computed (defined by segment index and eta,xsi).
     * Then, a line is constructed, which is defined by this point and a direction given by the user.
     * The intersection of this line with the upper wing surface is finally returned.
     * The point is returned in absolute world coordinates.
     * 
     * @param wingIndex - The index number of the wing.
     * @param segmentIndex- the segment index where the realtiv coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @param direction Direction of the intersection line
     * @return - a Point and an error distance (see documentation)
     * @throws TiglException 
     */
    public GetPointDirectionResult wingGetUpperPointAtDirection(final int wingIndex, final int segmentIndex, final double eta, final double xsi, TiglPoint direction) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();
        
        DoubleByReference pointX  = new DoubleByReference();
        DoubleByReference pointY  = new DoubleByReference();
        DoubleByReference pointZ  = new DoubleByReference();
        DoubleByReference errDist = new DoubleByReference();

        // get uppper Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetUpperPointAtDirection(cpacsHandle, wingIndex, segmentIndex, 
                eta, xsi, 
                direction.getX(), direction.getY(), direction.getZ(), 
                pointX, pointY, pointZ, errDist);
        throwIfError("tiglWingGetUpperPointAtDirection", errorCode);

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        GetPointDirectionResult result = new GetPointDirectionResult(point, errDist.getValue());

        return result;
    }
    
    
    /**
     * Returns a point on the lower wing surface for a
     * a given wing and segment index. This function is different from ::tiglWingGetLowerPoint: 
     * First, a point on the wing chord surface is computed (defined by segment index and eta,xsi).
     * Then, a line is constructed, which is defined by this point and a direction given by the user.
     * The intersection of this line with the lower wing surface is finally returned.
     * The point is returned in absolute world coordinates.
     * 
     * @param wingIndex - The index number of the wing.
     * @param segmentIndex- the segment index where the realtiv coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @param direction Direction of the intersection line
     * @return - The point and an error distance (see documentation)
     * @throws TiglException 
     */
    public GetPointDirectionResult wingGetLowerPointAtDirection(final int wingIndex, final int segmentIndex, final double eta, final double xsi, TiglPoint direction) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();
        
        DoubleByReference pointX  = new DoubleByReference();
        DoubleByReference pointY  = new DoubleByReference();
        DoubleByReference pointZ  = new DoubleByReference();
        DoubleByReference errDist = new DoubleByReference();

        // get uppper Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetLowerPointAtDirection(cpacsHandle, wingIndex, segmentIndex, 
                eta, xsi, 
                direction.getX(), direction.getY(), direction.getZ(), 
                pointX, pointY, pointZ, errDist);
        throwIfError("tiglWingGetLowerPointAtDirection", errorCode);

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        GetPointDirectionResult result = new GetPointDirectionResult(point, errDist.getValue());

        return result;
    }
    
    
    /**
     * Returns an absolute point from a given relative coordinates eta, xsi on the wing chord surface,
     * 
     * @param wingIndex - The index number of the wing.
     * @param segmentIndex- the segment index where the relative coordinates belong to.
     * @param eta - the eta coordinate, going from 0 - 1
     * @param xsi - the xsi coordinate, going from 0 - 1
     * @return - a Point object with x, y, z.
     * @throws TiglException 
     */
    public TiglPoint wingGetChordPoint(final int wingIndex, final int segmentIndex, final double eta, final double xsi) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetChordPoint(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        throwIfError("tiglWingGetChordPoint", errorCode);

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
     * @throws TiglException 
     */
    public TiglPoint wingGetChordNormal(final int wingIndex, final int segmentIndex, final double eta, final double xsi) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingGetChordNormal(cpacsHandle, wingIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        throwIfError("tiglWingGetChordNormal", errorCode);

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }
    
    public double wingGetReferenceArea(final int wingIndex, final TiglSymmetryAxis projectionPlane) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference referenceArea = new DoubleByReference();
        errorCode = TiglNativeInterface.tiglWingGetReferenceArea(wingIndex, wingIndex, projectionPlane.getValue(), referenceArea);
        throwIfError("tiglWingGetReferenceArea", errorCode);
        
        return referenceArea.getValue();
    }
    
    
    /**
     * Returns a points on the wing component segment
     * @param componentSegmentUID CPACS UID of the wing component segment
     * @param eta Eta (span wise) coordinate, with 0<=eta<=1
     * @param xsi Xsi (profile depth) coordinate, with 0<=xsi<=1
     * 
     * @return Point on the wing component segment
     * @throws TiglException 
     */
    public TiglPoint wingComponentSegmentGetPoint(String componentSegmentUID, double eta, double xsi) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();
        
        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglWingComponentSegmentGetPoint(cpacsHandle, componentSegmentUID, eta, xsi, pointX, pointY, pointZ);
        throwIfError("tiglWingComponentSegmentGetPoint", errorCode);

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }

    /**
     * Returns the number of fuselages defined for the current configuration
     * @throws TiglException 
     */
    public int getFuselageCount() throws TiglException {
        checkTiglConfiguration();

        
        IntByReference fuselageCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglGetFuselageCount(cpacsHandle, fuselageCount);
        throwIfError("tiglGetFuselageCount", errorCode);
        
        return fuselageCount.getValue();
    }
    
    
    public String fuselageGetUID(final int fuselageIndex) throws TiglException {
        checkTiglConfiguration();

        
        PointerByReference c_fuid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetUID(cpacsHandle, fuselageIndex, c_fuid);
        throwIfError("tiglFuselageGetUID", errorCode);
        
        String fuselageUID = c_fuid.getValue().getString(0);
        return fuselageUID;
    }


    /**
     * Returns the number of sections of the fuselage
     * 
     * @param fuselageIndex Index of the fuselage
     * @return Number of fuselage sections
     * @throws TiglException
     */
    public int fuselageGetSectionCount(final int fuselageIndex) throws TiglException {
        checkTiglConfiguration();

        IntByReference sectionCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSectionCount(cpacsHandle, fuselageIndex, sectionCount);
        throwIfError("tiglFuselageGetSectionCount", errorCode);
        
        return sectionCount.getValue();
    }


    /**
     * Returns the UID of the i-th fuselage section
     * 
     * @param wingIndex Index of the fuselage 
     * @param sectionIndex Index of the section
     * @return
     * @throws TiglException
     */
    public String fuselageGetSectionUID(final int fuselageIndex, final int sectionIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSectionUID(cpacsHandle, fuselageIndex, sectionIndex, c_suid);
        throwIfError("tiglFuselageGetSectionUID", errorCode);
        
        String sectionUID = c_suid.getValue().getString(0);
        return sectionUID;
    }
    
    
    /**
     * Returns the number of fuselage segments for the given fuselage
     * @param fuselageIndex Index of the wing (1 <= index <= numerOfFuselages)
     * 
     * @return Number of fuselage segments
     * @throws TiglException 
     */
    public int fuselageGetSegmentCount(int fuselageIndex) throws TiglException {
        checkTiglConfiguration();

        
        IntByReference segmentCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, segmentCount);
        throwIfError("tiglFuselageGetSegmentCount", errorCode);
        
        return segmentCount.getValue();
    }
    
    
    /**
     * Returns the UID of a fuselage segment
     * @param fuselageIndex Index of the fuselage to query (1 <= index <= #fuselageCount)
     * @param segmentIndex Index of the segment to query (1 <= index <= #segmentCount)
     * 
     * @return UID of the segment as a String
     * @throws TiglException 
     */
    public String fuselageGetSegmentUID(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();

        
        PointerByReference c_suid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSegmentUID(cpacsHandle, fuselageIndex, segmentIndex, c_suid);
        throwIfError("tiglFuselageGetSegmentUID", errorCode);
        
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
     * @throws TiglException 
     */
    public TiglPoint fuselageGetPoint(final int fuselageIndex, final int segmentIndex, final double eta, final double xsi) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();


        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglFuselageGetPoint(cpacsHandle, fuselageIndex, segmentIndex, eta, xsi, pointX, pointY, pointZ);
        throwIfError("tiglFuselageGetPoint", errorCode);

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }

    /**
     * Returns the B-Spline parameterization of a CPACS profile (wing or fuselage currently)
     * @param uid CPACS UID of the profile
     * @return List of B-Spline the profile is constructed of
     * @throws TiglException 
     */
    public ArrayList<TiglBSpline> getProfileSplines(String uid) throws TiglException {
        checkTiglConfiguration();

        ArrayList<TiglBSpline> list = new ArrayList<>();
        
        IntByReference splineCount = new IntByReference(0);
        errorCode = TiglNativeInterface.tiglProfileGetBSplineCount(cpacsHandle, uid, splineCount);
        throwIfError("tiglProfileGetBSplineCount", errorCode);

        for (int ispl = 1; ispl <= splineCount.getValue(); ++ispl) {
            IntByReference degree = new IntByReference();
            IntByReference ncp    = new IntByReference();
            IntByReference nk     = new IntByReference();
            // get data sizes
            errorCode = TiglNativeInterface.tiglProfileGetBSplineDataSizes(cpacsHandle, uid, ispl, degree, ncp, nk);
            throwIfError("tiglProfileGetBSplineDataSizes", errorCode);
            
            // allocate memory
            Pointer cpx   = new Memory(ncp.getValue() * Native.getNativeSize(Double.TYPE));
            Pointer cpy   = new Memory(ncp.getValue() * Native.getNativeSize(Double.TYPE));
            Pointer cpz   = new Memory(ncp.getValue() * Native.getNativeSize(Double.TYPE));
            Pointer knots = new Memory(nk.getValue()  * Native.getNativeSize(Double.TYPE));
            
            // get data
            errorCode = TiglNativeInterface.tiglProfileGetBSplineData(cpacsHandle, uid, ispl, ncp.getValue(), cpx, cpy, cpz, nk.getValue(), knots);
            throwIfError("tiglProfileGetBSplineData", errorCode);
            
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
     * @throws TiglException 
     */    
    public void exportIGES(final String exportFileName) throws TiglException { 
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportIGES(cpacsHandle, exportFileName); 
        throwIfError("tiglExportIGES", errorCode);
    }
    
    /**
     * Exports a CPACS Geometry in STEP format to a local file. 
     * 
     * @param exportFileName
     *            The full filename of the file to be exported.
     * @throws TiglException 
     */    
    public void exportSTEP(final String exportFileName) throws TiglException { 
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportSTEP(cpacsHandle, exportFileName);
        throwIfError("tiglExportSTEP", errorCode);
    }
    
    
    /**
     * Exports a CPACS wing geometry in VTK format to a local file.
     * 
     * @param wingUID - UID of the wing to export
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedWingVTK(final String wingUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedWingVTKByUID(cpacsHandle, wingUID, fileName, deflection); 
        throwIfError("TiglNativeInterface.tiglExportMeshedWingVTKByUID", errorCode);
    }

    /**
     * Exports a wing of a CPACS Geometry (stored in a string) in IGES format to a local file.
     * Uses simple and fast method to export quick results!
     * 
     * @param wingUID - The UID of the wing to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedWingVTKSimple(final String wingUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedWingVTKSimpleByUID(cpacsHandle, wingUID, fileName, deflection); 
        throwIfError("tiglExportMeshedWingVTKSimpleByUID", errorCode);
    }

    /**
     * 
     * Exports a CPACS fuselage geometry in VTK format to a local file.
     * Uses simple and fast method to export quick results!
     * 
     * @param fuselageUID - The UID of the fuselage to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedFuselageVTKSimple(final String fuselageUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedFuselageVTKSimpleByUID(cpacsHandle, fuselageUID, fileName, deflection); 
        throwIfError("tiglExportMeshedFuselageVTKSimpleByUID", errorCode);
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
     * @throws TiglException 
     */
    public void exportMeshedFuselageVTK(final String fuselageUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedFuselageVTKByUID(cpacsHandle, fuselageUID, fileName, deflection); 
        throwIfError("tiglExportMeshedFuselageVTKSimpleByUID", errorCode);
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
    
    public void setCPACSHandle(int handle) {
        cpacsHandle = handle;
    }
    
    public String getConfigurationUID() {
        return configUID; 
    }
    
    public void setConfigurationUID(String uid) {
        configUID = uid;
    }
    
    public void setTixiHandle(int handle) {
        tixiHandle = handle;
    }


    /**
     * Checks if this instance has a cpacs configuration loaded.
     * 
     * @return true if success
     * @throws TiglException 
     */
    private void checkTiglConfiguration() throws TiglException 
    {
        // check if configuration is loaded
        IntByReference isValid = new IntByReference();
        TiglNativeInterface.tiglIsCPACSConfigurationHandleValid(cpacsHandle, isValid);
        if (isValid.getValue() != TiglBoolean.TIGL_TRUE.getValue()) {
            LOGGER.error("checkTiglConfiguration::Tried to work with a cpacs configuration whose handle is invalid (not loaded/imported before)");
            throw new TiglException("Invalid cpacs handle", TiglReturnCode.TIGL_NOT_FOUND);
        }
    }
    
    private static void throwIfError(String methodname, int errorCode) throws TiglException {
        if (errorCode != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            String message = " In TiGL function \"" + methodname + "."
                    + "\"";
            throw new TiglException(message, TiglReturnCode.getEnum(errorCode));
        }
    }
    
    /**
     * Cleanup TIGL stuff. Force TIGL to free memory. 
     */
    private void closeDocument() {
        if (cpacsHandle < 0) {
            return;
        }
        
        LOGGER.info("Closing cpacs configuration " + configUID);

        // close TIGL configuration
        errorCode = TiglNativeInterface.tiglCloseCPACSConfiguration(cpacsHandle);
        cpacsHandle = -1;

        //close TIXI document
        TixiNativeInterface.tixiCloseDocument(tixiHandle);

        tixiHandle = -1;
        return;
    }


    @Override
    public void close() {
        closeDocument();
        
    }
}
