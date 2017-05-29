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
* This file is automatically created from tigl.h on 2017-05-29.
* If you experience any bugs please contact the authors
*/

package de.dlr.sc.tigl;

import com.sun.jna.*;
import com.sun.jna.ptr.*;


public class TiglNativeInterface {

    static {
        Native.register("TIGL");
    }

    public static native int tiglOpenCPACSConfiguration(int tixiHandle, String configurationUID, IntByReference cpacsHandlePtr);
    public static native int tiglSaveCPACSConfiguration(String configurationUID, int cpacsHandle);
    public static native int tiglCloseCPACSConfiguration(int cpacsHandle);
    public static native int tiglGetCPACSTixiHandle(int cpacsHandle, IntByReference tixiHandlePtr);
    public static native int tiglIsCPACSConfigurationHandleValid(int cpacsHandle, IntByReference isValidPtr);
    public static native String tiglGetVersion();
    public static native int tiglGetWingCount(int cpacsHandle, IntByReference wingCountPtr);
    public static native int tiglWingGetSegmentCount(int cpacsHandle, int wingIndex, IntByReference segmentCountPtr);
    public static native int tiglWingGetComponentSegmentCount(int cpacsHandle, int wingIndex, IntByReference compSegmentCountPtr);
    public static native int tiglWingGetComponentSegmentUID(int cpacsHandle, int wingIndex, int compSegmentIndex, PointerByReference uidNamePtr);
    public static native int tiglWingGetComponentSegmentIndex(int cpacsHandle, int wingIndex, String compSegmentUID, IntByReference segmentIndexPtr);
    public static native int tiglWingGetUpperPoint(int cpacsHandle, int wingIndex, int segmentIndex, double eta, double xsi, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglWingGetLowerPoint(int cpacsHandle, int wingIndex, int segmentIndex, double eta, double xsi, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglWingGetChordPoint(int cpacsHandle, int wingIndex, int segmentIndex, double eta, double xsi, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglWingGetChordNormal(int cpacsHandle, int wingIndex, int segmentIndex, double eta, double xsi, DoubleByReference normalXPtr, DoubleByReference normalYPtr, DoubleByReference normalZPtr);
    public static native int tiglWingGetUpperPointAtDirection(int cpacsHandle, int wingIndex, int segmentIndex, double eta, double xsi, double dirx, double diry, double dirz, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr, DoubleByReference errorDistance);
    public static native int tiglWingGetLowerPointAtDirection(int cpacsHandle, int wingIndex, int segmentIndex, double eta, double xsi, double dirx, double diry, double dirz, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr, DoubleByReference errorDistance);
    public static native int tiglWingGetSegmentEtaXsi(int cpacsHandle, int wingIndex, double pointX, double pointY, double pointZ, IntByReference segmentIndex, DoubleByReference eta, DoubleByReference xsi, IntByReference isOnTop);
    public static native int tiglWingGetInnerConnectedSegmentCount(int cpacsHandle, int wingIndex, int segmentIndex, IntByReference segmentCountPtr);
    public static native int tiglWingGetOuterConnectedSegmentCount(int cpacsHandle, int wingIndex, int segmentIndex, IntByReference segmentCountPtr);
    public static native int tiglWingGetInnerConnectedSegmentIndex(int cpacsHandle, int wingIndex, int segmentIndex, int n, IntByReference connectedIndexPtr);
    public static native int tiglWingGetOuterConnectedSegmentIndex(int cpacsHandle, int wingIndex, int segmentIndex, int n, IntByReference connectedIndexPtr);
    public static native int tiglWingGetInnerSectionAndElementIndex(int cpacsHandle, int wingIndex, int segmentIndex, IntByReference sectionIndexPtr, IntByReference elementIndexPtr);
    public static native int tiglWingGetOuterSectionAndElementIndex(int cpacsHandle, int wingIndex, int segmentIndex, IntByReference sectionIndexPtr, IntByReference elementIndexPtr);
    public static native int tiglWingGetInnerSectionAndElementUID(int cpacsHandle, int wingIndex, int segmentIndex, PointerByReference sectionUIDPtr, PointerByReference elementUIDPtr);
    public static native int tiglWingGetOuterSectionAndElementUID(int cpacsHandle, int wingIndex, int segmentIndex, PointerByReference sectionUIDPtr, PointerByReference elementUIDPtr);
    public static native int tiglWingGetProfileName(int cpacsHandle, int wingIndex, int sectionIndex, int elementIndex, PointerByReference profileNamePtr);
    public static native int tiglWingGetUID(int cpacsHandle, int wingIndex, PointerByReference uidNamePtr);
    public static native int tiglWingGetIndex(int cpacsHandle, String wingUID, IntByReference wingIndexPtr);
    public static native int tiglWingGetSegmentUID(int cpacsHandle, int wingIndex, int segmentIndex, PointerByReference uidNamePtr);
    public static native int tiglWingGetSegmentIndex(int cpacsHandle, String segmentUID, IntByReference segmentIndexPtr, IntByReference wingIndexPtr);
    public static native int tiglWingGetSectionCount(int cpacsHandle, int wingIndex, IntByReference sectionCount);
    public static native int tiglWingGetSectionUID(int cpacsHandle, int wingIndex, int sectionIndex, PointerByReference uidNamePtr);
    public static native int tiglWingGetSymmetry(int cpacsHandle, int wingIndex, IntByReference symmetryAxisPtr);
    public static native int tiglWingComponentSegmentFindSegment(int cpacsHandle, String componentSegmentUID, double x, double y, double z, PointerByReference segmentUID, PointerByReference wingUID);
    public static native int tiglWingComponentSegmentGetPoint(int cpacsHandle, String componentSegmentUID, double eta, double xsi, DoubleByReference x, DoubleByReference y, DoubleByReference z);
    public static native int tiglWingComponentSegmentPointGetEtaXsi(int cpacsHandle, String componentSegmentUID, double pX, double pY, double pZ, DoubleByReference eta, DoubleByReference xsi, DoubleByReference errorDistance);
    public static native int tiglWingComponentSegmentPointGetSegmentEtaXsi(int cpacsHandle, String componentSegmentUID, double eta, double xsi, PointerByReference wingUID, PointerByReference segmentUID, DoubleByReference segmentEta, DoubleByReference segmentXsi);
    public static native int tiglWingSegmentPointGetComponentSegmentEtaXsi(int cpacsHandle, String segmentUID, String componentSegmentUID, double segmentEta, double segmentXsi, DoubleByReference eta, DoubleByReference xsi);
    public static native int tiglWingComponentSegmentGetSegmentIntersection(int cpacsHandle, String componentSegmentUID, String segmentUID, double csEta1, double csXsi1, double csEta2, double csXsi2, double segmentEta, DoubleByReference segmentXsi, IntByReference hasWarning);
    public static native int tiglWingComponentSegmentComputeEtaIntersection(int cpacsHandle, String componentSegmentUID, double csEta1, double csXsi1, double csEta2, double csXsi2, double eta, DoubleByReference xsi, IntByReference hasWarning);
    public static native int tiglWingComponentSegmentGetNumberOfSegments(int cpacsHandle, String componentSegmentUID, IntByReference nsegments);
    public static native int tiglWingComponentSegmentGetSegmentUID(int cpacsHandle, String componentSegmentUID, int segmentIndex, PointerByReference segmentUID);
    public static native int tiglGetFuselageCount(int cpacsHandle, IntByReference fuselageCountPtr);
    public static native int tiglFuselageGetSegmentCount(int cpacsHandle, int fuselageIndex, IntByReference segmentCountPtr);
    public static native int tiglFuselageGetPoint(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, double zeta, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglFuselageGetPointAngle(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, double alpha, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglFuselageGetPointAngleTranslated(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, double alpha, double y_cs, double z_cs, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglFuselageGetPointOnXPlane(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, double xpos, int pointIndex, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglFuselageGetPointOnYPlane(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, double ypos, int pointIndex, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglFuselageGetNumPointsOnXPlane(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, double xpos, IntByReference numPointsPtr);
    public static native int tiglFuselageGetNumPointsOnYPlane(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, double ypos, IntByReference numPointsPtr);
    public static native int tiglFuselageGetCircumference(int cpacsHandle, int fuselageIndex, int segmentIndex, double eta, DoubleByReference circumferencePtr);
    public static native int tiglFuselageGetStartConnectedSegmentCount(int cpacsHandle, int fuselageIndex, int segmentIndex, IntByReference segmentCountPtr);
    public static native int tiglFuselageGetEndConnectedSegmentCount(int cpacsHandle, int fuselageIndex, int segmentIndex, IntByReference segmentCountPtr);
    public static native int tiglFuselageGetStartConnectedSegmentIndex(int cpacsHandle, int fuselageIndex, int segmentIndex, int n, IntByReference connectedIndexPtr);
    public static native int tiglFuselageGetEndConnectedSegmentIndex(int cpacsHandle, int fuselageIndex, int segmentIndex, int n, IntByReference connectedIndexPtr);
    public static native int tiglFuselageGetStartSectionAndElementUID(int cpacsHandle, int fuselageIndex, int segmentIndex, PointerByReference sectionUIDPtr, PointerByReference elementUIDPtr);
    public static native int tiglFuselageGetEndSectionAndElementUID(int cpacsHandle, int fuselageIndex, int segmentIndex, PointerByReference sectionUIDPtr, PointerByReference elementUIDPtr);
    public static native int tiglFuselageGetStartSectionAndElementIndex(int cpacsHandle, int fuselageIndex, int segmentIndex, IntByReference sectionIndexPtr, IntByReference elementIndexPtr);
    public static native int tiglFuselageGetEndSectionAndElementIndex(int cpacsHandle, int fuselageIndex, int segmentIndex, IntByReference sectionIndexPtr, IntByReference elementIndexPtr);
    public static native int tiglFuselageGetProfileName(int cpacsHandle, int fuselageIndex, int sectionIndex, int elementIndex, PointerByReference profileNamePtr);
    public static native int tiglFuselageGetUID(int cpacsHandle, int fuselageIndex, PointerByReference uidNamePtr);
    public static native int tiglFuselageGetIndex(int cpacsHandle, String fuselageUID, IntByReference fuselageIndexPtr);
    public static native int tiglFuselageGetSegmentUID(int cpacsHandle, int fuselageIndex, int segmentIndex, PointerByReference uidNamePtr);
    public static native int tiglFuselageGetSegmentIndex(int cpacsHandle, String segmentUID, IntByReference segmentIndexPtr, IntByReference fuselageIndexPtr);
    public static native int tiglFuselageGetSectionCount(int cpacsHandle, int fuselageIndex, IntByReference sectionCount);
    public static native int tiglFuselageGetSectionUID(int cpacsHandle, int fuselageIndex, int sectionIndex, PointerByReference uidNamePtr);
    public static native int tiglFuselageGetSymmetry(int cpacsHandle, int fuselageIndex, IntByReference symmetryAxisPtr);
    public static native int tiglFuselageGetMinumumDistanceToGround(int cpacsHandle, String fuselageUID, double axisPntX, double axisPntY, double axisPntZ, double axisDirX, double axisDirY, double axisDirZ, double angle, DoubleByReference pointXPtr, DoubleByReference pointYPtr, DoubleByReference pointZPtr);
    public static native int tiglGetRotorCount(int cpacsHandle, IntByReference rotorCountPtr);
    public static native int tiglRotorGetUID(int cpacsHandle, int rotorIndex, PointerByReference uidNamePtr);
    public static native int tiglRotorGetIndex(int cpacsHandle, String rotorUID, IntByReference rotorIndexPtr);
    public static native int tiglRotorGetRadius(int cpacsHandle, int rotorIndex, DoubleByReference radiusPtr);
    public static native int tiglRotorGetReferenceArea(int cpacsHandle, int rotorIndex, DoubleByReference referenceAreaPtr);
    public static native int tiglRotorGetTotalBladePlanformArea(int cpacsHandle, int rotorIndex, DoubleByReference totalBladePlanformAreaPtr);
    public static native int tiglRotorGetSolidity(int cpacsHandle, int rotorIndex, DoubleByReference solidityPtr);
    public static native int tiglRotorGetSurfaceArea(int cpacsHandle, int rotorIndex, DoubleByReference surfaceAreaPtr);
    public static native int tiglRotorGetVolume(int cpacsHandle, int rotorIndex, DoubleByReference volumePtr);
    public static native int tiglRotorGetTipSpeed(int cpacsHandle, int rotorIndex, DoubleByReference tipSpeedPtr);
    public static native int tiglRotorGetRotorBladeCount(int cpacsHandle, int rotorIndex, IntByReference rotorBladeCountPtr);
    public static native int tiglRotorBladeGetWingIndex(int cpacsHandle, int rotorIndex, int rotorBladeIndex, IntByReference wingIndexPtr);
    public static native int tiglRotorBladeGetWingUID(int cpacsHandle, int rotorIndex, int rotorBladeIndex, PointerByReference wingUIDPtr);
    public static native int tiglRotorBladeGetAzimuthAngle(int cpacsHandle, int rotorIndex, int rotorBladeIndex, DoubleByReference azimuthAnglePtr);
    public static native int tiglRotorBladeGetRadius(int cpacsHandle, int rotorIndex, int rotorBladeIndex, DoubleByReference radiusPtr);
    public static native int tiglRotorBladeGetPlanformArea(int cpacsHandle, int rotorIndex, int rotorBladeIndex, DoubleByReference planformAreaPtr);
    public static native int tiglRotorBladeGetSurfaceArea(int cpacsHandle, int rotorIndex, int rotorBladeIndex, DoubleByReference surfaceAreaPtr);
    public static native int tiglRotorBladeGetVolume(int cpacsHandle, int rotorIndex, int rotorBladeIndex, DoubleByReference volumePtr);
    public static native int tiglRotorBladeGetTipSpeed(int cpacsHandle, int rotorIndex, int rotorBladeIndex, DoubleByReference tipSpeedPtr);
    public static native int tiglRotorBladeGetLocalRadius(int cpacsHandle, int rotorIndex, int rotorBladeIndex, int segmentIndex, double eta, DoubleByReference radiusPtr);
    public static native int tiglRotorBladeGetLocalChord(int cpacsHandle, int rotorIndex, int rotorBladeIndex, int segmentIndex, double eta, DoubleByReference chordPtr);
    public static native int tiglRotorBladeGetLocalTwistAngle(int cpacsHandle, int rotorIndex, int rotorBladeIndex, int segmentIndex, double eta, DoubleByReference twistAnglePtr);
    public static native int tiglIntersectComponents(int cpacsHandle, String componentUidOne, String componentUidTwo, PointerByReference intersectionID);
    public static native int tiglIntersectWithPlane(int cpacsHandle, String componentUid, double px, double py, double pz, double nx, double ny, double nz, PointerByReference intersectionID);
    public static native int tiglIntersectGetLineCount(int cpacsHandle, String intersectionID, IntByReference lineCount);
    public static native int tiglIntersectGetPoint(int cpacsHandle, String intersectionID, int lineIdx, double eta, DoubleByReference pointX, DoubleByReference pointY, DoubleByReference pointZ);
    public static native int tiglExportIGES(int cpacsHandle, String filenamePtr);
    public static native int tiglExportFusedWingFuselageIGES(int cpacsHandle, String filenamePtr);
    public static native int tiglExportSTEP(int cpacsHandle, String filenamePtr);
    public static native int tiglExportFusedSTEP(int cpacsHandle, String filenamePtr);
    public static native int tiglExportMeshedWingSTL(int cpacsHandle, int wingIndex, String filenamePtr, double deflection);
    public static native int tiglExportMeshedWingSTLByUID(int cpacsHandle, String wingUID, String filenamePtr, double deflection);
    public static native int tiglExportMeshedFuselageSTL(int cpacsHandle, int fuselageIndex, String filenamePtr, double deflection);
    public static native int tiglExportMeshedFuselageSTLByUID(int cpacsHandle, String fuselageUID, String filenamePtr, double deflection);
    public static native int tiglExportMeshedGeometrySTL(int cpacsHandle, String filenamePtr, double deflection);
    public static native int tiglExportMeshedWingVTKByIndex(int cpacsHandle, int wingIndex, String filenamePtr, double deflection);
    public static native int tiglExportMeshedWingVTKByUID(int cpacsHandle, String wingUID, String filenamePtr, double deflection);
    public static native int tiglExportMeshedFuselageVTKByIndex(int cpacsHandle, int fuselageIndex, String filenamePtr, double deflection);
    public static native int tiglExportMeshedFuselageVTKByUID(int cpacsHandle, String fuselageUID, String filenamePtr, double deflection);
    public static native int tiglExportMeshedGeometryVTK(int cpacsHandle, String filenamePtr, double deflection);
    public static native int tiglExportMeshedWingVTKSimpleByUID(int cpacsHandle, String wingUID, String filenamePtr, double deflection);
    public static native int tiglExportMeshedFuselageVTKSimpleByUID(int cpacsHandle, String fuselageUID, String filenamePtr, double deflection);
    public static native int tiglExportMeshedGeometryVTKSimple(int cpacsHandle, String filenamePtr, double deflection);
    public static native int tiglExportFuselageColladaByUID(int cpacsHandle, String fuselageUID, String filename, double deflection);
    public static native int tiglExportWingColladaByUID(int cpacsHandle, String wingUID, String filename, double deflection);
    public static native int tiglExportFusedBREP(int cpacsHandle, String filename);
    public static native int tiglWingComponentSegmentGetMaterialCount(int cpacsHandle, String compSegmentUID, int structureType, double eta, double xsi, IntByReference materialCount);
    public static native int tiglWingComponentSegmentGetMaterialUID(int cpacsHandle, String compSegmentUID, int structureType, double eta, double xsi, int materialIndex, PointerByReference uid);
    public static native int tiglWingComponentSegmentGetMaterialThickness(int cpacsHandle, String compSegmentUID, int structureType, double eta, double xsi, int materialIndex, DoubleByReference thickness);
    public static native int tiglFuselageGetVolume(int cpacsHandle, int fuselageIndex, DoubleByReference volumePtr);
    public static native int tiglWingGetVolume(int cpacsHandle, int wingIndex, DoubleByReference volumePtr);
    public static native int tiglWingGetSegmentVolume(int cpacsHandle, int wingIndex, int segmentIndex, DoubleByReference volumePtr);
    public static native int tiglFuselageGetSegmentVolume(int cpacsHandle, int fuselageIndex, int segmentIndex, DoubleByReference volumePtr);
    public static native int tiglWingGetSurfaceArea(int cpacsHandle, int wingIndex, DoubleByReference surfaceAreaPtr);
    public static native int tiglFuselageGetSurfaceArea(int cpacsHandle, int fuselageIndex, DoubleByReference surfaceAreaPtr);
    public static native int tiglWingGetSegmentSurfaceArea(int cpacsHandle, int wingIndex, int segmentIndex, DoubleByReference surfaceAreaPtr);
    public static native int tiglWingGetSegmentUpperSurfaceAreaTrimmed(int cpacsHandle, int wingIndex, int segmentIndex, double eta1, double xsi1, double eta2, double xsi2, double eta3, double xsi3, double eta4, double xsi4, DoubleByReference surfaceArea);
    public static native int tiglWingGetSegmentLowerSurfaceAreaTrimmed(int cpacsHandle, int wingIndex, int segmentIndex, double eta1, double xsi1, double eta2, double xsi2, double eta3, double xsi3, double eta4, double xsi4, DoubleByReference surfaceArea);
    public static native int tiglFuselageGetSegmentSurfaceArea(int cpacsHandle, int fuselageIndex, int segmentIndex, DoubleByReference surfaceAreaPtr);
    public static native int tiglWingGetReferenceArea(int cpacsHandle, int wingIndex, int symPlane, DoubleByReference referenceAreaPtr);
    public static native int tiglWingGetWettedArea(int cpacsHandle, String wingUID, DoubleByReference wettedAreaPtr);
    public static native int tiglProfileGetBSplineCount(int cpacsHandle, String profileUID, IntByReference curveCount);
    public static native int tiglProfileGetBSplineDataSizes(int cpacsHandle, String profileUID, int curveid, IntByReference degree, IntByReference nControlPoints, IntByReference nKnots);
    public static native int tiglProfileGetBSplineData(int cpacsHandle, String profileUID, int curveid, int nControlPoints, Pointer cpx, Pointer cpy, Pointer cpz, int nKnots, Pointer knots);
    public static native int tiglLogToFileEnabled(String filePrefix);
    public static native int tiglLogToFileDisabled();
    public static native int tiglLogSetFileEnding(String ending);
    public static native int tiglLogSetTimeInFilenameEnabled(int enabled);
    public static native int tiglLogSetVerbosity(int level);
    public static native int tiglComponentGetHashCode(int cpacsHandle, String componentUID, IntByReference hashCodePtr);
    public static native String tiglGetErrorString(int errorCode);
    public static native int tiglConfigurationGetLength(int cpacsHandle, DoubleByReference pLength);
    public static native int tiglWingGetSpan(int cpacsHandle, String wingUID, DoubleByReference pSpan);
    public static native int tiglWingGetMAC(int cpacsHandle, String wingUID, DoubleByReference mac_chord, DoubleByReference mac_x, DoubleByReference mac_y, DoubleByReference mac_z);
};
