/* 
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
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
/**
* @file
* @date   $Date$
* @brief  Declaration of the TIGL Fortran wrapper interface.
*/

/*
* g77 on Linux, MinGW, IRIX:
* Append one underscore if the name does not contain an underscore otherwise
* append two. All names are in lower case.
*
* HP-UX:
* All names are in lower case.
*
* f90/f77 on IRIX:
* Append one underscore
*
* ifort/cvf
* All names are upper-case
*/

#ifndef TIGL_FORTRAN_H
#define TIGL_FORTRAN_H

#include "tigl.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined G77_LINUX || defined G77_IRIX || defined G77_MINGW || defined G77
    #define FORTRAN_NAME(a) a##__
#elif defined F90_HPUX
    #define FORTRAN_NAME(a) a
#elif defined F90_IRIX || defined GFORTRAN
    #define FORTRAN_NAME(a) a##_
#else
    #define FORTRAN_NAME(a) a
#endif

#ifdef IFORT_WIN32

#define tiglOpenCPACSConfiguration_f TIGL_OPEN_CPACS_CONFIGURATION
#define tiglCloseCPACSConfiguration_f TIGL_CLOSE_CPACS_CONFIGURATION
#define tiglGetCPACSTixiHandle_f TIGL_GET_CPACS_TIXI_HANDLE
#define tiglIsCPACSConfigurationHandleValid_f TIGL_IS_CPACS_CONFIGURATION_HANDLE_VALID
#define tiglGetVersion_f TIGL_GET_VERSION
#define tiglExportIGES_f TIGL_EXPORT_IGES
#define tiglExportFusedIGES_f TIGL_EXPORT_FUSED_WING_FUSELAGE_IGES
#define tiglExportSTEP_f TIGL_EXPORT_STEP
#define tiglExportMeshedWingSTL_f TIGL_EXPORT_MESHED_WING_STL
#define tiglExportMeshedFuselageSTL_f TIGL_EXPORT_MESHED_FUSELAGE_STL
#define tiglExportMeshedGeometrySTL_f TIGL_EXPORT_MESHED_GEOMETRY_STL
#define tiglGetWingCount_f TIGL_GET_WING_COUNT
#define tiglWingGetSegmentCount_f TIGL_WING_GET_SEGMENT_COUNT
#define tiglWingGetComponentSegmentCount_f TIGL_WING_GET_COMPONENT_SEGMENT_COUNT
#define tiglWingGetComponentSegmentUID_f TIGL_WING_GET_COMPONENT_SEGMENT_UID
#define tiglWingGetComponentSegmentIndex_f TIGL_WING_GET_COMPONENT_SEGMENT_INDEX
#define tiglWingGetUpperPoint_f TIGL_WING_GET_UPPER_POINT
#define tiglWingGetLowerPoint_f TIGL_WING_GET_LOWER_POINT
#define tiglWingGetInnerConnectedSegmentCount_f TIGL_WING_GET_INNER_CONNECTED_SEGMENT_COUNT
#define tiglWingGetOuterConnectedSegmentCount_f TIGL_WING_GET_OUTER_CONNECTED_SEGMENT_COUNT
#define tiglWingGetInnerConnectedSegmentIndex_f TIGL_WING_GET_INNER_CONNECTED_SEGMENT_INDEX
#define tiglWingGetOuterConnectedSegmentIndex_f TIGL_WING_GET_OUTER_CONNECTED_SEGMENT_INDEX
#define tiglWingGetInnerSectionAndElementUID_f TIGL_WING_GET_INNER_SECTION_AND_ELEMENT_UID
#define tiglWingGetOuterSectionAndElementUID_f TIGL_WING_GET_OUTER_SECTION_AND_ELEMENT_UID
#define tiglWingGetProfileName_f TIGL_WING_GET_PROFILE_NAME
#define tiglWingGetUID_f TIGL_WING_GET_UID
#define tiglWingGetIndex_f TIGL_WING_GET_INDEX
#define tiglWingGetSegmentUID_f TIGL_WING_GET_SEGMENT_UID
#define tiglWingGetSegmentIndex_f TIGL_WING_GET_SEGMENT_INDEX
#define tiglWingGetSectionUID_f TIGL_WING_GET_SECTION_UID
#define tiglWingGetSymmetry_f TIGL_WING_GET_SYMMETRY
#define tiglWingComponentSegmentGetPoint_f TIGL_WING_COMPONENT_SEGMENT_GET_POINT
#define tiglGetFuselageCount_f TIGL_GET_FUSELAGE_COUNT
#define tiglFuselageGetSegmentCount_f TIGL_FUSELAGE_GET_SEGMENT_COUNT
#define tiglFuselageGetPoint_f TIGL_FUSELAGE_GET_POINT
#define tiglFuselageGetPointAngle_f TIGL_FUSELAGE_GET_POINT_ANGLE
#define tiglFuselageGetPointAngleTranslated_f TIGL_FUSELAGE_GET_POINT_ANGLE_TRANSLATED
#define tiglFuselageGetStartConnectedSegmentCount_f TIGL_FUSELAGE_GET_START_CONNECTED_SEGMENT_COUNT
#define tiglFuselageGetEndConnectedSegmentCount_f TIGL_FUSELAGE_GET_END_CONNECTED_SEGMENT_COUNT
#define tiglFuselageGetStartConnectedSegmentIndex_f TIGL_FUSELAGE_GET_START_CONNECTED_SEGMENT_INDEX
#define tiglFuselageGetEndConnectedSegmentIndex_f TIGL_FUSELAGE_GET_END_CONNECTED_SEGMENT_INDEX
#define tiglFuselageGetStartSectionAndElementUID_f TIGL_FUSELAGE_GET_START_SECTION_AND_ELEMENT_UID
#define tiglFuselageGetEndSectionAndElementUID_f TIGL_FUSELAGE_GET_END_SECTION_AND_ELEMENT_UID
#define tiglFuselageGetSymmetry_f TIGL_FUSELAGE_GET_SYMMETRY
#define tiglFuselageGetVolume_f TIGL_FUSELAGE_GET_VOLUME
#define tiglWingGetVolume_f TIGL_WING_GET_VOLUME
#define tiglWingGetSegmentVolume_f TIGL_WING_GET_SEGMENT_VOLUME
#define tiglFuselageGetSegmentVolume_f TIGL_FUSELAGE_GET_SEGMENT_VOLUME
#define tiglFuselageGetSurfaceArea_f TIGL_FUSELAGE_GET_SURFACE_AREA
#define tiglWingGetSurfaceArea_f TIGL_WING_GET_SURFACE_AREA
#define tiglWingGetSegmentSurfaceArea_f TIGL_WING_GET_SEGMENT_SURFACE_AREA
#define tiglWingGetReferenceArea_f TIGL_WING_GET_REFERENCE_AREA
#define tiglFuselageGetSegmentSurfaceArea_f TIGL_FUSELAGE_GET_SEGMENT_SURFACE_AREA
#define tiglFuselageGetPointOnXPlane_f TIGL_FUSELAGE_GET_POINT_ON_XPLANE
#define tiglFuselageGetPointOnYPlane_f TIGL_FUSELAGE_GET_POINT_ON_YPLANE
#define tiglFuselageGetNumPointsOnYPlane_f TIGL_FUSELAGE_GET_NUM_POINTS_ON_YPLANE
#define tiglFuselageGetNumPointsOnXPlane_f TIGL_FUSELAGE_GET_NUM_POINTS_ON_XPLANE
#define tiglFuselageGetCircumference_f TIGL_FUSELAGE_GET_CIRCUMFERENCE
#define tiglExportMeshedWingVTKByIndex_f TIGL_EXPORT_MESHED_WING_VTK_BY_INDEX
#define tiglExportMeshedWingVTKByUID_f TIGL_EXPORT_MESHED_WING_VTK_BY_UID
#define tiglExportMeshedFuselageVTKByIndex_f TIGL_EXPORT_MESHED_FUSELAGE_VTK_BY_INDEX
#define tiglExportMeshedFuselageVTKByUID_f TIGL_EXPORT_MESHED_FUSELAGE_VTK_BY_UID
#define tiglExportMeshedGeometryVTK_f TIGL_EXPORT_MESHED_GEOMETRY_VTK
#define tiglExportMeshedWingVTKSimpleByUID_f TIGL_EXPORT_MESHED_WING_VTK_SIMPLE_BY_UID
#define tiglExportMeshedFuselageVTKSimpleByUID_f TIGL_EXPORT_MESHED_FUSELAGE_VTK_SIMPLE_BY_UID
#define tiglExportMeshedGeometryVTKSimpleByUID_f TIGL_EXPORT_MESHED_GEOMETRY_VTK_SIMPLE_BY_UID
#define tiglWingComponentSegmentGetMaterialUIDs_f TIGL_WING_COMPONENT_SEGMENT_GET_MATERIAL_UIDS
#define tiglComponentGetHashCode_f TIGL_COMPONENT_GET_HASH_CODE
#define tiglFuselageGetMinumumDistanceToGround_f TIGL_FUSELAGE_GET_MINIMUM_DISTANCE_TO_GROUND
#define tiglWingGetWettedArea_f TIGL_WING_GET_WETTED_AREA
#define tiglGetErrorString_f TIGL_GET_ERROR_STRING

#else

#define tiglOpenCPACSConfiguration_f FORTRAN_NAME(tigl_open_cpacs_configuration)
#define tiglCloseCPACSConfiguration_f FORTRAN_NAME(tigl_close_cpacs_configuration)
#define tiglGetCPACSTixiHandle_f FORTRAN_NAME(tigl_get_cpacs_tixi_handle)
#define tiglIsCPACSConfigurationHandleValid_f FORTRAN_NAME(tigl_is_cpacs_configuration_handle_valid)
#define tiglGetVersion_f FORTRAN_NAME(tigl_get_version)
#define tiglExportIGES_f FORTRAN_NAME(tigl_export_iges)
#define tiglExportSTEP_f FORTRAN_NAME(tigl_export_step)
#define tiglExportFusedIGES_f FORTRAN_NAME(tigl_export_fused_wing_fuselage_iges)
#define tiglExportMeshedWingSTL_f FORTRAN_NAME(tigl_export_meshed_wing_stl)
#define tiglExportMeshedFuselageSTL_f FORTRAN_NAME(tigl_export_meshed_fuselage_stl)
#define tiglExportMeshedGeometrySTL_f FORTRAN_NAME(tigl_export_meshed_geometry_stl)
#define tiglGetWingCount_f FORTRAN_NAME(tigl_get_wing_count)
#define tiglWingGetSegmentCount_f FORTRAN_NAME(tigl_wing_get_segment_count)
#define tiglWingGetComponentSegmentCount_f FORTRAN_NAME(tigl_wing_get_component_segment_count)
#define tiglWingGetComponentSegmentUID_f FORTRAN_NAME(tigl_wing_get_component_segment_uid)
#define tiglWingGetComponentSegmentIndex_f FORTRAN_NAME(tigl_wing_get_component_segment_index)
#define tiglWingGetUpperPoint_f FORTRAN_NAME(tigl_wing_get_upper_point)
#define tiglWingGetLowerPoint_f FORTRAN_NAME(tigl_wing_get_lower_point)
#define tiglWingGetInnerConnectedSegmentCount_f FORTRAN_NAME(tigl_wing_get_inner_connected_segment_count)
#define tiglWingGetOuterConnectedSegmentCount_f FORTRAN_NAME(tigl_wing_get_outer_connected_segment_count)
#define tiglWingGetInnerConnectedSegmentIndex_f FORTRAN_NAME(tigl_wing_get_inner_connected_segment_index)
#define tiglWingGetOuterConnectedSegmentIndex_f FORTRAN_NAME(tigl_wing_get_outer_connected_segment_index)
#define tiglWingGetInnerSectionAndElementUID_f FORTRAN_NAME(tigl_wing_get_inner_section_and_element_uid)
#define tiglWingGetOuterSectionAndElementUID_f FORTRAN_NAME(tigl_wing_get_outer_section_and_element_uid)
#define tiglWingGetProfileName_f FORTRAN_NAME(tigl_wing_get_profile_name)
#define tiglWingGetUID_f FORTRAN_NAME(tigl_wing_get_uid)
#define tiglWingGetIndex_f FORTRAN_NAME(tigl_wing_get_index)
#define tiglWingGetSegmentUID_f FORTRAN_NAME(tigl_wing_get_segment_uid)
#define tiglWingGetSegmentIndex_f FORTRAN_NAME(tigl_wing_get_segment_index)
#define tiglWingGetSectionUID_f FORTRAN_NAME(tigl_wing_get_section_uid)
#define tiglWingGetSymmetry_f FORTRAN_NAME(tigl_wing_get_symmetry)
#define tiglWingComponentSegmentGetPoint_f FORTRAN_NAME(tigl_wing_component_segment_get_point)
#define tiglGetFuselageCount_f FORTRAN_NAME(tigl_get_fuselage_count)
#define tiglFuselageGetSegmentCount_f FORTRAN_NAME(tigl_fuselage_get_segment_count)
#define tiglFuselageGetPoint_f FORTRAN_NAME(tigl_fuselage_get_point)
#define tiglFuselageGetPointAngle_f FORTRAN_NAME(tigl_fuselage_get_point_angle)
#define tiglFuselageGetPointAngleTranslated_f FORTRAN_NAME(tigl_fuselage_get_point_angle_translated)
#define tiglFuselageGetStartConnectedSegmentCount_f FORTRAN_NAME(tigl_fuselage_get_start_connected_segment_count)
#define tiglFuselageGetEndConnectedSegmentCount_f FORTRAN_NAME(tigl_fuselage_get_end_connected_segment_count)
#define tiglFuselageGetStartConnectedSegmentIndex_f FORTRAN_NAME(tigl_fuselage_get_start_connected_segment_index)
#define tiglFuselageGetEndConnectedSegmentIndex_f FORTRAN_NAME(tigl_fuselage_get_end_connected_segment_index)
#define tiglFuselageGetStartSectionAndElementUID_f FORTRAN_NAME(tigl_fuselage_get_start_section_and_element_uid)
#define tiglFuselageGetEndSectionAndElementUID_f FORTRAN_NAME(tigl_fuselage_get_end_section_and_element_uid)
#define tiglFuselageGetSymmetry_f FORTRAN_NAME(tigl_fuselage_get_symmetry)
#define tiglFuselageGetVolume_f FORTRAN_NAME(tigl_fuselage_get_volume)
#define tiglWingGetVolume_f FORTRAN_NAME(tigl_wing_get_volume)
#define tiglWingGetSegmentVolume_f FORTRAN_NAME(tigl_wing_get_segment_volume)
#define tiglFuselageGetSegmentVolume_f FORTRAN_NAME(tigl_fuselage_get_segment_volume)
#define tiglFuselageGetSurfaceArea_f FORTRAN_NAME(tigl_fuselage_get_surface_area)
#define tiglWingGetSurfaceArea_f FORTRAN_NAME(tigl_wing_get_surface_area)
#define tiglWingGetSegmentSurfaceArea_f FORTRAN_NAME(tigl_wing_get_segment_surface_area)
#define tiglWingGetReferenceArea_f FORTRAN_NAME(tigl_wing_get_reference_area)
#define tiglFuselageGetSegmentSurfaceArea_f FORTRAN_NAME(tigl_fuselage_get_segment_surface_area)
#define tiglFuselageGetPointOnXPlane_f FORTRAN_NAME(tigl_fuselage_get_point_on_xplane)
#define tiglFuselageGetPointOnYPlane_f FORTRAN_NAME(tigl_fuselage_get_point_on_yplane)
#define tiglFuselageGetNumPointsOnYPlane_f FORTRAN_NAME(tigl_fuselage_get_num_points_on_yplane)
#define tiglFuselageGetNumPointsOnXPlane_f FORTRAN_NAME(tigl_fuselage_get_num_points_on_xplane)
#define tiglFuselageGetCircumference_f  FORTRAN_NAME(tigl_fuselage_get_circumference)
#define tiglExportMeshedWingVTKByIndex_f FORTRAN_NAME(tigl_export_meshed_wing_vtk_by_index)
#define tiglExportMeshedWingVTKByUID_f FORTRAN_NAME(tigl_export_meshed_wing_vtk_by_uid)
#define tiglExportMeshedFuselageVTKByIndex_f FORTRAN_NAME(tigl_export_meshed_fuselage_vtk_by_index)
#define tiglExportMeshedFuselageVTKByUID_f FORTRAN_NAME(tigl_export_meshed_fuselage_vtk_by_uid)
#define tiglExportMeshedGeometryVTK_f FORTRAN_NAME(tigl_export_meshed_geometry_vtk)
#define tiglExportMeshedWingVTKSimpleByUID_f FORTRAN_NAME(tigl_export_meshed_wing_vtk_simple_by_uid)
#define tiglExportMeshedFuselageVTKSimpleByUID_f FORTRAN_NAME(tigl_export_meshed_fuselage_vtk_simple_by_uid)
#define tiglExportMeshedGeometryVTKSimpleByUID_f FORTRAN_NAME(tigl_export_meshed_geometry_vtk_simple_by_uid)
#define tiglWingComponentSegmentGetMaterialUIDs_f FORTRAN_NAME(tigl_wing_component_segment_get_material_uids)
#define tiglComponentGetHashCode_f FORTRAN_NAME(tigl_component_get_hash_code)
#define tiglFuselageGetMinumumDistanceToGround_f FORTRAN_NAME(tigl_fuselage_get_minumum_distance_to_ground)
#define tiglWingGetWettedArea_f FORTRAN_NAME(tigl_wing_get_wetted_area)
#define tiglGetErrorString_f FORTRAN_NAME(tigl_get_error_string)

#endif /* IFORT_WIN32 */


void tiglOpenCPACSConfiguration_f(TixiDocumentHandle* tixiHandle,
                                  TiglCPACSConfigurationHandle* cpacsHandlePtr,
                                  char* configurationUID,
                                  TiglReturnCode* returnCode);

void tiglCloseCPACSConfiguration_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                   TiglReturnCode* returnCode);

void tiglGetCPACSTixiHandle_f(TiglCPACSConfigurationHandle* cpacsHandle,
                              TixiDocumentHandle* tixiHandlePtr,
                              TiglReturnCode* returnCode);

void tiglIsCPACSConfigurationHandleValid_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                           TiglBoolean* isValidPtr,
                                           TiglReturnCode* returnCode);

void tiglGetVersion_f(char* version,
                      int stringLength );

void tiglExportIGES_f(TiglCPACSConfigurationHandle* cpacsHandle,
                      char* filenamePtr,
                      TiglReturnCode* returnCode,
                      int lengthString1);

void tiglExportSTEP_f(TiglCPACSConfigurationHandle* cpacsHandle,
                      char* filenamePtr,
                      TiglReturnCode* returnCode,
                      int lengthString1);

void tiglExportFusedIGES_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           char* filenamePtr,
                           TiglReturnCode* returnCode,
                           int lengthString1);

void tiglExportMeshedWingSTL_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           int* wingIndex,
                           char* filenamePtr,
                           double* deflection,
                           TiglReturnCode* returnCode,
                           int lengthString1);

void tiglExportMeshedFuselageSTL_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           int* fuselageIndex,
                           char* filenamePtr,
                           double* deflection,
                           TiglReturnCode* returnCode,
                           int lengthString1);

void tiglExportMeshedGeometrySTL_f(TiglCPACSConfigurationHandle* cpacsHandle,
                           char* filenamePtr,
                           double* deflection,
                           TiglReturnCode* returnCode,
                           int lengthString1);

void tiglGetWingCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                        int* wingCountPtr,
                        TiglReturnCode* returnCode);

void tiglWingGetSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                               int* wingIndex,
                               int* segmentCountPtr,
                               TiglReturnCode* returnCode);

void tiglWingGetUpperPoint_f(TiglCPACSConfigurationHandle* cpacsHandle,
                             int* wingIndex,
                             int* segmentIndex,
                             double* eta,
                             double* xsi,
                             double* pointXPtr,
                             double* pointYPtr,
                             double* pointZPtr,
                             TiglReturnCode* returnCode);

void tiglWingGetLowerPoint_f(TiglCPACSConfigurationHandle* cpacsHandle,
                             int* wingIndex,
                             int* segmentIndex,
                             double* eta,
                             double* xsi,
                             double* pointXPtr,
                             double* pointYPtr,
                             double* pointZPtr,
                             TiglReturnCode* returnCode);

void tiglWingGetInnerConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* segmentCountPtr,
                                             TiglReturnCode* returnCode);

void tiglWingGetOuterConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* segmentCountPtr,
                                             TiglReturnCode* returnCode);

void tiglWingGetInnerConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* n,
                                             int* connectedIndexPtr,
                                             TiglReturnCode* returnCode);

void tiglWingGetOuterConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             int* wingIndex,
                                             int* segmentIndex,
                                             int* n,
                                             int* connectedIndexPtr,
                                             TiglReturnCode* returnCode);

void tiglWingGetInnerSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                              int* wingIndex,
                                              int* segmentIndex,
                                              char* sectionUIDPtr,
                                              char* elementUIDPtr,
                                              TiglReturnCode* returnCode);

void tiglWingGetOuterSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                              int* wingIndex,
                                              int* segmentIndex,
                                              char* sectionUIDPtr,
                                              char* elementUIDPtr,
                                              TiglReturnCode* returnCode);

void tiglWingGetProfileName_f(TiglCPACSConfigurationHandle* cpacsHandle,
                              int* wingIndex,
                              int* sectionIndex,
                              int* elementIndex,
                              char* profileNamePtr,
                              TiglReturnCode* returnCode,
                              int lengthString1);

void tiglWingGetUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            char* uidNamePtr,
                            TiglReturnCode* returnCode,
                            int lengthString1);

void tiglWingGetSegmentUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            int* segmentIndex,
                            char* uidNamePtr,
                            TiglReturnCode* returnCode,
                            int lengthString1);

void tiglWingGetSectionUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* wingIndex,
                            int* sectionIndex,
                            char* uidNamePtr,
                            TiglReturnCode* returnCode,
                            int lengthString1);

void tiglWingGetSymmetry_f(TiglCPACSConfigurationHandle *cpacsHandle,
                            int *wingIndex,
                            TiglSymmetryAxis *symmetryAxisPtr,
                            TiglReturnCode* returnCode);

//########################### GetPoint Functions #####################################

void tiglFuselageGetPointOnXPlane_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                   int *fuselageIndex,
                                   int *segmentIndex,
                                   double *eta,
                                   double *xpos,
                                   int *pointIndex,
                                   double* pointXPtr,
                                   double* pointYPtr,
                                   double* pointZPtr,
                                   TiglReturnCode* returnCode);


void tiglFuselageGetPointOnYPlane_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                   int *fuselageIndex,
                                   int *segmentIndex,
                                   double *eta,
                                   double *ypos,
                                   int *pointIndex,
                                   double* pointXPtr,
                                   double* pointYPtr,
                                   double* pointZPtr,
                                   TiglReturnCode* returnCode);

void tiglFuselageGetNumPointsOnYPlane_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                       int *fuselageIndex,
                                       int *segmentIndex,
                                       double *eta,
                                       double *ypos,
                                       int* numPointsPtr,
                                       TiglReturnCode* returnCode);


void tiglFuselageGetNumPointsOnXPlane_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                       int *fuselageIndex,
                                       int *segmentIndex,
                                       double *eta,
                                       double *xpos,
                                       int* numPointsPtr,
                                       TiglReturnCode* returnCode);

void tiglFuselageGetCircumference_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                    int *fuselageIndex,
                                    int *segmentIndex,
                                    double *eta,
                                    double *circumferencePtr,
                                    TiglReturnCode* returnCode);


void tiglFuselageGetPoint_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* fuselageIndex,
                            int* segmentIndex,
                            double* eta,
                            double* zeta,
                            double* pointXPtr,
                            double* pointYPtr,
                            double* pointZPtr,
                            TiglReturnCode* returnCode);


void tiglFuselageGetPointAngle_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                 int* fuselageIndex,
                                 int* segmentIndex,
                                 double* eta,
                                 double* alpha,
                                 double* pointXPtr,
                                 double* pointYPtr,
                                 double* pointZPtr,
                                 TiglReturnCode* returnCode);


void tiglFuselageGetPointAngleTranslated_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                            int* fuselageIndex,
                                            int* segmentIndex,
                                            double* eta,
                                            double* alpha,
                                            double* x_cs,
                                            double* z_cs,
                                            double* pointXPtr,
                                            double* pointYPtr,
                                            double* pointZPtr,
                                            TiglReturnCode* returnCode);


//######################################################################################

void tiglGetFuselageCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                            int* fuselageCountPtr,
                            TiglReturnCode* returnCode);

void tiglFuselageGetSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                   int* fuselageIndex,
                                   int* segmentCountPtr,
                                   TiglReturnCode* returnCode);


void tiglFuselageGetStartConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                 int* fuselageIndex,
                                                 int* segmentIndex,
                                                 int* segmentCountPtr,
                                                 TiglReturnCode* returnCode);

void tiglFuselageGetEndConnectedSegmentCount_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                               int* fuselageIndex,
                                               int* segmentIndex,
                                               int* segmentCountPtr,
                                               TiglReturnCode* returnCode);

void tiglFuselageGetStartConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                 int* fuselageIndex,
                                                 int* segmentIndex,
                                                 int* n,
                                                 int* connectedIndexPtr,
                                                 TiglReturnCode* returnCode);

void tiglFuselageGetEndConnectedSegmentIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                               int* fuselageIndex,
                                               int* segmentIndex,
                                               int* n,
                                               int* connectedIndexPtr,
                                               TiglReturnCode* returnCode);

void tiglFuselageGetStartSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                  int* fuselageIndex,
                                                  int* segmentIndex,
                                                  char* sectionIndexPtr,
                                                  char* elementIndexPtr,
                                                  TiglReturnCode* returnCode);

void tiglFuselageGetEndSectionAndElementUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                                int* fuselageIndex,
                                                int* segmentIndex,
                                                char* sectionIndexPtr,
                                                char* elementIndexPtr,
                                                TiglReturnCode* returnCode);

void tiglFuselageGetSymmetry_f(TiglCPACSConfigurationHandle *cpacsHandle,
                            int *fuselageIndex,
                            TiglSymmetryAxis *symmetryAxisPtr,
                            TiglReturnCode* returnCode);


/*****************************************************************************************************/
/*                     Volume calculations                                                           */
/*****************************************************************************************************/

void tiglFuselageGetVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                             int *fuselageIndex,
                             double *volumePtr,
                             TiglReturnCode* returnCode);

void tiglWingGetVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                             int *wingIndex,
                             double *volumePtr,
                             TiglReturnCode* returnCode);

void tiglWingGetSegmentVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                int *wingIndex,
                                int *segmentIndex,
                                double *volumePtr,
                                TiglReturnCode* returnCode);

void tiglFuselageGetSegmentVolume_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                int *fuselageIndex,
                                int *segmentIndex,
                                double *volumePtr,
                                TiglReturnCode* returnCode);


/*****************************************************************************************************/
/*                     Surface Area calculations                                                     */
/*****************************************************************************************************/

void tiglFuselageGetSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                 int *fuselageIndex,
                                 double *surfaceAreaPtr,
                                 TiglReturnCode* returnCode);

void tiglWingGetSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                 int *wingIndex,
                                 double *surfaceAreaPtr,
                                 TiglReturnCode* returnCode);

void tiglWingGetSegmentSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                    int *wingIndex,
                                    int *segmentIndex,
                                    double *surfaceAreaPtr,
                                    TiglReturnCode* returnCode);

void tiglFuselageGetSegmentSurfaceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                    int *fuselageIndex,
                                    int *segmentIndex,
                                    double *surfaceAreaPtr,
                                    TiglReturnCode* returnCode);

void tiglWingGetReferenceArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                 int *wingIndex,
                                 double *referenceAreaPtr,
                                 TiglReturnCode* returnCode);

 void tiglWingGetWettedArea_f(TiglCPACSConfigurationHandle *cpacsHandle,
                                  char *wingUID,
                                  double *referenceAreaPtr,
                                  TiglReturnCode* returnCode);

/*****************************************************************************************************/
/*                    VTK EXPORT                                                                     */
/*****************************************************************************************************/

void tiglExportMeshedWingVTKByIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                       int* wingIndex,
                                       char* filenamePtr,
                                       double* deflection,
                                       TiglReturnCode* returnCode,
                                       int lengthString1);

void tiglExportMeshedWingVTKByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                       char* wingUID,
                                       char* filenamePtr,
                                       double* deflection,
                                       TiglReturnCode* returnCode,
                                       int lengthString1,
                                       int lengthString2);

void tiglExportMeshedFuselageVTKByIndex_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                           int* fuselageIndex,
                                           char* filenamePtr,
                                           double* deflection,
                                           TiglReturnCode* returnCode,
                                           int lengthString1);

void tiglExportMeshedFuselageVTKByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                        char* fuselageUID,
                                        char* filenamePtr,
                                        double* deflection,
                                        TiglReturnCode* returnCode,
                                        int lengthString1,
                                        int lengthString2);

void tiglExportMeshedGeometryVTK_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                   char* filenamePtr,
                                   double* deflection,
                                   TiglReturnCode* returnCode,
                                   int lengthString1);


void tiglExportMeshedWingVTKSimpleByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                          char* wingUID,
                                          char* filenamePtr,
                                          double* deflection,
                                          TiglReturnCode* returnCode,
                                          int lengthString1,
                                          int lengthString2);

void tiglExportMeshedFuselageVTKSimpleByUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                              char* wingUID,
                                              char* filenamePtr,
                                              double* deflection,
                                              TiglReturnCode* returnCode,
                                              int lengthString1,
                                              int lengthString2);

void tiglExportMeshedGeometryVTKSimple_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                         char* filenamePtr,
                                         double* deflection,
                                         TiglReturnCode* returnCode,
                                         int lengthString1);

/*****************************************************************************************************/
/*                    MATERIAL FUNCTIONS                                                             */
/*****************************************************************************************************/

void tiglGetMaterialUID_f(TiglCPACSConfigurationHandle* cpacsHandle,
                               char* uID,
                               double* eta,
                               double* xsi,
                               char* materialUID,
                               TiglReturnCode* returnCode,
                               int lengthString1);


/*****************************************************************************************************/
/*                     UTILITY FUNCTIONS                                                             */
/*****************************************************************************************************/
void tiglComponentGetHashCode_f(TiglCPACSConfigurationHandle* cpacsHandle,
                               char* uID,
                               int* hashCode,
                               TiglReturnCode* returnCode,
                               int lengthString1);


void tiglFuselageGetMinumumDistanceToGround_f(TiglCPACSConfigurationHandle* cpacsHandle,
                                             char *fuselageUID,
                                             double* axisPntX,
                                             double* axisPntY,
                                             double* axisPntZ,
                                             double* axisDirX,
                                             double* axisDirY,
                                             double* axisDirZ,
                                             double* angle,
                                             double* pointXPtr,
                                             double* pointYPtr,
                                             double* pointZPtr,
                                             TiglReturnCode* returnCode,
                                             int lengthString1);



#ifdef __cplusplus
}
#endif

#endif /* TIGL_FORTRAN_H */

