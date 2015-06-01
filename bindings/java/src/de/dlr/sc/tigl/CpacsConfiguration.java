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
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.DoubleByReference;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

import de.dlr.sc.tigl.Tigl.SectionAndElementIndex;
import de.dlr.sc.tigl.Tigl.SectionAndElementUID;
import de.dlr.sc.tigl.Tigl.SurfaceMaterial;
import de.dlr.sc.tigl.Tigl.WCSFindSegmentResult;
import de.dlr.sc.tigl.Tigl.WCSGetSegmentEtaXsiResult;
import de.dlr.sc.tigl.Tigl.WGetPointDirectionResult;
import de.dlr.sc.tigl.Tigl.WSProjectionResult;
import de.dlr.sc.tigl.Tigl.WingCoordinates;
import de.dlr.sc.tigl.Tigl.GetSegmentIndexResult;
import de.dlr.sc.tigl.Tigl.WingMAC;

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
     * Returns a unique HashCode for a geometric component. 
     *
     * The component, for example a wing or a fuselage, could be specified via
     * its UID. The HashCode is the same as long as the geometry of this component
     * has not changed. The HashCode is valid through the current session only!
     * The hash value is computed from the value of the underlying shape reference. 
     *
     * @param componentUID
     * @return
     * @throws TiglException
     */
    public int componentGetHashCode(String componentUID)  throws TiglException {
        checkTiglConfiguration();
        IntByReference c_hash = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglComponentGetHashCode(cpacsHandle, componentUID, c_hash);
        throwIfError("tiglComponentGetHashCode", errorCode);
        
        return c_hash.getValue();
    }
    
    /**
     * Computes the length of the whole configuration/aircraft
     * 
     * @return The total length
     * @throws TiglException
     */
    public double getLength() throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_len = new DoubleByReference();
        errorCode = TiglNativeInterface.tiglConfigurationGetLength(cpacsHandle, c_len);
        throwIfError("tiglConfigurationGetLength", errorCode);
        
        return c_len.getValue();
    }
    
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

    /**
     * Returns the CPACS UID of a wing
     * 
     * @param wingIndex - Index of the wing with 1 <= index <= nWings
     * 
     * @return UID of the Wing
     * @throws TiglException
     */
    public String wingGetUID(final int wingIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_wuid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglWingGetUID(cpacsHandle, wingIndex, c_wuid);
        throwIfError("tiglWingGetUID", errorCode);
        
        String wingUID = c_wuid.getValue().getString(0);
        return wingUID;
    }
    
    /**
     * Returns the number of component segments for a wing in a CPACS configuration.
     * 
     * @param wingIndex - Index of the wing with 1 <= index <= nWings
     * 
     * @return The number of component segments
     * @throws TiglException
     */
    public int wingGetComponentSegmentCount(final int wingIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_count = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetComponentSegmentCount(cpacsHandle, wingIndex, c_count);
        throwIfError("tiglWingGetComponentSegmentCount", errorCode);
        
        return c_count.getValue();
    }
    
    
    /**
     * Returns the Index of a component segment of a wing.
     * 
     * @param wingIndex - Index of the wing with 1 <= index <= nWings
     * @param componentSegmentUID - The CPACS UID of the wing component segment
     * 
     * @return The index of the component segment inside the wing
     * @throws TiglException
     */
    public int wingGetComponentSegmentIndex(final int wingIndex, final String componentSegmentUID) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_index = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetComponentSegmentIndex(cpacsHandle, wingIndex, componentSegmentUID, c_index);
        throwIfError("tiglWingGetComponentSegmentIndex", errorCode);
        
        return c_index.getValue();
    }
    
    /**
     * Returns the UID of a component segment of a wing.
     * 
     * @param wingIndex - Index of the wing with 1 <= index <= nWings
     * @param componentSegmentIndex - The index of a component segment, starting at 1
     *  
     * @return The UID of the component segment
     * @throws TiglException
     */
    public String wingGetComponentSegmentUID(final int wingIndex, final int componentSegmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_uid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetComponentSegmentUID(cpacsHandle, wingIndex, componentSegmentIndex, c_uid);
        throwIfError("tiglWingGetComponentSegmentUID", errorCode);
        
        return c_uid.getValue().getString(0);
    }
    
    /**
     * Returns the index of a wing given its UID
     * 
     * @param wingUID - CPACS UID of the wing
     * 
     * @return Wing index
     * @throws TiglException
     */
    public int wingGetIndex(final String wingUID) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_windex = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetIndex(cpacsHandle, wingUID, c_windex);
        throwIfError("tiglWingGetIndex", errorCode);
        
        return c_windex.getValue();
    }
    
    /**
     * Returns the number of sections of the wing
     * 
     * @param wingIndex - Index of the wing
     * 
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
     * @param wingIndex - Index of the wing 
     * @param sectionIndex - Index of the section
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
     * Returns the section UID and section element UID of the inner side of a given wing segment. 
     * 
     * @param wingIndex - Index of the wing, starting at 1
     * @param segmentIndex - Index of the wing segment, starting at 1
     * 
     * @return The section UID and the section element UID of the inner side 
     * @throws TiglException
     */
    public SectionAndElementUID wingGetInnerSectionAndElementUID(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        PointerByReference c_euid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetInnerSectionAndElementUID(cpacsHandle, wingIndex, segmentIndex, c_suid, c_euid);
        throwIfError("tiglWingGetInnerSectionAndElementUID", segmentIndex);
        
        return new SectionAndElementUID(c_suid.getValue().getString(0), c_euid.getValue().getString(0));
    }
    
    /**
     * Returns the section UID and section element UID of the outer side of a given wing segment. 
     * 
     * @param wingIndex - Index of the wing, starting at 1
     * @param segmentIndex - Index of the wing segment, starting at 1
     * 
     * @return The section UID and the section element UID of the outer side 
     * @throws TiglException
     */
    public SectionAndElementUID wingGetOuterSectionAndElementUID(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        PointerByReference c_euid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetOuterSectionAndElementUID(cpacsHandle, wingIndex, segmentIndex, c_suid, c_euid);
        throwIfError("tiglWingGetOuterSectionAndElementUID", segmentIndex);
        
        return new SectionAndElementUID(c_suid.getValue().getString(0), c_euid.getValue().getString(0));
    }
    
    /**
     * Returns the section index and section element index of the inner side of a given wing segment. 
     * 
     * @param wingIndex - Index of the wing, starting at 1
     * @param segmentIndex - Index of the wing segment, starting at 1
     * 
     * @return The section index and the section element index of the inner side 
     * @throws TiglException
     */
    public SectionAndElementIndex wingGetStartSectionAndElementIndex(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sindex = new IntByReference();
        IntByReference c_eindex = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetInnerSectionAndElementIndex(cpacsHandle, wingIndex, segmentIndex, c_sindex, c_eindex);
        throwIfError("tiglWingGetInnerSectionAndElementIndex", segmentIndex);
        
        return new SectionAndElementIndex(c_sindex.getValue(), c_eindex.getValue());
    }
    
    /**
     * Returns the section index and section element index of the outer side of a given wing segment. 
     * 
     * @param wingIndex - Index of the wing, starting at 1
     * @param segmentIndex - Index of the wing segment, starting at 1
     * 
     * @return The section index and the section element index of the outer side 
     * @throws TiglException
     */
    public SectionAndElementIndex wingGetOuterSectionAndElementIndex(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sindex = new IntByReference();
        IntByReference c_eindex = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetOuterSectionAndElementIndex(cpacsHandle, wingIndex, segmentIndex, c_sindex, c_eindex);
        throwIfError("tiglWingGetOuterSectionAndElementIndex", segmentIndex);
        
        return new SectionAndElementIndex(c_sindex.getValue(), c_eindex.getValue());
    }
    
    /**
     * Returns the symmetry of a wing
     * 
     * @param wingIndex - Index of the wing with 1 <= index <= nWings
     * 
     * @return Symmetry Axis of the wing
     * @throws TiglException
     */
    public TiglSymmetryAxis wingGetSymmetry(final int wingIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sym = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetSymmetry(cpacsHandle, wingIndex, c_sym);
        throwIfError("tiglWingGetSymmetry", errorCode);
        
        return TiglSymmetryAxis.getEnum(c_sym.getValue());
    }
    
    /**
     * Returns the number of wing segments for the given wing
     * @param wingIndex - Index of the wing (1 <= index <= numerOfWings)
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
     * @param wingIndex - Index of the wing to query (1 <= index <= #wingCount)
     * @param segmentIndex - Index of the segment to query (1 <= index <= #segmentCount)
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
     * Returns the index of a wing given its UID
     * 
     * @param segmentUID - CPACS UID of the wing segment
     * 
     * @return Wing index (parent index) and wing segment index
     * @throws TiglException
     */
    public GetSegmentIndexResult wingGetSegmentIndex(final String segmentUID) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sindex = new IntByReference();
        IntByReference c_windex = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetSegmentIndex(cpacsHandle, segmentUID, c_sindex, c_windex);
        throwIfError("tiglWingGetIndex", errorCode);
        
        return new GetSegmentIndexResult(c_windex.getValue(), c_sindex.getValue());
    }


    /**
     * Inverse function to tiglWingGetLowerPoint and tiglWingGetLowerPoint. 
     * Calculates to a point (x,y,z) in global coordinates the wing segment
     * coordinates and the wing segment index. 
     * 
     * @param wingIndex - The index of the wing, starting at 1 
     * @param p - The point that should be projected onto the wing segment
     * 
     * @return Eta and Xsi coordinates of the point in segment coordinates
     * 
     * @throws TiglException
     */
    public WSProjectionResult wingGetSegmentEtaXsi(final int wingIndex, final TiglPoint p) throws TiglException {
        checkTiglConfiguration();
        DoubleByReference c_eta = new DoubleByReference();
        DoubleByReference c_xsi = new DoubleByReference();
        IntByReference   c_sIdx = new IntByReference();
        IntByReference    c_Top = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetSegmentEtaXsi(cpacsHandle,
                wingIndex, p.getX(), p.getY(), p.getZ(), c_sIdx, c_eta, c_xsi,
                c_Top);
        
        throwIfError("tiglWingGetSegmentEtaXsi", errorCode);
        
        return new WSProjectionResult(
                new WingCoordinates(c_eta.getValue(), c_xsi.getValue()),
                c_Top.getValue() == 1,
                c_sIdx.getValue());
    }
    
    /**
     * Returns eta, xsi coordinates of a componentSegment given 
     * segmentEta and segmentXsi on a wing segment.
     * 
     * @param segmentUID - UID of the wing segment to search for 
     * @param componentSegmentUID - UID of the associated componentSegment 
     * @param segmentEta, segmentXsi - Eta and Xsi coordinates of the point on the wing segment
     *  
     * @return  Eta and Xsi Coordinates of the point on the corresponding component segment. 
     * 
     * @throws TiglException
     */
    public WingCoordinates wingSegmentPointGetComponentSegmentEtaXsi(
            final String segmentUID, final String componentSegmentUID,
            final double segmentEta, final double segmentXsi) throws TiglException {
        
        checkTiglConfiguration();
        
        DoubleByReference c_eta = new DoubleByReference();
        DoubleByReference c_xsi = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingSegmentPointGetComponentSegmentEtaXsi(
                cpacsHandle, segmentUID, componentSegmentUID, 
                segmentEta, segmentXsi, c_eta, c_eta);
        
        throwIfError("tiglWingSegmentPointGetComponentSegmentEtaXsi", errorCode);
        
        return new WingCoordinates(c_eta.getValue(), c_xsi.getValue());
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
    public WGetPointDirectionResult wingGetUpperPointAtDirection(final int wingIndex, final int segmentIndex, final double eta, final double xsi, final TiglPoint direction) throws TiglException {
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

        WGetPointDirectionResult result = new WGetPointDirectionResult(point, errDist.getValue());

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
    public WGetPointDirectionResult wingGetLowerPointAtDirection(final int wingIndex, final int segmentIndex, final double eta, final double xsi, final TiglPoint direction) throws TiglException {
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

        WGetPointDirectionResult result = new WGetPointDirectionResult(point, errDist.getValue());

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
     * Returns the count of wing segments connected to the inner 
     * section of a given segment.
     * 
     * @param wingIndex - Index of the wing
     * @param segmentIndex - Index of the wing segment
     * 
     * @return Number of connected segments
     * @throws TiglException
     */
    public int wingGetInnerConnectedSegmentCount(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_segCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetInnerConnectedSegmentCount(cpacsHandle, wingIndex, segmentIndex, c_segCount);
        throwIfError("tiglWingGetInnerConnectedSegmentCount", errorCode);
        
        return c_segCount.getValue();
    }
    
    /**
     * Returns the count of wing segments connected to the outer 
     * section of a given segment.
     * 
     * @param wingIndex - Index of the wing
     * @param segmentIndex - Index of the wing segment
     * 
     * @return Number of connected segments
     * @throws TiglException
     */
    public int wingGetOuterConnectedSegmentCount(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_segCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetOuterConnectedSegmentCount(cpacsHandle, wingIndex, segmentIndex, c_segCount);
        throwIfError("tiglWingGetOuterConnectedSegmentCount", errorCode);
        
        return c_segCount.getValue();
    }
    
    /**
     * Returns the index (number) of the n-th wing segment connected 
     * to the inner section of a given segment. n starts at 1.
     * 
     * @param wingIndex - Index of the wing
     * @param segmentIndex - Index of the wing segment
     * @param n - n-th segment searched, 1 <= n <= tiglWingGetInnerConnectedSegmentCount(...)
     * 
     * @return Index of the n-th connected segment
     * @throws TiglException
     */
    public int wingGetInnerConnectedSegmentIndex(final int wingIndex, final int segmentIndex, final int n) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_segIndex = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetInnerConnectedSegmentIndex(cpacsHandle, wingIndex, segmentIndex, n, c_segIndex);
        throwIfError("tiglWingGetInnerConnectedSegmentIndex", errorCode);
        
        return c_segIndex.getValue();
    }
    
    /**
     * Returns the index (number) of the n-th wing segment connected 
     * to the outer section of a given segment. n starts at 1.
     * 
     * @param wingIndex - Index of the wing
     * @param segmentIndex - Index of the wing segment
     * @param n - n-th segment searched, 1 <= n <= tiglWingGetOuterConnectedSegmentCount(...)
     * 
     * @return Index of the n-th connected segment
     * @throws TiglException
     */
    public int wingGetOuterConnectedSegmentIndex(final int wingIndex, final int segmentIndex, final int n) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_segIndex = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingGetOuterConnectedSegmentIndex(cpacsHandle, wingIndex, segmentIndex, n, c_segIndex);
        throwIfError("tiglWingGetOuterConnectedSegmentIndex", errorCode);
        
        return c_segIndex.getValue();
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
    
    /**
     * Returns the name of a wing profile.
     * 
     * @param wingIndex - The index of a wing, starting at 1
     * @param sectionIndex - The index of a section, starting at 1 
     * @param elementIndex - The index of an element on the section
     * 
     * @return The name of the wing profile
     * 
     * @throws TiglException
     */
    public String wingGetProfileName(final int wingIndex, final int sectionIndex, final int elementIndex) throws TiglException {
        checkTiglConfiguration();
        PointerByReference c_puid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetProfileName(cpacsHandle, wingIndex, sectionIndex, elementIndex, c_puid);
        
        throwIfError("tiglWingGetProfileName", elementIndex);
        
        return c_puid.getValue().getString(0);
    }
    
    /**
     * Returns the reference area of the wing. 
     * 
     * @param wingIndex - Index of the Wing to calculate the area, starting at 1 
     * @param projectionPlane - Plane on which the wing is projected for calculating the refarea
     *
     * @return The reference area of the wing
     * 
     * @throws TiglException
     */
    public double wingGetReferenceArea(final int wingIndex, final TiglSymmetryAxis projectionPlane) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference referenceArea = new DoubleByReference();
        errorCode = TiglNativeInterface.tiglWingGetReferenceArea(wingIndex, wingIndex, projectionPlane.getValue(), referenceArea);
        throwIfError("tiglWingGetReferenceArea", errorCode);
        
        return referenceArea.getValue();
    }
    
    /**
     * Returns the surface area of the wing.
     * 
     * @param wingIndex - Index of the Wing to calculate the area, starting at 1
     * 
     * @return The surface area
     * @throws TiglException
     */
    public double wingGetSurfaceArea(final int wingIndex) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_area = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetSurfaceArea(cpacsHandle, wingIndex, c_area);
        throwIfError("tiglWingGetSurfaceArea", errorCode);
        
        return c_area.getValue();
    }
    
    /**
     * Returns the surface area of the wing.
     * 
     * @param wingIndex - Index of the Wing to calculate the area, starting at 1
     * @param segmentIndex - Index of the wing segment, with 1 <= segmentIndex <= NumberSegments
     * 
     * @return The wing segment surface area
     * @throws TiglException
     */
    public double wingGetSegmentSurfaceArea(final int wingIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_area = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetSegmentSurfaceArea(cpacsHandle, wingIndex, segmentIndex, c_area);
        throwIfError("tiglWingGetSegmentSurfaceArea", errorCode);
        
        return c_area.getValue();
    }
    
    /**
     * Computes the area of the trimmed upper wing segment surface. 
     * This function can be e.g. used to determine the area of the wing flaps.
     *
     * The computed area does not include the trailing edge or any closing side faces.
     * The trimmed area is defined with the four corner point P1, P2, P3, and P4. 
     * The order of the points should be right handed (see documentation)
     * 
     * @param wingIndex - Index of the Wing to calculate the area, starting at 1
     * @param segmentIndex - Index of the wing segment, with 1 <= segmentIndex <= NumberSegments
     * @param p1 - eta, xsi coordinates of the first point on the wing segment
     * @param p2 - eta, xsi coordinates of the second point on the wing segment
     * @param p3 - eta, xsi coordinates of the third point on the wing segment
     * @param p4 - eta, xsi coordinates of the fourth point on the wing segment
     * 
     * @return Area of the trimmed upper wing surface
     * @throws TiglException
     */
    public double wingGetSegmentUpperSurfaceAreaTrimmed(
            final int wingIndex, final int segmentIndex,
            final WingCoordinates p1, final WingCoordinates p2,
            final WingCoordinates p3, final WingCoordinates p4) throws TiglException {
        
        checkTiglConfiguration();
        
        DoubleByReference c_area = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetSegmentUpperSurfaceAreaTrimmed(
                cpacsHandle, wingIndex, segmentIndex, 
                p1.eta, p1.xsi, 
                p2.eta, p2.xsi, 
                p3.eta, p3.xsi, 
                p4.eta, p4.xsi, 
                c_area);
        
        throwIfError("tiglWingGetSegmentUpperSurfaceAreaTrimmed", errorCode);
        
        return c_area.getValue();
    }
    
    /**
     * Computes the area of the trimmed lower wing segment surface. 
     * This function can be e.g. used to determine the area of the wing flaps.
     *
     * The computed area does not include the trailing edge or any closing side faces.
     * The trimmed area is defined with the four corner point P1, P2, P3, and P4. 
     * The order of the points should be right handed (see documentation)
     * 
     * @param wingIndex - Index of the Wing to calculate the area, starting at 1
     * @param segmentIndex - Index of the wing segment, with 1 <= segmentIndex <= NumberSegments
     * @param p1 - eta, xsi coordinates of the first point on the wing segment
     * @param p2 - eta, xsi coordinates of the second point on the wing segment
     * @param p3 - eta, xsi coordinates of the third point on the wing segment
     * @param p4 - eta, xsi coordinates of the fourth point on the wing segment
     * 
     * @return Area of the trimmed lower wing surface
     * @throws TiglException
     */
    public double wingGetSegmentLowerSurfaceAreaTrimmed(
            final int wingIndex, final int segmentIndex,
            final WingCoordinates p1, final WingCoordinates p2,
            final WingCoordinates p3, final WingCoordinates p4) throws TiglException {
        
        checkTiglConfiguration();
        
        DoubleByReference c_area = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetSegmentLowerSurfaceAreaTrimmed(
                cpacsHandle, wingIndex, segmentIndex, 
                p1.eta, p1.xsi, 
                p2.eta, p2.xsi, 
                p3.eta, p3.xsi, 
                p4.eta, p4.xsi, 
                c_area);
        
        throwIfError("tiglWingGetSegmentLowerSurfaceAreaTrimmed", errorCode);
        
        return c_area.getValue();
    }
    
    /**
     * Returns the wetted area of the wing. 
     * 
     * @param wingUID - wingUID UID of the Wing to calculate the wetted area
     *  
     * @return The wetted area of the wing
     * @throws TiglException
     */
    public double wingGetWettedArea(final String wingUID) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_area = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetWettedArea(cpacsHandle, wingUID, c_area);
        throwIfError("tiglWingGetWettedArea", errorCode);
        
        return c_area.getValue();
    }
    
    /**
     * Computes the volume of the wing
     * 
     * @param wingIndex - Index of the Wing to calculate the volume, starting at 1
     * 
     * @return Volume of the wing
     * @throws TiglException
     */
    public double wingGetVolume(final int wingIndex) throws TiglException {
        checkTiglConfiguration();
        DoubleByReference c_vol = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetVolume(cpacsHandle, wingIndex, c_vol);
        throwIfError("tiglWingGetVolume", errorCode);
        
        return c_vol.getValue();
    }
    
    /**
     * Returns the span of a wing. Fore more details, see documentation.
     * 
     * @param wingUID - CPACS UID of the Wing
     *  
     * @return The wing span.
     * @throws TiglException
     */
    public double wingGetSpan(final String wingUID) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_span = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetSpan(cpacsHandle, wingUID, c_span);
        throwIfError("tiglWingGetSpan", errorCode);
        
        return c_span.getValue();
    }
    
    /**
     * Computes the mean aerodynamic chord and its position.
     * 
     * @param wingUID  CPACS UID of the Wing
     * 
     * @return Object containing MAC and MAC position
     * @throws TiglException
     */
    public WingMAC wingGetMAC(String wingUID) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();
        DoubleByReference mac    = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingGetMAC(cpacsHandle, wingUID, mac, pointZ, pointY, pointZ);
        throwIfError("tiglWingGetMAC", errorCode);
        
        TiglPoint p = new TiglPoint(pointX.getValue(), pointY.getValue(), pointZ.getValue());
        return new WingMAC(mac.getValue(), p);
    }
    
    
    /**
     * Returns a points on the wing component segment
     * @param componentSegmentUID - CPACS UID of the wing component segment
     * @param eta - Eta (span wise) coordinate, with 0<=eta<=1
     * @param xsi - Xsi (profile depth) coordinate, with 0<=xsi<=1
     * 
     * @return Point on the wing component segment
     * @throws TiglException 
     */
    public TiglPoint wingComponentSegmentGetPoint(final String componentSegmentUID, final double eta, final double xsi) throws TiglException {
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
     * Returns the segmentUID and wingUID for a given point on a componentSegment.
     * 
     * @param componentSegmentUID - UID of the componentSegment to search for 
     * @param p - 3D Coordinates of the point of the componentSegment
     * 
     * @return Wing UID and SegmentUID to which the point belongs to
     * 
     * @throws TiglException
     */
    public WCSFindSegmentResult wingComponentSegmentFindSegment(final String componentSegmentUID, final TiglPoint p) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_wuid = new PointerByReference();
        PointerByReference c_suid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglWingComponentSegmentFindSegment(
                cpacsHandle, 
                componentSegmentUID, 
                p.getX(), p.getY(), p.getZ(), 
                c_suid, c_wuid);
        throwIfError("tiglWingComponentSegmentFindSegment", errorCode);
        
        
        return new WCSFindSegmentResult(
                c_wuid.getValue().getString(0), 
                c_suid.getValue().getString(0));
    }
    
    /**
     * Returns eta, xsi, segmentUID and wingUID for a given eta and xsi on a componentSegment. 
     * 
     * @param componentSegmentUID - UID of the componentSegment to search for
     * @param csEta, csXsi - Eta and Xsi of the point of the componentSegment 
     * 
     * 
     * @return Wing UID, SegmentUID, eta and xsi coordinates of the wing segment 
     * @throws TiglException
     */
    public WCSGetSegmentEtaXsiResult wingComponentSegmentPointGetSegmentEtaXsi(final String componentSegmentUID, final double csEta, final double csXsi)  throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_wuid = new PointerByReference();
        PointerByReference c_suid = new PointerByReference();
        DoubleByReference  c_eta = new DoubleByReference();
        DoubleByReference  c_xsi = new DoubleByReference();
        DoubleByReference  c_error = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingComponentSegmentPointGetSegmentEtaXsi(
                cpacsHandle, 
                componentSegmentUID, csEta, csXsi, 
                c_wuid, c_suid, c_eta, c_xsi, c_error);
        
        throwIfError("tiglWingComponentSegmentPointGetSegmentEtaXsi", errorCode);
        
        return new WCSGetSegmentEtaXsiResult(
                c_wuid.getValue().getString(0),
                c_suid.getValue().getString(0),
                c_eta.getValue(),
                c_xsi.getValue(),
                c_error.getValue());
    }
    
    /**
     * Computes the intersection of a line (defined by component segment coordinates) 
     * with an iso-eta line on a specified wing segment. 
     * 
     * @param componenSegmentUID - UID of the componentSegment 
     * @param segmentUID - UID of the segment, the intersection should be calculated with
     * @param csEta1, csEta2 - Start and end eta coordinates of the intersection line (given as component segment coordinates) 
     * @param csXsi1, csXsi2 - Start and end xsi coordinates of the intersection line (given as component segment coordinates) 
     * @param segmentEta -  Eta coordinate of the iso-eta segment intersection line
     * 
     * @return Xsi coordinate of the intersection point on the wing segment
     * @throws TiglException
     */
    public double wingComponentSegmentGetSegmentIntersection(final String componentSegmentUID,
            final String segmentUID,
            final double csEta1, final double csXsi1,
            final double csEta2, final double csXsi2,
            final double segmentEta) throws TiglException {
        
        checkTiglConfiguration();
        
        DoubleByReference c_sxsi = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglWingComponentSegmentGetSegmentIntersection(
                cpacsHandle, 
                componentSegmentUID, 
                segmentUID, 
                csEta1, csXsi1, csEta2, csXsi2, 
                segmentEta, c_sxsi);
        
        throwIfError("tiglWingComponentSegmentGetSegmentIntersection", errorCode);
        
        return c_sxsi.getValue();
    }
    
    /**
     * Returns the number of segments belonging to a component segment.
     * 
     * @param componentSegmentUID - UID of the componentSegment
     * 
     * @return Number of segments belonging to the component segment
     * @throws TiglException
     */
    public int wingComponentSegmentGetNumberOfSegments(final String componentSegmentUID) throws TiglException {
        
        checkTiglConfiguration();
        
        IntByReference c_scount = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglWingComponentSegmentGetNumberOfSegments(cpacsHandle, componentSegmentUID, c_scount);
        throwIfError("tiglWingComponentSegmentGetNumberOfSegments", errorCode);
        
        return c_scount.getValue();
    }
    
    /**
     * Returns the segment UID of a segment belonging to a component segment. The segment is
     * specified with its index,which is in the 1...nsegments. The number of segments
     * nsegments can be queried with tiglWingComponentSegmentGetNumberOfSegments. 
     * 
     * @param componentSegmentUID - UID of the componentSegment 
     * @param segmentIndex - Index of the segment (1 <= index <= nsegments) 
     * @return UID of the segment
     * 
     * @throws TiglException
     */
    public String wingComponentSegmentGetSegmentUID(final String componentSegmentUID, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        errorCode = TiglNativeInterface.tiglWingComponentSegmentGetSegmentUID(cpacsHandle, componentSegmentUID, segmentIndex, c_suid);
        throwIfError("tiglWingComponentSegmentGetSegmentUID", cpacsHandle);
        
        return c_suid.getValue().getString(0);
    }
    
    /**
     * Returns all materials that are defined at a certain position on the wing component segment
     * 
     * @param componentSegmentUID - UID of the componentSegment 
     * @param type - The type of the surface to query, i.e. upper wing, lower wing, or inner structure
     * @param eta, xsi - The coordinates of the point (wing component segment coordinates)
     *
     * @return List of Materials defined on the point
     * @throws TiglException
     */
    public List<SurfaceMaterial> wingCompomentSegmentGetMaterials(final String componentSegmentUID, final TiglStructureType type, 
            final double eta, final double xsi) throws TiglException {
        
        checkTiglConfiguration();
        
        // get number of materials at a point on the component segment
        IntByReference c_matcount = new IntByReference();
        errorCode = TiglNativeInterface.tiglWingComponentSegmentGetMaterialCount(cpacsHandle, componentSegmentUID, type.getValue(), eta, xsi, c_matcount);
        throwIfError("tiglWingComponentSegmentGetMaterialCount", errorCode);
        
        List<SurfaceMaterial> materials = new ArrayList<>();
        
        for (int i = 1; i <= c_matcount.getValue(); ++i) {
            DoubleByReference c_thick = new DoubleByReference();
            PointerByReference c_uid  = new PointerByReference();
            
            errorCode = TiglNativeInterface.tiglWingComponentSegmentGetMaterialThickness(cpacsHandle, componentSegmentUID, type.getValue(), eta, xsi, i, c_thick);
            throwIfError("tiglWingComponentSegmentGetMaterialThickness", errorCode);
            
            errorCode = TiglNativeInterface.tiglWingComponentSegmentGetMaterialUID(cpacsHandle, componentSegmentUID, type.getValue(), eta, xsi, i, c_uid);
            throwIfError("tiglWingComponentSegmentGetMaterialUID", errorCode);
            
            SurfaceMaterial mat = new SurfaceMaterial(c_uid.getValue().getString(0), c_thick.getValue());
            materials.add(mat);
        }
        
        return materials;
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
    
    /**
     * Returns the index of a fuselage given its UID
     * 
     * @param fuselageUID - CPACS UID of the fuselage
     * 
     * @return Wing index
     * @throws TiglException
     */
    public int fuselageGetIndex(final String fuselageUID) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_findex = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetIndex(cpacsHandle, fuselageUID, c_findex);
        throwIfError("tiglFuselageGetIndex", errorCode);
        
        return c_findex.getValue();
    }
    
    /**
     * Returns the name of a fuselage profile.
     * 
     * @param fuselageIndex - The index of a fuselage, starting at 1
     * @param sectionIndex - The index of a section, starting at 1 
     * @param elementIndex - The index of an element on the section
     * 
     * @return The name of the fuselage profile
     * 
     * @throws TiglException
     */
    public String fuselageGetProfileName(final int fuselageIndex, final int sectionIndex, final int elementIndex) throws TiglException {
        checkTiglConfiguration();
        PointerByReference c_puid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetProfileName(cpacsHandle, fuselageIndex, sectionIndex, elementIndex, c_puid);
        
        throwIfError("tiglFuselageGetProfileName", elementIndex);
        
        return c_puid.getValue().getString(0);
    }
    
    /**
     * Returns the UID of a fuselage.
     * 
     * @param fuselageIndex - The index of a fuselage, starting at 1 
     * 
     * @return The uid of the fuselage
     * @throws TiglException
     */
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
     * @param fuselageIndex - Index of the fuselage
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
     * @param fuselageIndex - Index of the fuselage 
     * @param sectionIndex - Index of the section
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
     * Returns the section UID and section element UID of the start side of a given fuselage segment. 
     * 
     * @param fuselageIndex - Index of the fuselage, starting at 1
     * @param segmentIndex - Index of the fuselage segment, starting at 1
     * 
     * @return The section UID and the section element UID of the start side 
     * @throws TiglException
     */
    public SectionAndElementUID fuselageGetStartSectionAndElementUID(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        PointerByReference c_euid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetStartSectionAndElementUID(cpacsHandle, fuselageIndex, segmentIndex, c_suid, c_euid);
        throwIfError("tiglFuselageGetStartSectionAndElementUID", segmentIndex);
        
        return new SectionAndElementUID(c_suid.getValue().getString(0), c_euid.getValue().getString(0));
    }
    
    /**
     * Returns the section UID and section element UID of the end side of a given fuselage segment. 
     * 
     * @param fuselageIndex - Index of the fuselage, starting at 1
     * @param segmentIndex - Index of the fuselage segment, starting at 1
     * 
     * @return The section UID and the section element UID of the end side 
     * @throws TiglException
     */
    public SectionAndElementUID fuselageGetEndSectionAndElementUID(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_suid = new PointerByReference();
        PointerByReference c_euid = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetEndSectionAndElementUID(cpacsHandle, fuselageIndex, segmentIndex, c_suid, c_euid);
        throwIfError("tiglFuselageGetEndSectionAndElementUID", segmentIndex);
        
        return new SectionAndElementUID(c_suid.getValue().getString(0), c_euid.getValue().getString(0));
    }
    
    /**
     * Returns the section index and section element index of the start side of a given fuselage segment. 
     * 
     * @param fuselageIndex - Index of the fuselage, starting at 1
     * @param segmentIndex - Index of the fuselage segment, starting at 1
     * 
     * @return The section index and the section element index of the start side 
     * @throws TiglException
     */
    public SectionAndElementIndex fuselageGetStartSectionAndElementIndex(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sindex = new IntByReference();
        IntByReference c_eindex = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetStartSectionAndElementIndex(cpacsHandle, fuselageIndex, segmentIndex, c_sindex, c_eindex);
        throwIfError("tiglFuselageGetStartSectionAndElementIndex", segmentIndex);
        
        return new SectionAndElementIndex(c_sindex.getValue(), c_eindex.getValue());
    }
    
    /**
     * Returns the section index and section element index of the end side of a given fuselage segment. 
     * 
     * @param fuselageIndex - Index of the fuselage, starting at 1
     * @param segmentIndex - Index of the fuselage segment, starting at 1
     * 
     * @return The section index and the section element index of the end side 
     * @throws TiglException
     */
    public SectionAndElementIndex fuselageGetEndSectionAndElementIndex(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sindex = new IntByReference();
        IntByReference c_eindex = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetEndSectionAndElementIndex(cpacsHandle, fuselageIndex, segmentIndex, c_sindex, c_eindex);
        throwIfError("tiglFuselageGetEndSectionAndElementIndex", segmentIndex);
        
        return new SectionAndElementIndex(c_sindex.getValue(), c_eindex.getValue());
    }
    
    
    /**
     * Returns the number of fuselage segments for the given fuselage
     * @param fuselageIndex Index of the wing (1 <= index <= numerOfFuselages)
     * 
     * @return Number of fuselage segments
     * @throws TiglException 
     */
    public int fuselageGetSegmentCount(final int fuselageIndex) throws TiglException {
        checkTiglConfiguration();

        
        IntByReference segmentCount = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSegmentCount(cpacsHandle, fuselageIndex, segmentCount);
        throwIfError("tiglFuselageGetSegmentCount", errorCode);
        
        return segmentCount.getValue();
    }
    
    /**
     * Returns the index of a fuselage given its UID
     * 
     * @param fuselageSegmentUID - CPACS UID of the fuselage segment
     * 
     * @return Fuselage index (parent index) and fuselage segment index
     * @throws TiglException
     */
    public GetSegmentIndexResult fuselageGetSegmentIndex(final String fuselageSegmentUID) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sindex = new IntByReference();
        IntByReference c_findex = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSegmentIndex(cpacsHandle, fuselageSegmentUID, c_sindex, c_findex);
        throwIfError("tiglFuselageGetSegmentIndex", errorCode);
        
        return new GetSegmentIndexResult(c_findex.getValue(), c_sindex.getValue());
    }
    
    /**
     * Returns the UID of a fuselage segment
     * @param fuselageIndex - Index of the fuselage to query (1 <= index <= #fuselageCount)
     * @param segmentIndex - Index of the segment to query (1 <= index <= #segmentCount)
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
     * Returns the surface area of the fuselage.
     * 
     * @param fuselageIndex - Index of the fuselage to calculate the area, starting at 1
     * 
     * @return The surface area
     * @throws TiglException
     */
    public double fuselageGetSurfaceArea(final int fuselageIndex) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_area = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetSurfaceArea(cpacsHandle, fuselageIndex, c_area);
        throwIfError("tiglFuselageGetSurfaceArea", errorCode);
        
        return c_area.getValue();
    }
    
    /**
     * Returns the surface area of the fuselage.
     * 
     * @param fuselageIndex - Index of the fuselage to calculate the area, starting at 1
     * @param segmentIndex - Index of the fuselage segment, with 1 <= segmentIndex <= NumberSegments
     * 
     * @return The fuselage segment surface area
     * @throws TiglException
     */
    public double fuselageGetSegmentSurfaceArea(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_area = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetSegmentSurfaceArea(cpacsHandle, fuselageIndex, segmentIndex, c_area);
        throwIfError("tiglFuselageGetSegmentSurfaceArea", errorCode);
        
        return c_area.getValue();
    }
    
    /**
     * Returns the symmetry of a fuselage
     * 
     * @param fuselageIndex - Index of the fuselage with 1 <= index <= nFuselages
     * 
     * @return Symmetry Axis of the fuselage
     * @throws TiglException
     */
    public TiglSymmetryAxis fuselageGetSymmetry(final int fuselageIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_sym = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetSymmetry(cpacsHandle, fuselageIndex, c_sym);
        throwIfError("tiglFuselageGetSymmetry", errorCode);
        
        return TiglSymmetryAxis.getEnum(c_sym.getValue());
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
     * 
     * Returns a point on a fuselage surface for a given fuselage and segment 
     * index and an angle alpha (degree). 
     * 
     * @param fuselageIndex - The index number of the fuselage starting at 1
     * @param segmentIndex- The index of the segment of the fuselage, starting at 1 
     * @param eta - Eta in the range 0.0 <= eta <= 1.0 
     * @param alpha - Angle alpha in degrees. No range restrictions.
     * 
     * @return - a Point object with x, y, z.
     * @throws TiglException 
     */
    public TiglPoint fuselageGetPointAngle(final int fuselageIndex, final int segmentIndex, final double eta, final double alpha) throws TiglException {
        TiglPoint point = new TiglPoint();

        checkTiglConfiguration();


        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglFuselageGetPointAngle(cpacsHandle, fuselageIndex, segmentIndex, eta, alpha, pointX, pointY, pointZ);
        throwIfError("tiglFuselageGetPointAngle", errorCode);

        point.setX(pointX.getValue());
        point.setY(pointY.getValue());
        point.setZ(pointZ.getValue());

        return point;
    }
    
    /**
     * Returns a point on a fuselage surface for a given fuselage and segment index
     * and an angle alpha (degree). 0 degree of the angle alpha is meant to be "up"
     * in the direction of the positive z-axis like specifies in cpacs. The origin of
     * the line that will be rotated with the angle alpha could be translated via 
     * the parameters y_cs and z_cs. 
     * 
     * @param fuselageIndex - The index number of the fuselage starting at 1
     * @param segmentIndex- The index of the segment of the fuselage, starting at 1 
     * @param eta - Eta in the range 0.0 <= eta <= 1.0 
     * @param alpha - Angle alpha in degrees. No range restrictions.
     * @param y_cs - Shifts the origin of the angle alpha in y-direction.
     * @param z_cs - Shifts the origin of the angle alpha in z-direction. 
     * 
     * @return - A Point on the fuselage in world coordinates.
     * @throws TiglException 
     */
    public TiglPoint fuselageGetPointAngleTranslated(
            final int fuselageIndex, final int segmentIndex, 
            final double eta, final double alpha,
            final double y_cs, final double z_cs) throws TiglException {

        checkTiglConfiguration();

        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();

        // get lower Point from TIGL
        errorCode = TiglNativeInterface.tiglFuselageGetPointAngleTranslated(
                cpacsHandle, fuselageIndex, segmentIndex, eta, alpha, 
                y_cs, z_cs,
                pointX, pointY, pointZ);
        
        throwIfError("tiglFuselageGetPointAngleTranslated", errorCode);

        return new TiglPoint(pointX.getValue(), pointY.getValue(), pointZ.getValue());
    }
    
    /**
     * Returns a point on a fuselage surface for a given fuselage and segment index.
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     * @param eta - eta in the range 0.0 <= eta <= 1.0
     * @param xpos - x position of a cutting plane
     * @param pointIndex - Defines witch point if more than one.
     * 
     * @return Point in absolute world coordinates 
     * @throws TiglException
     */
    public TiglPoint fuselageGetPointOnXPlane(final int fuselageIndex, final int segmentIndex,
            final double eta, final double xpos, final int pointIndex) throws TiglException {
        
        checkTiglConfiguration();
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetPointOnXPlane(
                cpacsHandle, fuselageIndex, segmentIndex, 
                eta, xpos, pointIndex, 
                pointX, pointY, pointZ);
        
        throwIfError("tiglFuselageGetPointOnXPlane", errorCode);
        
        return new TiglPoint(pointX.getValue(), pointY.getValue(), pointZ.getValue());
    }
    
    /**
     * Returns a point on a fuselage surface for a given fuselage and segment index.
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     * @param eta - eta in the range 0.0 <= eta <= 1.0
     * @param ypos - y position of a cutting plane
     * @param pointIndex - Defines witch point if more than one.
     * 
     * @return Point in absolute world coordinates 
     * @throws TiglException
     */
    public TiglPoint fuselageGetPointOnYPlane(final int fuselageIndex, final int segmentIndex,
            final double eta, final double ypos, final int pointIndex) throws TiglException {
        
        checkTiglConfiguration();
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetPointOnYPlane(
                cpacsHandle, fuselageIndex, segmentIndex, 
                eta, ypos, pointIndex, 
                pointX, pointY, pointZ);
        
        throwIfError("tiglFuselageGetPointOnYPlane", errorCode);
        
        return new TiglPoint(pointX.getValue(), pointY.getValue(), pointZ.getValue());
    }
    
    /**
     * Returns the number of points on a fuselage surface
     * for a given fuselage and a give x-position.
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     * @param eta - eta in the range 0.0 <= eta <= 1.0
     * @param xpos - x position of a cutting plane
     * 
     * 
     * @return The number of intersection points
     * @throws TiglException
     */
    public int fuselageGetNumPointsOnXPlane(final int fuselageIndex, final int segmentIndex,
            final double eta, final double xpos) throws TiglException {
        
        checkTiglConfiguration();
        IntByReference c_count = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetNumPointsOnXPlane(
                cpacsHandle, fuselageIndex, segmentIndex, eta, xpos, c_count);
        
        throwIfError("tiglFuselageGetNumPointsOnXPlane", errorCode);
        
        return c_count.getValue();
    }
    
    /**
     * Returns the number of points on a fuselage surface
     * for a given fuselage and a give x-position.
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     * @param eta - eta in the range 0.0 <= eta <= 1.0
     * @param ypos - x position of a cutting plane
     * 
     * 
     * @return The number of intersection points
     * @throws TiglException
     */
    public int fuselageGetNumPointsOnYPlane(final int fuselageIndex, final int segmentIndex,
            final double eta, final double ypos) throws TiglException {
        
        checkTiglConfiguration();
        IntByReference c_count = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetNumPointsOnYPlane(
                cpacsHandle, fuselageIndex, segmentIndex, eta, ypos, c_count);
        
        throwIfError("tiglFuselageGetNumPointsOnYPlane", errorCode);
        
        return c_count.getValue();
    }
    
    /**
     * Returns the circumference of a fuselage surface for a given fuselage and segment index and an eta.
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     * @param eta - eta in the range 0.0 <= eta <= 1.0
     * 
     * @return The Circumference of the fuselage at the given position
     * @throws TiglException
     */
    public double fuselageGetCircumference(final int fuselageIndex, final int segmentIndex, final double eta)  throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference c_circ = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetCircumference(cpacsHandle, fuselageIndex, segmentIndex, eta, c_circ);
        throwIfError("tiglFuselageGetCircumference", errorCode);
        
        return c_circ.getValue();
    }
    
    /**
     * Returns the count of segments connected to the
     * start section of a given fuselage segment. 
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     *
     * @return The number of connected segments
     * @throws TiglException
     */
    public int fuselageGetStartConnectedSegmentCount(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_count = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetStartConnectedSegmentCount(cpacsHandle, fuselageIndex, segmentIndex, c_count);
        throwIfError("tiglFuselageGetStartConnectedSegmentCount", errorCode);
        
        return c_count.getValue();
    }
    
    /**
     * Returns the count of segments connected to the
     * end section of a given fuselage segment. 
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     *
     * @return The number of connected segments
     * @throws TiglException
     */
    public int fuselageGetEndConnectedSegmentCount(final int fuselageIndex, final int segmentIndex) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_count = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetEndConnectedSegmentCount(cpacsHandle, fuselageIndex, segmentIndex, c_count);
        throwIfError("tiglFuselageGetStartConnectedEndCount", errorCode);
        
        return c_count.getValue();
    }
    
    /**
     * Returns the index (number) of the n-th segment connected to
     * the start section of a given fuselage segment. n starts at 1. 
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     * @param n - n-th segment searched, 1 <= n <= tiglFuselageGetStartConnectedSegmentCount(...) 
     *
     * @return Segment index of the n-th connected segment
     * @throws TiglException
     */
    public int fuselageGetStartConnectedSegmentIndex(final int fuselageIndex, final int segmentIndex, final int n) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_index = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetStartConnectedSegmentIndex(cpacsHandle, fuselageIndex, segmentIndex, n, c_index);
        throwIfError("tiglFuselageGetStartConnectedSegmentIndex", errorCode);
        
        return c_index.getValue();
    }
    
    /**
     * Returns the index (number) of the n-th segment connected to
     * the end section of a given fuselage segment. n starts at 1. 
     * 
     * @param fuselageIndex - The index of the fuselage, starting at 1 
     * @param segmentIndex - The index of the segment of the fuselage, starting at 1
     * @param n - n-th segment searched, 1 <= n <= tiglFuselageGetEndConnectedSegmentCount(...) 
     *
     * @return Segment index of the n-th connected segment
     * @throws TiglException
     */
    public int fuselageGetEndConnectedSegmentIndex(final int fuselageIndex, final int segmentIndex, final int n) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_index = new IntByReference();
        errorCode = TiglNativeInterface.tiglFuselageGetEndConnectedSegmentIndex(cpacsHandle, fuselageIndex, segmentIndex, n, c_index);
        throwIfError("tiglFuselageGetEndConnectedSegmentIndex", errorCode);
        
        return c_index.getValue();
    }
    
    /**
     * Computes the volume of the fuselage
     * 
     * @param fuselageIndex - Index of the fuselage to calculate the volume, starting at 1
     * 
     * @return Volume of the fuselage
     * @throws TiglException
     */
    public double fuselageGetVolume(final int fuselageIndex) throws TiglException {
        checkTiglConfiguration();
        DoubleByReference c_vol = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetVolume(cpacsHandle, fuselageIndex, c_vol);
        throwIfError("tiglFuselageGetVolume", errorCode);
        
        return c_vol.getValue();
    }
    
    /**
     * Returns the point where the distance between the selected fuselage 
     * and the ground is at minimum. The Fuselage could be turned with a 
     * given angle at at given axis, specified by a point and a direction
     *  
     * @param fuselageUID - CPACS UID of the fuselage
     * @param pointOnAxis, direction - The axis , around which the fuselage should be rotated
     * @param angle - Angle of rotation around the axis (in degree).
     * 
     * @return Point on fuselage with minimum distance to the ground (z=const)
     * @throws TiglException
     */
    public TiglPoint fuselageGetMinumumDistanceToGround(final String fuselageUID, 
            final TiglPoint pointOnAxis, final TiglPoint direction, double angle) throws TiglException {
        
        checkTiglConfiguration();
        
        DoubleByReference c_pointX = new DoubleByReference();
        DoubleByReference c_pointY = new DoubleByReference();
        DoubleByReference c_pointZ = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglFuselageGetMinumumDistanceToGround(
                cpacsHandle, fuselageUID, 
                pointOnAxis.getX(), pointOnAxis.getY(), pointOnAxis.getZ(), 
                direction.getX(), direction.getY(), direction.getZ(), angle, 
                c_pointX, c_pointY, c_pointZ);
        
        throwIfError("tiglFuselageGetMinumumDistanceToGround", errorCode);
        
        return new TiglPoint(c_pointX.getValue(), c_pointY.getValue(), c_pointZ.getValue());
    }
    
    /**
     * tiglIntersectComponents computes the intersection line(s) between two shapes
     * specified by their CPACS uid. It returns an intersection ID for further computations
     * on the result. To query points on the intersection line, ::tiglIntersectGetPoint has
     * to be called.
     * 
     * @param componentUidOne - The UID of the first component
     * @param componentUidTwo - The UID of the second component
     * 
     * @return A unique identifier that is associated with the computed intersection.
     * @throws TiglException
     */
    public String intersectComponents(final String componentUidOne, final String componentUidTwo) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_id = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglIntersectComponents(cpacsHandle, componentUidOne, componentUidTwo, c_id);
        throwIfError("tiglIntersectComponents", errorCode);
        
        return c_id.getValue().getString(0);
    }
    
    /**
     * tiglIntersectComponents computes the intersection line(s) between two shapes
     * specified by their CPACS uid. It returns an intersection ID for further computations
     * on the result. To query points on the intersection line, ::tiglIntersectGetPoint has
     * to be called.
     * 
     * @param componentUid - The UID of the first component
     * @param point, normal - The plane paramters i.e. a point on the plane and the plane's normal vector
     * 
     * @return A unique identifier that is associated with the computed intersection.
     * @throws TiglException
     */
    public String intersectWithPlane(final String componentUid, final TiglPoint point, final TiglPoint normal) throws TiglException {
        checkTiglConfiguration();
        
        PointerByReference c_id = new PointerByReference();
        
        errorCode = TiglNativeInterface.tiglIntersectWithPlane(cpacsHandle, componentUid, 
                point.getX(), point.getY(), point.getZ(), 
                normal.getX(), normal.getY(), normal.getZ(), 
                c_id);
        throwIfError("tiglIntersectWithPlane", errorCode);
        
        return c_id.getValue().getString(0);
    }
    
    /**
     * tiglIntersectGetLineCount return the number of intersection lines computed by 
     * tiglIntersectComponents or tiglIntersectWithPlane for the given intersectionID.
     * 
     * @param intersectionID - The intersection identifier returned by tiglIntersectComponents or tiglIntersectWithPlane
     * 
     * @return The number of intersection lines computed by tiglIntersectComponents or tiglIntersectWithPlane.
     * @throws TiglException
     */
    public int intersectGetLineCount(final String intersectionID) throws TiglException {
        checkTiglConfiguration();
        
        IntByReference c_count = new IntByReference();
        
        errorCode = TiglNativeInterface.tiglIntersectGetLineCount(cpacsHandle, intersectionID, c_count);
        throwIfError("tiglIntersectGetLineCount", errorCode);
        
        return c_count.getValue();
    }
    
    /**
     * tiglIntersectGetPoint samples a point on an intersection line calculated by
     * tiglIntersectComponents o::tiglIntersectWithPlane.
     * 
     * @param intersectionID - The intersection identifier returned by tiglIntersectComponents or tiglIntersectWithPlane
     * @param lineIdx - Line index to sample from. To get the number of lines, call ::tiglIntersectGetLineCount.
     *                  1 <= lineIdx <= lineCount.
     * @param eta - Parameter on the curve that determines the point position, with 0 <= eta <= 1.
     * 
     * @return Point on the i-th intersection line
     * @throws TiglException
     */
    public TiglPoint intersectGetPoint(final String intersectionID, final int lineIdx, final double eta) throws TiglException {
        checkTiglConfiguration();
        
        DoubleByReference pointX = new DoubleByReference();
        DoubleByReference pointY = new DoubleByReference();
        DoubleByReference pointZ = new DoubleByReference();
        
        errorCode = TiglNativeInterface.tiglIntersectGetPoint(cpacsHandle, intersectionID, lineIdx, eta, pointX, pointY, pointZ);
        throwIfError("tiglIntersectGetPoint", errorCode);
        
        return new TiglPoint(pointX.getValue(), pointY.getValue(), pointZ.getValue());
    }

    /**
     * Returns the B-Spline parameterization of a CPACS profile (wing or fuselage currently)
     * @param uid CPACS UID of the profile
     * @return List of B-Spline the profile is constructed of
     * @throws TiglException 
     */
    public List<TiglBSpline> getProfileSplines(final String uid) throws TiglException {
        checkTiglConfiguration();

        List<TiglBSpline> list = new ArrayList<>();
        
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
     * Exports the trimmed/fused CPACS Geometry in IGES format a file. 
     * 
     * @param exportFileName
     *            The full filename of the file to be exported.
     * @throws TiglException 
     */    
    public void exportFusedIGES(final String exportFileName) throws TiglException { 
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportFusedWingFuselageIGES(cpacsHandle, exportFileName);
        throwIfError("tiglExportFusedWingFuselageIGES", errorCode);
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
     * Exports the trimmed/fused CPACS Geometry in STEP format a file. 
     * 
     * @param exportFileName
     *            The full filename of the file to be exported.
     * @throws TiglException 
     */    
    public void exportFusedSTEP(final String exportFileName) throws TiglException { 
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportFusedSTEP(cpacsHandle, exportFileName);
        throwIfError("tiglExportFusedSTEP", errorCode);
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
     * Exports the trimmed/fused CPACS geometry in VTK format to a local file. The output file
     * includes metadata, i.e. location of each triangle on each segment etc.
     * 
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedGeometryVTK(final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedGeometryVTK(cpacsHandle, fileName, deflection); 
        throwIfError("tiglExportMeshedGeometryVTK", errorCode);
    }
    
    /**
     * Exports the trimmed/fused CPACS geometry in VTK format to a local file. This export does not contain
     * any metadata and is faster to compute.
     * 
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedGeometryVTKSimple(final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedGeometryVTKSimple(cpacsHandle, fileName, deflection); 
        throwIfError("tiglExportMeshedGeometryVTKSimple", errorCode);
    }
    
    /**
     * Exports a wing of a CPACS Geometry as a collada file for the use in Blender.
     * 
     * @param wingUID - The UID of the wing to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection/accuracy of the meshing. (typical value: 0.01)
     * @throws TiglException 
     */
    public void exportWingCollada(final String wingUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();
        
        errorCode = TiglNativeInterface.tiglExportWingColladaByUID(cpacsHandle, wingUID, fileName, deflection);
        throwIfError("tiglExportWingColladaByUID", errorCode);
    }
    
    /**
     * Exports a CPACS fuselage geometry in STL format to a local file.
     * 
     * @param fuselageUID - The UID of the fuselage to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedFuselageSTL(final String fuselageUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedFuselageSTLByUID(cpacsHandle, fuselageUID, fileName, deflection); 
        throwIfError("tiglExportMeshedFuselageSTLByUID", errorCode);
    }
    
    /**
     * Exports a CPACS wing geometry in STL format to a local file.
     * 
     * @param wingUID - The UID of the wing to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedWingSTL(final String wingUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedWingSTLByUID(cpacsHandle, wingUID, fileName, deflection); 
        throwIfError("tiglExportMeshedWingSTLByUID", errorCode);
    }
    
    /**
     * Exports the trimmed/fused CPACS geometry in STL format to a local file.
     * 
     * @param fileName - The filename to save with.
     * @param deflection - The deflection of the meshing.
     * @throws TiglException 
     */
    public void exportMeshedGeometrySTL(final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();

        // export to the file
        errorCode = TiglNativeInterface.tiglExportMeshedGeometrySTL(cpacsHandle, fileName, deflection); 
        throwIfError("tiglExportMeshedGeometrySTL", errorCode);
    }
    
    /**
     * Exports a fuselage of a CPACS Geometry as a collada file for the use in Blender.
     * 
     * @param fuselageUID - The UID of the fuselage to export.
     * @param fileName - The filename to save with.
     * @param deflection - The deflection/accuracy of the meshing. (typical value: 0.01)
     * @throws TiglException 
     */
    public void exportFuselageCollada(final String fuselageUID, final String fileName, final double deflection) throws TiglException {
        checkTiglConfiguration();
        
        errorCode = TiglNativeInterface.tiglExportFuselageColladaByUID(cpacsHandle, fuselageUID, fileName, deflection);
        throwIfError("tiglExportFuselageColladaByUID", errorCode);
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
            LOGGER.error("TiGL: checkTiglConfiguration::Tried to work with a cpacs configuration whose handle is invalid (not loaded/imported before)");
            throw new TiglException("Invalid cpacs handle", TiglReturnCode.TIGL_NOT_FOUND);
        }
    }
    
    private static void throwIfError(String methodname, int errorCode) throws TiglException {
        if (errorCode != TiglReturnCode.TIGL_SUCCESS.getValue()) {
            String message = " In TiGL function \"" + methodname + "."
                    + "\"";
            LOGGER.error("TiGL: Function " + methodname + " returned " + TiglReturnCode.getEnum(errorCode).toString() + ".");
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
