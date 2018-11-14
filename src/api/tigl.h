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
* @brief  Declaration of the TIGL C interface.
*/

#ifdef __cplusplus
extern "C" {
#endif


#ifndef TIGL_H
#define TIGL_H

#include "tixi.h"
#include <stdio.h>

#if defined(WIN32)
  #if defined (tigl3_EXPORTS)
    #define TIGL_COMMON_EXPORT __declspec (dllexport)
  #else
    #define TIGL_COMMON_EXPORT
  #endif
#else
    #define TIGL_COMMON_EXPORT
#endif

/**
 \defgroup Enums Enumerations
 */

/**
 \ingroup Enums
 Definition of possible error return codes of some functions.
 */

/* If these values are changed, please also modify tigl_error_strings.h */
enum TiglReturnCode
{
    TIGL_SUCCESS             = 0,
    TIGL_ERROR               = 1,
    TIGL_NULL_POINTER        = 2,
    TIGL_NOT_FOUND           = 3,
    TIGL_XML_ERROR           = 4,
    TIGL_OPEN_FAILED         = 5,
    TIGL_CLOSE_FAILED        = 6,
    TIGL_INDEX_ERROR         = 7,
    TIGL_STRING_TRUNCATED    = 8,
    TIGL_WRONG_TIXI_VERSION  = 9,
    TIGL_UID_ERROR           = 10,
    TIGL_WRONG_CPACS_VERSION = 11,
    TIGL_UNINITIALIZED       = 12,
    TIGL_MATH_ERROR          = 13,
    TIGL_WRITE_FAILED        = 14
};

/**
* @brief Definition of error return type.
*
* Possible values are:
*
* - TIGL_SUCCESS
* - TIGL_ERROR
* - TIGL_NULL_POINTER
* - TIGL_NOT_FOUND
* - TIGL_XML_ERROR
* - TIGL_OPEN_FAILED
* - TIGL_CLOSE_FAILED
* - TIGL_INDEX_ERROR
* - TIGL_STRING_TRUNCATED
* - TIGL_WRONG_TIXI_VERSION
* - TIGL_UID_ERROR
*
*/
typedef enum TiglReturnCode TiglReturnCode;

/**
 \ingroup Enums
 Definition of possible logging levels
*/
enum TiglLogLevel 
{
    TILOG_SILENT   =0, /*!< No messages are printed. TiGL is completely silent, even in case of errors. */
    TILOG_ERROR    =1, /*!< Only error messages are printed. */
    TILOG_WARNING  =2, /*!< Only errors and warnings are printed on console. This is the default log level of TiGL. */
    TILOG_INFO     =3, /*!< In addition to TILOG_WANING, also informative messages are printed. */
    TILOG_DEBUG    =4, /*!< Also debug messages are printed. Enable this if you want to track down potential errors in TiGL. */
    TILOG_DEBUG1   =5, /*!< This level is only interesting for TiGL developers */
    TILOG_DEBUG2   =6, /*!< This level is only interesting for TiGL developers */
    TILOG_DEBUG3   =7, /*!< This level is only interesting for TiGL developers */
    TILOG_DEBUG4   =8  /*!< This level is only interesting for TiGL developers */
};

/**
* @brief Definition of logging levels
*
* Possible values are:
*
* - TILOG_SILENT
* - TILOG_ERROR
* - TILOG_WARNING
* - TILOG_INFO
* - TILOG_DEBUG
* - TILOG_DEBUG1
* - TILOG_DEBUG2
* - TILOG_DEBUG3
* - TILOG_DEBUG4
*
*/
typedef enum TiglLogLevel TiglLogLevel;

/**
 \ingroup Enums
  Definition of boolean values used in TIGL.
*/
enum TiglBoolean
{
    TIGL_FALSE = 0,
    TIGL_TRUE  = 1
};

/**
* @brief Definition of boolean type used in TIGL.
*
* Possible values are:
*
* - TIGL_FALSE
* - TIGL_TRUE
*
*/
typedef enum TiglBoolean TiglBoolean;


/**
 \ingroup Enums
  Definition of Symmetry Axis used in TIGL.
*/
enum TiglSymmetryAxis
{
    TIGL_NO_SYMMETRY = 0,
    TIGL_X_Y_PLANE   = 1,
    TIGL_X_Z_PLANE   = 2,
    TIGL_Y_Z_PLANE   = 3
};

/**
* @brief Definition of Symmetry Axis used in TIGL.
*
* Possible values are:
*
*   - TIGL_NO_SYMMETRY
*   - TIGL_X_Y_PLANE
*   - TIGL_X_Z_PLANE
*   - TIGL_Y_Z_PLANE
*
*/
typedef enum TiglSymmetryAxis TiglSymmetryAxis;

/**
 \ingroup Enums 
  Definition of possible algorithms used in calculations. Use
  these constants to define the algorithm to be used e.g. in interpolations.
*/
enum TiglAlgorithmCode
{
    TIGL_INTERPOLATE_LINEAR_WIRE  = 0,         /**< Use a linear interpolation between the points of a wire */
    TIGL_INTERPOLATE_BSPLINE_WIRE = 1,         /**< Use a BSpline interpolation between the points of a wire */
    TIGL_APPROXIMATE_BSPLINE_WIRE = 2          /**< Use a BSpline approximation for the points of a wire */
};


/**
 \ingroup Enums 
  Definition of possible types for geometric components. Used for calculations where
  the type if the component changes the way of behavior.
*/
typedef unsigned int TiglGeometricComponentType;

#define  TIGL_COMPONENT_PHYSICAL          1        /**< A phyisical component like a fuselage, wing, nacelle, something you could touch */
#define  TIGL_COMPONENT_LOGICAL           2        /**< A logical component, like a wing segment */
#define  TIGL_COMPONENT_PLANE             4        /**< The whole aircraft */
#define  TIGL_COMPONENT_FUSELAGE          8        /**< The Component is a fuselage */
#define  TIGL_COMPONENT_WING              16       /**< The Component is a wing */
#define  TIGL_COMPONENT_SEGMENT           32       /**< The Component is a general segment */
#define  TIGL_COMPONENT_WINGSEGMENT       64       /**< The Component is a wing segment */
#define  TIGL_COMPONENT_FUSELSEGMENT      128      /**< The Component is a fuselage segment */
#define  TIGL_COMPONENT_WINGCOMPSEGMENT   256      /**< The Component is a wing component segment */
#define  TIGL_COMPONENT_WINGSHELL         512      /**< The Component is a face of the wing (e.g. upper wing surface) */
#define  TIGL_COMPONENT_GENERICSYSTEM     1024     /**< The Component is a generic system */
#define  TIGL_COMPONENT_ROTOR             2048     /**< The Component is a rotor */
#define  TIGL_COMPONENT_ROTORBLADE        4096     /**< The Component is a rotor blade */
#define  TIGL_COMPONENT_PRESSURE_BULKHEAD 8192     /**< The Component is a pressure bulkhead */
#define  TIGL_COMPONENT_CROSS_BEAM_STRUT  16384    /**< The Component is a cross beam strut */

#define  TIGL_COMPONENT_CARGO_DOOR        32768    /**< The Component is a cargo door */

#define  TIGL_COMPONENT_LONG_FLOOR_BEAM   65536    /**< The Component is a long floor beam */
#define  TIGL_COMPONENT_ENGINE_PYLON      131072   /**< The Component is a engine pylon */

enum TiglStructureType 
{
    UPPER_SHELL = 0,
    LOWER_SHELL = 1,
    INNER_STRUCTURE = 2
};

typedef enum TiglStructureType TiglStructureType;


enum TiglLoftSide
{
    UPPER_SIDE = 0,
    LOWER_SIDE = 1
};

typedef enum TiglLoftSide TiglLoftSide;


enum TiglCoordinateSystem
{
    GLOBAL_COORDINATE_SYSTEM   = 0,
    WING_COORDINATE_SYSTEM     = 1,
    FUSELAGE_COORDINATE_SYSTEM = 2
};

typedef enum TiglCoordinateSystem TiglCoordinateSystem;

enum TiglShapeModifier
{
    UNMODIFIED_SHAPE   = 0,
    SHARP_TRAILINGEDGE = 1,
    BLUNT_TRAILINGEDGE = 2
};

typedef enum TiglShapeModifier TiglShapeModifier;

enum TiglContinuity
{
    C0 = 0,
    C1 = 1,
    C2 = 2
};

typedef enum TiglContinuity TiglContinuity;

/**
* @brief Typedef for possible algorithm types used in calculations.
*
* Possible values for variables of type TiglAlgorithmCode are:
*
* - TIGL_INTERPOLATE_LINEAR_WIRE:  Use a linear interpolation between the points of a wire.
* - TIGL_INTERPOLATE_BSPLINE_WIRE: Use a BSpline interpolation between the points of a wire.
* - TIGL_APPROXIMATE_BSPLINE_WIRE: Use a BSpline approximation for the points of a wire.
*/
typedef enum TiglAlgorithmCode TiglAlgorithmCode;

/**
* @brief Datatype for a CPACS configuration handle.
*/
typedef int TiglCPACSConfigurationHandle;


/**
 \ingroup Enums
  Definition of the different file formats used for import/export used in TIGL.
*/
enum TiglImportExportFormat
{
    TIGL_IMPORTEXPORT_IGES = 0,            /**< Use IGES format for geometry import/export */
    TIGL_IMPORTEXPORT_STEP = 1,            /**< Use STEP format for geometry import/export */
    TIGL_IMPORTEXPORT_STL  = 2,            /**< Use STL format for geometry import/export */
    TIGL_IMPORTEXPORT_VTK  = 3             /**< Use VTK (XML/VTP) format for geometry import/export */
};

typedef const char** TiglStringList;


/**
* @brief Definition of the different file formats used for import/export used in TIGL.
*
* Possible values are:
*
* - TIGL_IMPORTEXPORT_IGES: Use IGES format for geometry import/export.
* - TIGL_IMPORTEXPORT_STEP: Use STEP format for geometry import/export.
* - TIGL_IMPORTEXPORT_STL:  Use STL format for geometry import/export.
* - TIGL_IMPORTEXPORT_VTK:  Use VTK format for geometry import/export.
*
*/
typedef enum TiglImportExportFormat TiglImportExportFormat;


/**
  \defgroup GeneralFunctions General TIGL handling functions
    Function to open, create, and close CPACS-files.
 */
/*@{*/
/**
* @brief Opens a CPACS configuration and builds up the data and geometry structure in memory.
*
*
* @param[in]  tixiHandle     Handle to a TIXI document. The TIXI document should not be closed until the
*                              CPACS configuration is closed. First close the CPACS configuration, then
*                              the TIXI document.
* @param[in]  configurationUID The UID of the configuration that should be loaded by TIGL. Could be NULL or
*                                  an empty string if the data set contains only one configuration.
* @param[out] cpacsHandlePtr Handle to the CPACS configuration. This handle is used in calls to other TIGL functions.
*
* @return
*   - TIGL_SUCCESS if successfully opened the CPACS configuration
*   - TIGL_XML_ERROR if file is not well-formed or another XML error occurred
*   - TIGL_OPEN_FAILED if some other error occurred
*   - TIGL_NULL_POINTER if cpacsHandlePtr is an invalid null pointer
*   - TIGL_ERROR if some other kind of error occurred
*   - TIGL_INVALID_UID is the UID does not exist or an error orrcured with this configuration
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglOpenCPACSConfiguration(TixiDocumentHandle tixiHandle, const char* configurationUID, TiglCPACSConfigurationHandle* cpacsHandlePtr);

/**
* @brief Writes a CPACS configuration based on the data and geometry structure in memory.
*
*
* @param[in] configurationUID The UID of the configuration that should be written.
* @param[in] cpacsHandle Handle to the CPACS configuration. This handle is used in calls to other TIGL functions.
*
* @return
*   - TIGL_SUCCESS if the CPACS configuration was successfully written
*   - TIGL_NULL_POINTER if cpacsHandle is an invalid null pointer
*   - TIGL_UNINITIALIZED if cpacsHandle is not managed by the CCPACSConfigurationManager
*   - TIGL_ERROR if some other kind of error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglSaveCPACSConfiguration(const char* configurationUID, TiglCPACSConfigurationHandle cpacsHandle);

/**
* @brief Closes a CPACS configuration and cleans up all memory used by the configuration.
*        After closing a configuration the associated configuration handle is no longer valid.
*        When the CPACS configuration has been closed, the companion tixi document can also be closed.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration.
*
* @return
*   - TIGL_SUCCESS if successfully opened the CPACS configuration file
*   - TIGL_CLOSE_FAILED if closing of the CPACS configuration failed
*   - TIGL_NOT_FOUND if handle ist not found in handle container
*   - TIGL_ERROR if some other kind of error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglCloseCPACSConfiguration(TiglCPACSConfigurationHandle cpacsHandle);

/**
* @brief Returns the underlying TixiDocumentHandle for a given CPACS configuration handle.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[out] tixiHandlePtr Handle for the TIXI document associated with the CPACS configuration
*
* @return
*   - TIGL_SUCCESS if the TIXI handle was found
*   - TIGL_NOT_FOUND if the TIXI handle was not found
*   - TIGL_NULL_POINTER if tixiHandlePtr is an invalid null pointer
*   - TIGL_ERROR if some other kind of error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetCPACSTixiHandle(TiglCPACSConfigurationHandle cpacsHandle, TixiDocumentHandle* tixiHandlePtr);

/**
* @brief Checks if a given CPACS configuration handle is a valid.
*
*
* @param[in]  cpacsHandle CPACS configuration handle to check
* @param[out] isValidPtr  Contains TIGL_TRUE, if handle is valid, otherwise contains TIGL_FALSE
*
* @return
*   - TIGL_SUCCESS if no error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglIsCPACSConfigurationHandleValid(TiglCPACSConfigurationHandle cpacsHandle, TiglBoolean* isValidPtr);


/**
    @brief Returns the version number of this TIGL version.

    @return
        - char* A string with the version number.
*/
TIGL_COMMON_EXPORT const char* tiglGetVersion();


/*@}*/
/*****************************************************************************************************/

/**
  \defgroup WingFunctions Functions for wing calculations
    Function to handle wing geometry's with TIGL.
 */
/*@{*/

/**
* @brief Returns the number of wings in a CPACS configuration.
*
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[out] wingCountPtr Pointer to the number of wings
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if wingCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetWingCount(TiglCPACSConfigurationHandle cpacsHandle,
                                           int* wingCountPtr);

/**
* @brief Returns the number of segments for a wing in a CPACS configuration.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of the wing, starting at 1
* @param[out] segmentCountPtr Pointer to the number of segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                  int wingIndex,
                                                  int* segmentCountPtr);

/**
* @brief Returns the number of component segments for a wing in a CPACS configuration.
*
*
* @param[in]  cpacsHandle         Handle for the CPACS configuration
* @param[in]  wingIndex           The index of the wing, starting at 1
* @param[out] compSegmentCountPtr Pointer to the number of component segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is not valid
*   - TIGL_NULL_POINTER if compSegmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                  int wingIndex,
                                                  int* compSegmentCountPtr);



/**
* @brief Returns the UID of a component segment of a wing. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle      Handle for the CPACS configuration
* @param[in]  wingIndex        The index of a wing, starting at 1
* @param[in]  compSegmentIndex The index of a segment, starting at 1
* @param[out] uidNamePtr       The uid of the wing
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglWingGetComponentSegmentUID(cpacsHandle, wing, segmentID, &uidPtr);
   printf("The UID of the component segment of wing %d is %s\n", wing, uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or the compSegmentIndex are not valid
*   - TIGL_NULL_POINTER if uidNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 int compSegmentIndex,
                                                 char** uidNamePtr);


/**
* @brief Returns the Index of a component segment of a wing. 
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  compSegmentUID  The uid of the wing
* @param[out] segmentIndexPtr  The index of a segment, starting at 1
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int segmentIndex;
   returnCode = tiglWingGetComponentSegmentIndex(cpacsHandle, wing, uidName, &segmentIndex);
   printf("The Index of the component segment of wing %d is %d\n", wing, segmentIndex);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is not valid
*   - TIGL_UID_ERROR if the compSegmentUID does not exist
*   - TIGL_NULL_POINTER if compSegmentUID is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetComponentSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 const char * compSegmentUID,
                                                 int * segmentIndexPtr);


/**
* @brief Returns a point on the upper wing surface for a
* a given wing and segment index.
*
* Returns a point on the upper wing surface in dependence of parameters eta and xsi,
* which range from 0.0 to 1.0. For eta = 0.0, xsi = 0.0 the point is equal to
* the leading edge on the inner section of the given segment. For eta = 1.0, xsi = 1.0
* the point is equal to the trailing edge on the outer section of the given segment. The
* point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  wingIndex    The index of the wing, starting at 1
* @param[in]  segmentIndex The index of the segment of the wing, starting at 1
* @param[in]  eta          eta in the range 0.0 <= eta <= 1.0; eta = 0 for inner section , eta = 1 for outer section
* @param[in]  xsi          xsi in the range 0.0 <= xsi <= 1.0; xsi = 0 for Leading Edge, xsi =  1 for Trailing Edge
* @param[out] pointXPtr    Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr    Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr    Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetUpperPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double xsi,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr);
        
/**
* @brief Returns a point on the lower wing surface for a
* a given wing and segment index.
*
* Returns a point on the lower wing surface in dependence of parameters eta and xsi,
* which range from 0.0 to 1.0. For eta = 0.0, xsi = 0.0 the point is equal to
* the leading edge on the inner section of the given segment. For eta = 1.0, xsi = 1.0
* the point is equal to the trailing edge on the outer section of the given segment. The
* point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  wingIndex    The index of the wing, starting at 1
* @param[in]  segmentIndex The index of the segment of the wing, starting at 1
* @param[in]  eta          eta in the range 0.0 <= eta <= 1.0
* @param[in]  xsi          xsi in the range 0.0 <= xsi <= 1.0
* @param[out] pointXPtr    Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr    Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr    Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no intersection point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occured
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetLowerPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double xsi,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr);

/**
* @brief Returns a point on the wing chord surface for a
* a given wing and segment index.
*
* Returns a point on the wing chord surface in dependence of parameters eta and xsi,
* which range from 0.0 to 1.0. For eta = 0.0, xsi = 0.0 the point is equal to
* the leading edge on the inner section of the given segment. For eta = 1.0, xsi = 1.0
* the point is equal to the trailing edge on the outer section of the given segment. The
* point is returned in absolute world coordinates.
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  wingIndex    The index of the wing, starting at 1
* @param[in]  segmentIndex The index of the segment of the wing, starting at 1
* @param[in]  eta          eta in the range 0.0 <= eta <= 1.0
* @param[in]  xsi          xsi in the range 0.0 <= xsi <= 1.0
* @param[out] pointXPtr    Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr    Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr    Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if cpacs handle is not valid
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occured
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetChordPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double xsi,
                                                        double* pointXPtr,
                                                        double* pointYPtr,
                                                        double* pointZPtr);

/**
* @brief Returns a normal vector on the wing chord surface for a
* a given wing and segment index.
*
* Returns a normal vector on the wing chord surface in dependence of parameters eta and xsi,
* which range from 0.0 to 1.0.
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  wingIndex    The index of the wing, starting at 1
* @param[in]  segmentIndex The index of the segment of the wing, starting at 1
* @param[in]  eta          eta in the range 0.0 <= eta <= 1.0
* @param[in]  xsi          xsi in the range 0.0 <= xsi <= 1.0
* @param[out] normalXPtr   Pointer to the x-coordinate of the resulting normal vector
* @param[out] normalYPtr   Pointer to the y-coordinate of the resulting normal vector
* @param[out] normalZPtr   Pointer to the z-coordinate of the resulting normal vector
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if cpacs handle is not valid
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if normalXPtr, normalYPtr or normalZPtr are null pointers
*   - TIGL_ERROR if some other error occured
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetChordNormal(TiglCPACSConfigurationHandle cpacsHandle,
                                                         int wingIndex,
                                                         int segmentIndex,
                                                         double eta,
                                                         double xsi,
                                                         double* normalXPtr,
                                                         double* normalYPtr,
                                                         double* normalZPtr);

/**
* @brief Returns a point on the upper wing surface for a
* a given wing and segment index. This function is different from ::tiglWingGetUpperPoint: 
* First, a point on the wing chord surface is computed (defined by segment index and eta,xsi).
* Then, a line is constructed, which is defined by this point and a direction given by the user.
* The intersection of this line with the upper wing surface is finally returned.
* The point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  wingIndex    The index of the wing, starting at 1
* @param[in]  segmentIndex The index of the segment of the wing, starting at 1
* @param[in]  eta          eta in the range 0.0 <= eta <= 1.0; eta = 0 for inner section , eta = 1 for outer section
* @param[in]  xsi          xsi in the range 0.0 <= xsi <= 1.0; xsi = 0 for Leading Edge, xsi =  1 for Trailing Edge
* @param[in]  dirx         X-component of the direction vector.
* @param[in]  diry         Y-component of the direction vector.
* @param[in]  dirz         Z-component of the direction vector.
* @param[out] pointXPtr    Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr    Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr    Pointer to the z-coordinate of the point in absolute world coordinates
* @param[out] errorDistance If the upper surface is missed by the line, the absolute distance between line and 
*                           the nearest point on the surface is returned. The distance is zero in case of successful intersection.
*                           <b>It's up to the user to decide, if the distance is too large and the result has
*                           to be treated as an error.</b>
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no intersection point was found or the cpacs handle is not valid
*   - TIGL_MATH_ERROR if the given direction is a null vector (which has zero length)
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetUpperPointAtDirection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double xsi,
                                                                   double dirx,
                                                                   double diry,
                                                                   double dirz,
                                                                   double* pointXPtr,
                                                                   double* pointYPtr,
                                                                   double* pointZPtr,
                                                                   double* errorDistance);
               
/**
* @brief Returns a point on the lower wing surface for a
* a given wing and segment index. This function is different from ::tiglWingGetLowerPoint: 
* First, a point on the wing chord surface is computed (defined by segment index and eta,xsi).
* Then, a line is constructed, which is defined by this point and a direction given by the user.
* The intersection of this line with the lower wing surface is finally returned.
* The point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  wingIndex    The index of the wing, starting at 1
* @param[in]  segmentIndex The index of the segment of the wing, starting at 1
* @param[in]  eta          eta in the range 0.0 <= eta <= 1.0; eta = 0 for inner section , eta = 1 for outer section
* @param[in]  xsi          xsi in the range 0.0 <= xsi <= 1.0; xsi = 0 for Leading Edge, xsi =  1 for Trailing Edge
* @param[in]  dirx         X-component of the direction vector.
* @param[in]  diry         Y-component of the direction vector.
* @param[in]  dirz         Z-component of the direction vector.
* @param[out] pointXPtr    Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr    Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr    Pointer to the z-coordinate of the point in absolute world coordinates
* @param[out] errorDistance If the lower surface is missed by the line, the absolute distance between line and 
*                           the nearest point on the surface is returned. The distance is zero in case of successful intersection.
*                           <b>It's up to the user to decide, if the distance is too large and the result has
*                           to be treated as an error.</b>
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetLowerPointAtDirection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int wingIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double xsi,
                                                                   double dirx,
                                                                   double diry,
                                                                   double dirz,
                                                                   double* pointXPtr,
                                                                   double* pointYPtr,
                                                                   double* pointZPtr,
                                                                   double* errorDistance);
               
/**
* @brief Inverse function to tiglWingGetLowerPoint and tiglWingGetLowerPoint. Calculates to a point (x,y,z)
* in global coordinates the wing segment coordinates and the wing segment index.
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  wingIndex    The index of the wing, starting at 1
* @param[in]  pointX       X-Coordinate of the global point
* @param[in]  pointY       Y-Coordinate of the global point
* @param[in]  pointZ       Z-Coordinate of the global point
* @param[out] segmentIndex The index of the segment of the wing, starting at 1
* @param[out] eta          Eta value in segment coordinates
* @param[out] xsi          Xsi value in segment coordinates
* @param[out] isOnTop      isOnTop is 1, if the point lies on the upper wing face, else 0.
*
* @return
*   - TIGL_SUCCESS if a solution was found
*   - TIGL_NOT_FOUND if the point does not lie on the wing
*   - TIGL_INDEX_ERROR if wingIndex is not valid
*   - TIGL_NULL_POINTER if eta, xsi or isOnTop are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int wingIndex,
                                                           double pointX,
                                                           double pointY,
                                                           double pointZ,
                                                           int* segmentIndex,
                                                           double* eta,
                                                           double* xsi,
                                                           int* isOnTop);

/**
* @brief Returns the count of wing segments connected to the inner section of a given segment.
*
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] segmentCountPtr Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int* segmentCountPtr);
        
/**
* @brief Returns the count of wing segments connected to the outer section of a given segment.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] segmentCountPtr Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int* segmentCountPtr);
        
/**
* @brief Returns the index (number) of the n-th wing segment connected to the inner section
*        of a given segment. n starts at 1.
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  wingIndex         The index of a wing, starting at 1
* @param[in]  segmentIndex      The index of a segment, starting at 1
* @param[in]  n                 n-th segment searched, 1 <= n <= tiglWingGetInnerConnectedSegmentCount(...)
* @param[out] connectedIndexPtr Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if wingIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occured
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int n,
                                                                        int* connectedIndexPtr);


/**
* @brief Returns the index (number) of the n-th wing segment connected to the outer section
*        of a given segment. n starts at 1.
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  wingIndex         The index of a wing, starting at 1
* @param[in]  segmentIndex      The index of a segment, starting at 1
* @param[in]  n                 n-th segment searched, 1 <= n <= tiglWingGetOuterConnectedSegmentCount(...)
* @param[out] connectedIndexPtr Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if wingIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        int wingIndex,
                                                                        int segmentIndex,
                                                                        int n,
                                                                        int* connectedIndexPtr);
        

/**
* @brief Returns the section index and section element index of the inner side of a given wing segment.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionIndexPtr The section index of the inner side
* @param[out] elementIndexPtr The section element index of the inner side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         int wingIndex,
                                                                         int segmentIndex,
                                                                         int* sectionIndexPtr,
                                                                         int* elementIndexPtr);
        
/**
* @brief Returns the section index and section element index of the outer side of a given wing segment.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionIndexPtr The section index of the outer side
* @param[out] elementIndexPtr The section element index of the outer side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         int wingIndex,
                                                                         int segmentIndex,
                                                                         int* sectionIndexPtr,
                                                                         int* elementIndexPtr);
        

/**
* @brief Returns the section UID and section element UID of the inner side of a given wing segment.
*
* <b>Important change:</b> The memory necessary for the two UIDs must not be freed
* by the user anymore.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionUIDPtr   The section UID of the inner side
* @param[out] elementUIDPtr   The section element UID of the inner side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                       int wingIndex,
                                                                       int segmentIndex,
                                                                       char** sectionUIDPtr,
                                                                       char** elementUIDPtr);
      
/**
* @brief Returns the section UID and section element UID of the outer side of a given wing segment.
*
* <b>Important change:</b> The memory necessary for the two UIDs must not be freed
* by the user anymore.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionUIDPtr   The section UID of the outer side
* @param[out] elementUIDPtr   The section element UID of the outer side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                       int wingIndex,
                                                                       int segmentIndex,
                                                                       char** sectionUIDPtr,
                                                                       char** elementUIDPtr);
      

/**
* @brief Returns the name of a wing profile. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  sectionIndex    The index of a section, starting at 1
* @param[in]  elementIndex    The index of an element on the section
* @param[out] profileNamePtr  The name of the wing profile
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* namePtr = 0;
   returnCode = tiglWingGetProfileName(cpacsHandle, wing, section, element, &namePtr);
   printf("Profile name is %s\n", namePtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
                                                         int wingIndex,
                                                         int sectionIndex,
                                                         int elementIndex,
                                                         char** profileNamePtr);
        


/**
* @brief Returns the UID of a wing. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[out] uidNamePtr      The uid of the wing
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglWingGetUID(cpacsHandle, wing, &uidPtr);
   printf("The UID of the wing is %s\n", uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 char** uidNamePtr);

/**
* @brief Returns the Index of a wing.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingUID         The uid of the wing
* @param[out] wingIndexPtr    The index of a wing, starting at 1
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int wingIndex;
   returnCode = tiglWingGetUID(cpacsHandle, wingUID, &wingIndex);
   printf("The Index of the wing is %d\n", wingIndex);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if wingUID does not exist
*   - TIGL_NULL_POINTER if wingUID is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                   const char* wingUID,
                                                   int* wingIndexPtr);

/**
* @brief Returns the UID of a segment of a wing. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] uidNamePtr      The uid of the wing
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglWingGetSegmentUID(cpacsHandle, wing, segmentID, &uidPtr);
   printf("The UID of the segment of wing %d is %s\n", wing, uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int segmentIndex,
                                                        char** uidNamePtr);



/**
* @brief Returns the Index of a segment of a wing. 
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  segmentUID      The uid of the wing
* @param[out] segmentIndexPtr  The index of a segment, starting at 1
* @param[out] wingIndexPtr     The index of a wing, starting at 1
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int segmentIndex, wingIndex;
   returnCode = tiglWingGetSegmentIndex(cpacsHandle, segmentUID, &segmentIndex, &wingIndex);
   printf("The Index of the segment of wing %d is %d\n", wingIndex, segmentIndex);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is not valid
*   - TIGL_UID_ERROR if the segmentUID does not exist
*   - TIGL_NULL_POINTER if segmentUID is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                          const char * segmentUID,
                                                          int * segmentIndexPtr,
                                                          int * wingIndexPtr);


/**
* @brief Returns the number of sections of a wing. 
**
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[out] sectionCount    The number of sections of the wing
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int sectionCount = 0;
   returnCode = tiglWingGetSectionUID(cpacsHandle, wingIndex, &sectionCount);
   printf("The Number of sections of wing %d is %d\n", wingIndex, sectionCount);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is not valid
*   - TIGL_NULL_POINTER if sectionCount is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int wingIndex,
                                                          int* sectionCount);

/**
* @brief Returns the UID of a section of a wing. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  sectionIndex    The index of a section, starting at 1
* @param[out] uidNamePtr      The uid of the wing
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglWingGetSectionUID(cpacsHandle, wing, sectionUID, &uidPtr);
   printf("The UID of the section of wing %d is %s\n", wing, uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int wingIndex,
                                                        int sectionIndex,
                                                        char** uidNamePtr);
        

/**
* @brief Returns the Symmetry Enum if the wing has symmetry-axis.
*
*
*
* @param[in]  cpacsHandle      Handle for the CPACS configuration
* @param[in]  wingIndex        Index of the Wing to export
* @param[out] symmetryAxisPtr  Returning TiglSymmetryAxis enum pointer
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, 
                                                      int wingIndex,
                                                      TiglSymmetryAxis* symmetryAxisPtr);



/**
* @brief Returns the segmentUID and wingUID for a given point on a componentSegment. The returned strings must not be freed by the user anymore.
*
*
* @param[in]  cpacsHandle             Handle for the CPACS configuration
* @param[in]  componentSegmentUID     UID of the componentSegment to search for
* @param[in]  x, y, z                 Coordinates of the point of the componentSegment
* @param[out] segmentUID              UID of the segment that fits to the given point and componentSegment.
*                                     In contrast to old releases, the returned string <b>must not be freed </b>by the user!
* @param[out] wingUID                 UID of the wing that fits to the given point and componentSegment
*                                     In contrast to old releases, the returned string <b>must not be freed </b>by the user!
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NOT_FOUND if the point does not lie on the wing component segment within 1cm tolerance.
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentFindSegment(TiglCPACSConfigurationHandle cpacsHandle,
                                                                      const char *componentSegmentUID, double x, double y,
                                                                      double z, char** segmentUID, char** wingUID);


/**
* @brief Returns x,y,z koordinates for a given eta and xsi on a componentSegment.
*
*
* @param[in]  cpacsHandle               Handle for the CPACS configuration
* @param[in]  componentSegmentUID       UID of the componentSegment to search for
* @param[in]  eta, xsi                  Eta and Xsi of the point of the componentSegment
* @param[out] x                         X coordinate of the point on the corresponding segment.
* @param[out] y                         Y coordinate of the point on the corresponding segment.
* @param[out] z                         Z coordinate of the point on the corresponding segment.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if the componentSegment does not exist
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   const char *componentSegmentUID, 
                                                                   double eta, double xsi,
                                                                   double * x, 
                                                                   double * y, 
                                                                   double * z);

/**
 * @brief Projects a points onto the chord face of the wing component segment
 *        and returns the eta/xsi coordinates of the point of projection.
 *
 * @param[in] cpacsHandle           Handle for the CPACS configuration
 * @param[in] componentSegmentUID   UID of the componentSegment to search for
 * @param[in] pX                    X coordinate of the point to project
 * @param[in] pY                    Y coordinate of the point to project
 * @param[in] pZ                    Z coordinate of the point to project
 * @param[out] eta                  Eta coordinate of the projected point
 * @param[out] xsi                  Xsi coordinate of the projected point
 * @param[out] errorDistance        Distance between the point and the projected point
 *                                  on the component segment. If the point already lies
 *                                  on the component segment's chord face, the errorDistance
 *                                  should be zero!
 * @return
 *   - TIGL_SUCCESS if no error occurred
 *   - TIGL_NOT_FOUND if no configuration was found for the given handle
 *   - TIGL_UID_ERROR if the componentSegment does not exist
 *   - TIGL_ERROR if some other error occurred
 */
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentPointGetEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         const char* componentSegmentUID,
                                                                         double pX, double pY, double pZ,
                                                                         double* eta, double* xsi,
                                                                         double* errorDistance);

/**
* @brief Returns eta, xsi, segmentUID and wingUID for a given eta and xsi on a componentSegment.
*
* If the given component segment point lies outside the wing chord surface, the function returns
* an error distance > 0. If this distance is larger than 1 mm, the point is first projected onto the segment (see image). Then, 
* this point is transformed into segment coordinates. It is up to the user to handle this case correctly.
*
* @image html compseg-getetaxsi.png "If the given point (black) lies outside the wing chord surface, it will be projected onto the wing (red)."
* @image latex compseg-getetaxsi.pdf "If the given point (black) lies outside the wing chord surface, it will be projected onto the wing (red)." width=10cm  
*
* @param[in]  cpacsHandle             Handle for the CPACS configuration
* @param[in]  componentSegmentUID     UID of the componentSegment to search for
* @param[in]  eta, xsi                Eta and Xsi of the point of the componentSegment
* @param[out] wingUID                 UID of the wing that fits to the given point and componentSegment.
*                                     In contrast to old releases, the returned string <b>must not be freed </b>by the user!
* @param[out] segmentUID              UID of the segment that fits to the given point and componentSegment.
*                                     In contrast to old releases, the returned string <b>must not be freed </b>by the user!
* @param[out] segmentEta              Eta of the point on the corresponding segment.
* @param[out] segmentXsi              Xsi of the point on the corresponding segment.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if the componentSegment does not exist
*   - TIGL_MATH_ERROR if the given wing component segment point can not be transformed into a segment point.
*                     This might happen, if the component segment contains twisted section.
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentPointGetSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                const char *componentSegmentUID, 
                                                                                double eta, double xsi,
                                                                                char** wingUID, 
                                                                                char** segmentUID,
                                                                                double *segmentEta, double *segmentXsi);



/**
* @brief Returns eta, xsi coordinates of a componentSegment given segmentEta and segmentXsi on a wing segment.
*
*
* @param[in]  cpacsHandle             Handle for the CPACS configuration
* @param[in]  segmentUID              UID of the wing segment to search for
* @param[in]  componentSegmentUID     UID of the associated componentSegment
* @param[in]  segmentEta, segmentXsi  Eta and Xsi coordinates of the point on the wing segment
* @param[out] eta                     Eta of the point on the corresponding component segment.
* @param[out] xsi                     Xsi of the point on the corresponding component segment.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if the segment or the component segment does not exist
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingSegmentPointGetComponentSegmentEtaXsi(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                const char *segmentUID, const char * componentSegmentUID, 
                                                                                double segmentEta, double segmentXsi,
                                                                                double *eta, double *xsi);


/**
* @brief Computes the intersection of a line (defined by component segment coordinates) with an iso-eta line on a
* specified wing segment.
*
* The component segment line is defined by its inner and outer point, both defined in
* component segment coordinates. Typically, these might be spar positions or leading
* edge coordinates of flaps. The segment line is defined by a iso-eta line. Typically,
* the intersection with a wing section would be computed (i.e. eta=1 or eta=0).
* The function returns the xsi coordinate (depth coordinate) of the intersection point.
* This coordinate is given in the segment coordinate system. See image below for details.
*
* @image html segment-compseg-intersect.png "Computation of the component segment - segment intersection point."
* @image latex segment-compseg-intersect.pdf "Computation of the component segment - segment intersection point." width=7cm 
*
* @param[in]  cpacsHandle             Handle for the CPACS configuration
* @param[in]  componentSegmentUID     UID of the componentSegment
* @param[in]  segmentUID              UID of the segment, the intersection should be calculated with
* @param[in]  csEta1, csEta2          Start and end eta coordinates of the intersection line (given as component segment coordinates)
* @param[in]  csXsi1, csXsi2          Start and end xsi coordinates of the intersection line (given as component segment coordinates)
* @param[in]  segmentEta              Eta coordinate of the iso-eta segment intersection line
* @param[out] segmentXsi              Xsi coordinate of the intersection point on the wing segment
* @param[out] hasWarning              The hasWarning flag is true (1), if the resulting xsi value is outside the valid
*                                     range [0,1]. It is up to the user to handle these cases properly. This flag is only
*                                     valid, if the function returns TIGL_SUCCESS.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if the segment or the component segment does not exist
*   - TIGL_MATH_ERROR if the intersection could not be computed (e.g. if no intersection exists)
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetSegmentIntersection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                 const char* componentSegmentUID,
                                                                                 const char* segmentUID,
                                                                                 double csEta1, double csXsi1,
                                                                                 double csEta2, double csXsi2,
                                                                                 double segmentEta, 
                                                                                 double* segmentXsi,
                                                                                 TiglBoolean* hasWarning);


/**
* @brief Given a straight line in space defined by a pair of component segment (eta,xsi) coordinates,
* the function computes the intersection of the line with a component segment iso-eta line.
*
* The function is similar to ::tiglWingComponentSegmentGetSegmentIntersection, with the difference, that
* an iso-line of the component segment is used instead of an iso-line of a segment.
* The line is defined by its inner and outer point, both given in
* component segment coordinates. Typically, these might be spar positions or leading
* edge coordinates of flaps. The eta value for the iso-eta line should be in the range [csEta1, csEta2].
* The function returns the xsi coordinate (depth coordinate) of the intersection point.
* This coordinate is given in the component segment coordinate system. See image below for details.
*
* @image html compseg-intersect.png "Computation of the component segment interpolation point."
* @image latex compseg-intersect.pdf "Computation of the component segment interpolation point." width=7.5cm
*
* @param[in]  cpacsHandle             Handle for the CPACS configuration
* @param[in]  componentSegmentUID     UID of the componentSegment
* @param[in]  csEta1, csEta2          Start and end eta coordinates of the intersection line (given as component segment coordinates)
* @param[in]  csXsi1, csXsi2          Start and end xsi coordinates of the intersection line (given as component segment coordinates)
* @param[in]  eta                     Eta coordinate of the iso-eta component segment intersection line
* @param[out] xsi                     Xsi coordinate of the intersection point on the wing component segment
* @param[out] hasWarning              The hasWarning flag is true (1), if the resulting xsi value is either outside the valid
*                                     range [0,1]. It is up to the user to handle these cases properly. This flag is only
*                                     valid, if the function returns TIGL_SUCCESS.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if the segment or the component segment does not exist
*   - TIGL_MATH_ERROR if the intersection could not be computed (e.g. if no intersection exists)
*   - TIGL_NULL_POINTER if componentSegmentUID or xsi are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentComputeEtaIntersection(TiglCPACSConfigurationHandle cpacsHandle,
                                                                                 const char* componentSegmentUID,
                                                                                 double csEta1, double csXsi1,
                                                                                 double csEta2, double csXsi2,
                                                                                 double eta,
                                                                                 double* xsi,
                                                                                 TiglBoolean* hasWarning);

/**
* @brief Returns the number of segments belonging to a component segment
*
*
* @param[in]  cpacsHandle             Handle for the CPACS configuration
* @param[in]  componentSegmentUID     UID of the componentSegment
* @param[out] nsegments               Number of segments belonging to the component segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if the component segment does not exist
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetNumberOfSegments(TiglCPACSConfigurationHandle cpacsHandle,
                                                                              const char * componentSegmentUID,
                                                                              int * nsegments);

/**
* @brief Returns the segment UID of a segment belonging to a component segment. The segment is specified
* with its index, which is in the 1...nsegments. The number of segments nsegments can be queried with
* ::tiglWingComponentSegmentGetNumberOfSegments.
*
*
* @param[in]  cpacsHandle             Handle for the CPACS configuration
* @param[in]  componentSegmentUID     UID of the componentSegment
* @param[in]  segmentIndex            Index of the segment (1 <= index <= nsegments)
* @param[out] segmentUID              UID of the segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if the component segment does not exist
*   - TIGL_INDEX_ERROR if the segment index is invalid
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                        const char * componentSegmentUID,
                                                                        int  segmentIndex,
                                                                        char ** segmentUID);

/*@}*/
/*****************************************************************************************************/

/**
  \defgroup FuselageFunctions Functions for fuselage calculations
    Function to handle fuselage geometry's with TIGL.
 */
/*@{*/

/**
* @brief Returns the number of fuselages in a CPACS configuration.
*
*
* @param[in]  cpacsHandle      Handle for the CPACS configuration
* @param[out] fuselageCountPtr Pointer to the number of fuselages
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if fuselageCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetFuselageCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int* fuselageCountPtr);

/**
* @brief Returns the number of segments for a fuselage in a CPACS configuration.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of the fuselage, starting at 1
* @param[out] segmentCountPtr Pointer to the number of segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int fuselageIndex,
                                                              int* segmentCountPtr);

/**
* @brief Returns the section center of a fuselage
* @param[in] cpacsHandle        Handle for the CPACS configuration
* @param[in] fuselageSegmentUID UID of the segment
* @param[in] eta                Parameter value from where on the given object the section is cut out, eta in the range 0.0 <= eta <= 1.0
* @param[out] pointX            Pointer to the x-coordinate of the section center point
* @param[out] pointY            Pointer to the y-coordinate of the section center point
* @param[out] pointZ            Pointer to the z-coordinate of the section center point
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if UID is invalid or not a fuselage segment
*   - TIGL_NULL_POINTER if pointX, pointY or pointZ are null pointers
*   - TIGL_MATH_ERROR if eta is out of range, i.e., not in [0, 1]
*   - TIGL_ERROR if some other error occurred
 */
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionCenter(TiglCPACSConfigurationHandle cpacsHandle,
                                                               const char *fuselageSegmentUID,
                                                               double eta,
                                                               double *pointX,
                                                               double *pointY,
                                                               double *pointZ);

/**
* @brief Returns the value of the area of a cross section of a fuselage
* @param[in] cpacsHandle        Handle for the CPACS configuration
* @param[in] fuselageSegmentUID UID of the segment
* @param[in] eta                Parameter value from where on the given object the section is cut out, eta in the range 0.0 <= eta <= 1.0
* @param[out] area              the area of the given cross section of a given fuselage
* @return
*   - TIGL_SUCCESS if the area was found
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if UID is invalid or not a fuselage segment
*   - TIGL_NULL_POINTER if area is a null pointer
*   - TIGL_MATH_ERROR if eta is out of range, i.e., not in [0, 1]
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetCrossSectionArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *fuselageSegmentUID,
                                                                  double eta,
                                                                  double *area);

/**
* @brief Returns the length of center line of the given fuselage with all its segments which connects all the center points of the cross sections of the fuselage
* @param[in] cpacsHandle Handle for the CPACS configuration
* @param[in] fuselageUID UID of the fuselage
* @param[out] length     the sum of the lengths of all the fuselage segments
* @return
*   - TIGL_SUCCESS if the length was found
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if UID is invalid or not a fuselage
*   - TIGL_NULL_POINTER if length is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetCenterLineLength(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char *fuselageUID,
                                                                  double *length);

/**
* @brief Returns a point on a fuselage surface for a given fuselage and segment index.
*
* Returns a point on a fuselage segment of a given fuselage in dependence of parameters eta and zeta with
* 0.0 <= eta <= 1.0 and 0.0 <= zeta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment. For zeta = 0.0 the point is the
* identical to the start point of the profile wire, for zeta = 1.0 it is identical to the last profile point.
* The point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex The index of the fuselage, starting at 1
* @param[in]  segmentIndex  The index of the segment of the fuselage, starting at 1
* @param[in]  eta           eta in the range 0.0 <= eta <= 1.0
* @param[in]  zeta          zeta in the range 0.0 <= zeta <= 1.0
* @param[out] pointXPtr     Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr     Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr     Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int fuselageIndex,
                                                       int segmentIndex,
                                                       double eta,
                                                       double zeta,
                                                       double* pointXPtr,
                                                       double* pointYPtr,
                                                       double* pointZPtr);


/**
* @brief Returns a point on a fuselage surface for a given fuselage and segment index and an angle alpha (degree).
*
* Returns a point on a fuselage segment of a given fuselage in dependence of parameters eta and at all y-positions with
* 0.0 <= eta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment.
* The angle alpha is calculated in degrees. Alpha=0 degree is meant to be "up" in the direction of the positive z-axis 
* like specifies in cpacs. It's orientation is the mathematical negative rotation direction around the X-axis, i.e. looking
* in flight direction, an angle of 45 degrees resembles a point on the top-left fuselage.
* The point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex The index of the fuselage, starting at 1
* @param[in]  segmentIndex  The index of the segment of the fuselage, starting at 1
* @param[in]  eta           Eta in the range 0.0 <= eta <= 1.0
* @param[in]  alpha         Angle alpha in degrees. No range restrictions.
* @param[out] pointXPtr     Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr     Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr     Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred, for example if there is no point at the given 
                 eta.
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int segmentIndex,
                                                            double eta,
                                                            double alpha,
                                                            double* pointXPtr,
                                                            double* pointYPtr,
                                                            double* pointZPtr);
        

/**
* @brief Returns a point on a fuselage surface for a given fuselage and segment index and an angle alpha (degree).
* 0 degree of the angle alpha is meant to be "up" in the direction of the positive z-axis like specifies in cpacs.
* The origin of the line that will be rotated with the angle alpha could be translated via the parameters y_cs and z_cs.
*
* Returns a point on a fuselage segment of a given fuselage in dependence of parameters eta and at all y-positions with
* 0.0 <= eta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment.
* The angle alpha is calculated in degrees. It's orientation is the mathematical negative rotation direction around the X-axis, i.e. looking
* in flight direction, an angle of 45 degrees resembles a point on the top-left fuselage.
* The parameters y_cs and z_cs must be in absolute world coordinates.
* The point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex The index of the fuselage, starting at 1
* @param[in]  segmentIndex  The index of the segment of the fuselage, starting at 1
* @param[in]  eta           eta in the range 0.0 <= eta <= 1.0
* @param[in]  alpha         Angle alpha in degrees. No range restrictions.
* @param[in]  y_cs          Shifts the origin of the angle alpha in y-direction.
* @param[in]  z_cs          Shifts the origin of the angle alpha in z-direction.
* @param[out] pointXPtr     Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr     Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr     Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred, for example if there is no point at the given 
                 eta and the given shifting parameters y_cs and z_cs.
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointAngleTranslated(TiglCPACSConfigurationHandle cpacsHandle,
                                                                      int fuselageIndex,
                                                                      int segmentIndex,
                                                                      double eta,
                                                                      double alpha,
                                                                      double y_cs,
                                                                      double z_cs,
                                                                      double* pointXPtr,
                                                                      double* pointYPtr,
                                                                      double* pointZPtr);
      



/**
* @brief Returns a point on a fuselage surface for a given fuselage and segment index.
*
* Returns a point on a fuselage segment of a given fuselage in dependence of parameters eta and at all y-positions with
* 0.0 <= eta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment.
* The point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex The index of the fuselage, starting at 1
* @param[in]  segmentIndex  The index of the segment of the fuselage, starting at 1
* @param[in]  eta           eta in the range 0.0 <= eta <= 1.0
* @param[in]  xpos          x position of a cutting plane
* @param[in]  pointIndex    Defines witch point if more than one.
* @param[out] pointXPtr     Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr     Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr     Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int fuselageIndex,
                                                               int segmentIndex,
                                                               double eta,
                                                               double xpos,
                                                               int pointIndex,
                                                               double* pointXPtr,
                                                               double* pointYPtr,
                                                               double* pointZPtr);
        
        
/**
* @brief Returns a point on a fuselage surface for a given fuselage and segment index.
*
* Returns a point on a fuselage segment of a given fuselage in dependence of parameters eta and at all y-positions with
* 0.0 <= eta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment.
* The point is returned in absolute world coordinates.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex The index of the fuselage, starting at 1
* @param[in]  segmentIndex  The index of the segment of the fuselage, starting at 1
* @param[in]  eta           eta in the range 0.0 <= eta <= 1.0
* @param[in]  ypos          Y position
* @param[in]  pointIndex    Defines witch point if more than one.
* @param[out] pointXPtr     Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr     Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr     Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetPointOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int fuselageIndex,
                                                               int segmentIndex,
                                                               double eta,
                                                               double ypos,
                                                               int pointIndex,
                                                               double* pointXPtr,
                                                               double* pointYPtr,
                                                               double* pointZPtr);
        

/**
* @brief Returns the number of points on a fuselage surface for a given fuselage and a give x-position.
*
* Returns the number of points on a fuselage segment of a given fuselage in dependence of parameters eta and at all x-positions with
* 0.0 <= eta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex The index of the fuselage, starting at 1
* @param[in]  segmentIndex  The index of the segment of the fuselage, starting at 1
* @param[in]  eta           eta in the range 0.0 <= eta <= 1.0
* @param[in]  xpos          X position
* @param[out] numPointsPtr  Pointer to a integer for the number of intersection points
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int fuselageIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double xpos,
                                                                   int* numPointsPtr);
        


/**
* @brief Returns the number of points on a fuselage surface for a given fuselage and a give y-position.
*
* Returns the number of points on a fuselage segment of a given fuselage in dependence of parameters eta and at all y-positions with
* 0.0 <= eta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex The index of the fuselage, starting at 1
* @param[in]  segmentIndex  The index of the segment of the fuselage, starting at 1
* @param[in]  eta           eta in the range 0.0 <= eta <= 1.0
* @param[in]  ypos          Y position
* @param[out] numPointsPtr  Pointer to a interger for the number of intersection points
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int fuselageIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double ypos,
                                                                   int* numPointsPtr);
        

/**
* @brief Returns the circumference of a fuselage surface for a given fuselage and segment index and an eta.
*
* Returns the circumference of a fuselage segment of a given fuselage in dependence of parameters eta with
* 0.0 <= eta <= 1.0. For eta = 0.0 the point lies on the start profile of the
* segment, for eta = 1.0 it lies on the end profile of the segment.
*
*
* @param[in]  cpacsHandle      Handle for the CPACS configuration
* @param[in]  fuselageIndex    The index of the fuselage, starting at 1
* @param[in]  segmentIndex     The index of the segment of the fuselage, starting at 1
* @param[in]  eta              eta in the range 0.0 <= eta <= 1.0
* @param[out] circumferencePtr The Circumference of the fuselage at the given position
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetCircumference(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int fuselageIndex,
                                                               int segmentIndex,
                                                               double eta,
                                                               double* circumferencePtr);
       

/**
* @brief Returns the count of segments connected to the start section of a given fuselage segment.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] segmentCountPtr Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int fuselageIndex,
                                                                            int segmentIndex,
                                                                            int* segmentCountPtr);
        
/**
* @brief Returns the count of segments connected to the end section of a given fuselage segment.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] segmentCountPtr Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                          int fuselageIndex,
                                                                          int segmentIndex,
                                                                          int* segmentCountPtr);
        
/**
* @brief Returns the index (number) of the n-th segment connected to the start section
*        of a given fuselage segment. n starts at 1.
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  fuselageIndex     The index of a fuselage, starting at 1
* @param[in]  segmentIndex      The index of a segment, starting at 1
* @param[in]  n                 n-th segment searched, 1 <= n <= tiglFuselageGetStartConnectedSegmentCount(...)
* @param[out] connectedIndexPtr Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if fuselageIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int fuselageIndex,
                                                                            int segmentIndex,
                                                                            int n,
                                                                            int* connectedIndexPtr);

/**
* @brief Returns the index (number) of the n-th segment connected to the end section
*        of a given fuselage segment. n starts at 1.
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  fuselageIndex     The index of a fuselage, starting at 1
* @param[in]  segmentIndex      The index of a segment, starting at 1
* @param[in]  n                 n-th segment searched, 1 <= n <= tiglFuselageGetEndConnectedSegmentCount(...)
* @param[out] connectedIndexPtr Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if fuselageIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                          int fuselageIndex,
                                                                          int segmentIndex,
                                                                          int n,
                                                                          int* connectedIndexPtr);


/**
* @brief Returns the section UID and section element UID of the start side of a given fuselage segment.
*
* <b>Important change:</b> The memory necessary for the two UIDs must not to be freed
* by the user anymore.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionUIDPtr   The section UID of the start side
* @param[out] elementUIDPtr   The section element UID of the start side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                           int fuselageIndex,
                                                                           int segmentIndex,
                                                                           char** sectionUIDPtr,
                                                                           char** elementUIDPtr);
      
    

/**
* @brief Returns the section UID and section element UID of the end side of a given fuselage segment.
*
* <b>Important change:</b> The memory necessary for the two UIDs must not to be freed
* by the user anymore.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionUIDPtr   The section UID the end side
* @param[out] elementUIDPtr   The section element UID of the end side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         int fuselageIndex,
                                                                         int segmentIndex,
                                                                         char** sectionUIDPtr,
                                                                         char** elementUIDPtr);
      




/**
* @brief Returns the section index and section element index of the start side of a given fuselage segment.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionIndexPtr The section UID of the start side
* @param[out] elementIndexPtr The section element UID of the start side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                             int fuselageIndex,
                                                                             int segmentIndex,
                                                                             int* sectionIndexPtr,
                                                                             int* elementIndexPtr);
        
/**
* @brief Returns the section index and section element index of the end side of a given fuselage segment.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] sectionIndexPtr The section index UID the end side
* @param[out] elementIndexPtr The section element UID of the end side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                           int fuselageIndex,
                                                                           int segmentIndex,
                                                                           int* sectionIndexPtr,
                                                                           int* elementIndexPtr);
        


/**
* @brief Returns the name of a fuselage profile. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  sectionIndex    The index of a section, starting at 1
* @param[in]  elementIndex    The index of an element on the section
* @param[out] profileNamePtr  The name of the wing profile
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* namePtr = 0;
   returnCode = tiglFuselageGetProfileName(cpacsHandle, fuselage, section, element, &namePtr);
   printf("Profile name is %s\n", namePtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
                                                             int fuselageIndex,
                                                             int sectionIndex,
                                                             int elementIndex,
                                                             char** profileNamePtr);



/**
* @brief Returns the UID of a fuselage. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[out] uidNamePtr      The uid of the fuselage
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglFuselageGetUID(cpacsHandle, fuselage, &uidPtr);
   printf("The UID of the fuselage is %s\n", uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int fuselageIndex,
                                                     char** uidNamePtr);


/**
* @brief Returns the index of a fuselage.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageUID         The uid of the fuselage
* @param[out] fuselageIndexPtr    The index of a fuselage, starting at 1
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int fuselageIndex;
   returnCode = tiglFuselageGetIndex(cpacsHandle, fuselageUID, &fuselageIndex);
   printf("The Index of the fuselage is %d\n", fuselageIndex);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if fuselageUID does not exist
*   - TIGL_NULL_POINTER if fuselageUID is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                       const char* fuselageUID,
                                                       int* fuselageIndexPtr);


/**
* @brief Returns the UID of a segment of a fuselage. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] uidNamePtr      The uid of the fuselage
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglFuselageGetSegmentUID(cpacsHandle, fuselage, segmentID, &uidPtr);
   printf("The UID of the segment of fuselage %d is %s\n", fuselage, uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int segmentIndex,
                                                            char** uidNamePtr);


/**
* @brief Returns the Index of a segment of a fuselage.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  segmentUID      The uid of the fuselage
* @param[out] segmentIndexPtr  The index of a segment, starting at 1
* @param[out] fuselageIndexPtr     The index of a fuselage, starting at 1
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int segmentIndex, fuselageIndex;
   returnCode = tiglFuselageGetSegmentIndex(cpacsHandle, segmentUID, &segmentIndex, &fuselageIndex);
   printf("The Index of the segment of fuselage %d is %d\n", fuselageIndex, segmentIndex);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is not valid
*   - TIGL_UID_ERROR if the segmentUID does not exist
*   - TIGL_NULL_POINTER if segmentUID is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                          const char * segmentUID,
                                                          int * segmentIndexPtr,
                                                          int * fuselageIndexPtr);


/**
* @brief Returns the number of sections of a fuselage. 
**
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[out] sectionCount    The number of sections of the fuselage
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int sectionCount = 0;
   returnCode = tiglFuselageGetSectionUID(cpacsHandle, fuselageIndex, &sectionCount);
   printf("The Number of sections of fuselage %d is %d\n", fuselageIndex, sectionCount);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is not valid
*   - TIGL_NULL_POINTER if sectionCount is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int fuselageIndex,
                                                              int* sectionCount);


/**
* @brief Returns the UID of a section of a fuselage. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  sectionIndex    The index of a section, starting at 1
* @param[out] uidNamePtr      The uid of the fuselage
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglFuselageGetSectionUID(cpacsHandle, fuselage, sectionUID, &uidPtr);
   printf("The UID of the section of fuselage %d is %s\n", fuselage, uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int fuselageIndex,
                                                            int sectionIndex,
                                                            char** uidNamePtr);



/**
* @brief Returns the Symmetry Enum if the fuselage has symmetry-axis.
*
*
* @param[in]  cpacsHandle      Handle for the CPACS configuration
* @param[in]  fuselageIndex    Index of the fuselage in the cpacs file
* @param[out] symmetryAxisPtr  Returning TiglSymmetryAxis enum pointer
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int fuselageIndex,
                                                          TiglSymmetryAxis* symmetryAxisPtr);



/**
* @brief Returns the point where the distance between the selected fuselage and the ground is at minimum.
*         The Fuselage could be turned with a given angle at at given axis, specified by a point and a direction.
*
*
* @param[in]  cpacsHandle         Handle for the CPACS configuration
* @param[in]  fuselageUID         The uid of the fuselage
* @param[in]  axisPntX            X-coordinate of the point that specifies the axis of rotation
* @param[in]  axisPntY            Y-coordinate of the point that specifies the axis of rotation
* @param[in]  axisPntZ            Z-coordinate of the point that specifies the axis of rotation
* @param[in]  axisDirX            X-coordinate of the direction that specifies the axis of rotation
* @param[in]  axisDirY            Y-coordinate of the direction that specifies the axis of rotation
* @param[in]  axisDirZ            Z-coordinate of the direction that specifies the axis of rotation
* @param[in]  angle               The angle (in Degree) by which the fuselage should be turned on the axis of rotation
* @param[out] pointXPtr           Pointer to the x-coordinate of the point in absolute world coordinates
* @param[out] pointYPtr           Pointer to the y-coordinate of the point in absolute world coordinates
* @param[out] pointZPtr           Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageUID is not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetMinumumDistanceToGround(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         char *fuselageUID,
                                                                         double axisPntX,
                                                                         double axisPntY,
                                                                         double axisPntZ,
                                                                         double axisDirX,
                                                                         double axisDirY,
                                                                         double axisDirZ,
                                                                         double angle,
                                                                         double* pointXPtr,
                                                                         double* pointYPtr,
                                                                         double* pointZPtr);



/*****************************************************************************************************/

/*@}*/


/**
  \defgroup RotorFunctions Functions for rotor calculations
    Functions to handle rotor geometries with TIGL.
 */
/*@{*/

/**
* @brief Returns the number of rotors in a CPACS configuration.
*
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_count(integer cpacsHandle,
*                      integer rotorCountPtr,
*                      integer returnCode)
*
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[out] rotorCountPtr Pointer to the number of rotors
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if rotorCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetRotorCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                    int* rotorCountPtr);

/**
* @brief Returns the UID of a rotor. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_uid(integer cpacsHandle,
*                    integer rotorIndex,
*                    character*n uIDNamePtr,
*                    integer returnCode)
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  rotorIndex      The index of a rotor, starting at 1
* @param[out] uidNamePtr      The uid of the rotor
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   char* uidPtr = 0;
   returnCode = tiglRotorGetUID(cpacsHandle, rotor, &uidPtr);
   printf("The UID of the rotor is %s\n", uidPtr);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is not valid
*   - TIGL_NULL_POINTER if profileNamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                  int rotorIndex,
                                                  char** uidNamePtr);

/**
* @brief Returns the Index of a rotor.
*
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_index(integer cpacsHandle,
*                      character*n uIDNamePtr,
*                      integer rotorIndex,
*                      integer returnCode)
*
*
* @param[in]  cpacsHandle      Handle for the CPACS configuration
* @param[in]  rotorUID         The uid of the rotor
* @param[out] rotorIndexPtr    The index of a rotor, starting at 1
*
* Usage example:
*
@verbatim
   TiglReturnCode returnCode;
   int rotorIndex;
   returnCode = tiglRotorGetUID(cpacsHandle, rotorUID, &rotorIndex);
   printf("The Index of the rotor is %d\n", rotorIndex);
@endverbatim
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if rotorUID does not exist
*   - TIGL_NULL_POINTER if rotorUID is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                    const char* rotorUID,
                                                    int* rotorIndexPtr);

/**
* @brief Returns the radius of the rotor.
*
* This function returns the radius of the largest blade attached to the rotor hub.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_radius(integer cpacsHandle, int rotorIndex, real radiusPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor to calculate the radius, starting at 1
*
* @param[out] radiusPtr         The radius of the rotor
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is less or equal zero or greater than the rotor count
*   - TIGL_NULL_POINTER if radiusPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int rotorIndex,
                                                     double *radiusPtr);

/**
* @brief Returns the reference area of the rotor.
*
* The area of the rotor disk is taken as reference area of the rotor. It is calculated using the formula pi*r^2,
* where r denotes the radius of the largest attached blade.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_reference_area(integer cpacsHandle, int rotorIndex, real referenceAreaPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor to calculate the area, starting at 1
*
* @param[out] referenceAreaPtr  The reference area of the rotor
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is less or equal zero or greater than the rotor count
*   - TIGL_NULL_POINTER if referenceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetReferenceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int rotorIndex,
                                                            double *referenceAreaPtr);

/**
* @brief Returns the total blade planform area of the rotor.
*
* This function calculates the sum of the planform areas of all blades attached to the rotor hub.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_total_blade_planform_area(integer cpacsHandle, int rotorIndex, real totalBladePlanformAreaPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle               Handle for the CPACS configuration
* @param[in]  rotorIndex                Index of the rotor to calculate the area, starting at 1
*
* @param[out] totalBladePlanformAreaPtr The total blade planform area of the rotor
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is less or equal zero or greater than the rotor count
*   - TIGL_NULL_POINTER if totalBladePlanformAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetTotalBladePlanformArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                     int rotorIndex,
                                                                     double *totalBladePlanformAreaPtr);

/**
* @brief Returns the solidity of the rotor.
*
* The rotor solidity ratio is calculated by dividing the total blade planform area by the rotor disk area.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_solidity(integer cpacsHandle, int rotorIndex, real solidityPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor to calculate the area, starting at 1
*
* @param[out] solidityPtr       The reference area of the rotor
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is less or equal zero or greater than the rotor count
*   - TIGL_NULL_POINTER if solidityPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetSolidity(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int rotorIndex,
                                                       double *solidityPtr);

/**
* @brief Returns the surface area of the rotor.
*
* The returned surface area is the sum of the surface areas of all attached rotor blades.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_surface_area(integer cpacsHandle, int rotorIndex, real surfaceAreaPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  rotorIndex      Index of the Rotor to calculate the area, starting at 1
* @param[out] surfaceAreaPtr  The surface area of the rotor
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is less or equal zero or greater than the rotor count
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int rotorIndex,
                                                          double *surfaceAreaPtr);

/**
* @brief Returns the volume of the rotor.
*
* The returned volume is the sum of the volumes of all attached rotor blades.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_volume(integer cpacsHandle, int rotorIndex, real volumePtr, integer returnCode)
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  rotorIndex      Index of the rotor to calculate the volume, starting at 1
* @param[out] volumePtr       The volume of the rotor
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is less or equal zero or greater than the rotor count
*   - TIGL_NULL_POINTER if volumePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                     int rotorIndex,
                                                     double *volumePtr);

/**
* @brief Returns the tip speed of the rotor in [m/s].
*
* The rotor tip speed is calculated using the nominal rotation speed of the rotor and the rotor radius.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_get_tip_speed(integer cpacsHandle, int rotorIndex, real tipSpeedPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor to calculate the area, starting at 1
*
* @param[out] tipSpeedPtr       The tip speed of the rotor
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex is less or equal zero or greater than the rotor count
*   - TIGL_NULL_POINTER if tipSpeedPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetTipSpeed(TiglCPACSConfigurationHandle cpacsHandle,
                                                       int rotorIndex,
                                                       double *tipSpeedPtr);

/*@}*/  // end of doxygen group RotorFunctions
/*****************************************************************************************************/


/**
  \defgroup RotorBladeFunctions Functions for rotor blade calculations
    Functions to handle rotor blade geometries with TIGL.
 */
/*@{*/

/**
* @brief Returns the total number of rotor blades attached to a rotor.
*
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_count(integer cpacsHandle, integer rotorIndex, integer rotorBladeCountPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle  Handle for the CPACS configuration
* @param[in]  rotorIndex   Index of the rotor to count the attached rotor blades
* @param[out] rotorBladeCountPtr Pointer to the number of rotor blades
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if rotorBladeCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorGetRotorBladeCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int rotorIndex,
                                                              int* rotorBladeCountPtr);

/**
* @brief Returns the index of the parent wing definition of a rotor blade.
*
* Returns the index of the wing definition referenced by the parent rotor blade attachment of the rotor blade with the index rotorBladeIndex.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_wing_index(integer cpacsHandle, integer rotorIndex, integer rotorBladeIndex, integer wingIndexPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[out] wingIndexPtr      Pointer to the wing index
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if wingIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetWingIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                             int rotorIndex,
                                                             int rotorBladeIndex,
                                                             int* wingIndexPtr);

/**
* @brief Returns the UID of the parent wing definition of a rotor blade.
*
* Returns the UID of the wing definition referenced by the parent rotor blade attachment of the rotor blade with the index rotorBladeIndex.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_wing_uid(integer cpacsHandle, integer rotorIndex, integer rotorBladeIndex, character*n wingUIDPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[out] wingUIDPtr      Pointer to the wing index
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if wingUIDPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetWingUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int rotorIndex,
                                                           int rotorBladeIndex,
                                                           char** wingUIDPtr);

/**
* @brief Returns the azimuth angle of a rotor blade in degrees.
*
* Returns the azimuth angle in degrees of the rotor blade with the index rotorBladeIndex attached to the rotor with the index rotorIndex.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_azimuth(integer cpacsHandle, integer rotorIndex, integer rotorBladeIndex, real azimuthAngle, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[out] azimuthAnglePtr   Pointer to the azimuth angle
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if azimuthAnglePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetAzimuthAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int rotorIndex,
                                                                int rotorBladeIndex,
                                                                double* azimuthAnglePtr);

/**
* @brief Returns the radius of a rotor blade.
*
* Returns the radius of the rotor blade with the index rotorBladeIndex attached to the rotor with the index rotorIndex.
* The maximum distance of a point on the quarter chord line of the rotor blade from the z axis of the rotor coordinate system is taken as the rotor blade radius.
* It is calculated for the rotor blade at azimuth=0 and with no hinge transformations applied.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_radius(integer cpacsHandle, integer rotorIndex, integer rotorBladeIndex, real radiusPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[out] radiusPtr         Pointer to the radius
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if radiusPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                          int rotorIndex,
                                                          int rotorBladeIndex,
                                                          double* radiusPtr);

/**
* @brief Returns the planform area of the rotor blade.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_blade_get_planform_area(integer cpacsHandle, int rotorIndex, int rotorBladeIndex, real planformAreaPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade to calculate the area, starting at 1
* @param[out] planformAreaPtr   The planform area of the rotor blade
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if planformAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetPlanformArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                                int rotorIndex,
                                                                int rotorBladeIndex,
                                                                double *planformAreaPtr);

/**
* @brief Returns the surface area of the rotor blade.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_blade_get_surface_area(integer cpacsHandle, int rotorIndex, int rotorBladeIndex, real surfaceAreaPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade to calculate the area, starting at 1
* @param[out] surfaceAreaPtr    The surface area of the rotor blade
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                               int rotorIndex,
                                                               int rotorBladeIndex,
                                                               double *surfaceAreaPtr);

/**
* @brief Returns the volume of the rotor blade.
*
* <b>Fortran syntax:</b>
*
* tigl_rotor_blade_get_volume(integer cpacsHandle, int rotorIndex, int rotorBladeIndex, real volumePtr, integer returnCode)
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  rotorIndex      Index of the rotor
* @param[in]  rotorBladeIndex Index of the rotor blade to calculate the volume, starting at 1
* @param[out] volumePtr       The volume of the rotor blade
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if volumePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int rotorIndex,
                                                          int rotorBladeIndex,
                                                          double *volumePtr);

/**
* @brief Returns the tip speed of a rotor blade [m/s].
*
* The rotor blade tip speed is calculated using the nominal rotation speed of the rotor and the rotor blade radius.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_tip_speed(integer cpacsHandle, integer rotorIndex, integer rotorBladeIndex, real tipSpeedPtr, integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[out] tipSpeedPtr       Pointer to the radius
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if rotorIndex or rotorBladeIndex are invalid
*   - TIGL_NULL_POINTER if tipSpeedPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetTipSpeed(TiglCPACSConfigurationHandle cpacsHandle,
                                                            int rotorIndex,
                                                            int rotorBladeIndex,
                                                            double* tipSpeedPtr);

/**
* @brief Returns the local radius of a rotor blade.
*
* Returns the local radius of the rotor blade with the index rotorBladeIndex attached to the rotor with the index rotorIndex.
* The distance of the point with relative spanwise coordinate eta on the quarter chord line of the rotor blade segment segmentIndex 
* from the z axis of the rotor coordinate system is taken as the local radius.
* It is calculated for the rotor blade at azimuth=0 and with no hinge transformations applied.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_local_radius(integer cpacsHandle,
*                                   integer rotorIndex,
*                                   integer rotorBladeIndex,
*                                   integer segmentIndex,
*                                   real eta,
*                                   real radiusPtr,
*                                   integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[in]  segmentIndex      Index of the segment of the referenced wing definition
* @param[in]  eta               Relative spanwise segment coordinate: eta in the range 0.0 <= eta <= 1.0; eta = 0 for inner section , eta = 1 for outer section
* @param[out] radiusPtr         Pointer to the local radius
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no point was found
*   - TIGL_INDEX_ERROR if rotorIndex, rotorBladeIndex or segmentIndex are invalid
*   - TIGL_NULL_POINTER if radiusPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetLocalRadius(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int rotorIndex,
                                                               int rotorBladeIndex,
                                                               int segmentIndex,
                                                               double eta,
                                                               double* radiusPtr);

/**
* @brief Returns the local chord length of a rotor blade.
*
* Returns the local chord length of the section at the relative spanwise coordinate eta of the segment segmentIndex of the rotor blade with the 
* index rotorBladeIndex attached to the rotor with the index rotorIndex.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_local_chord(integer cpacsHandle,
*                                  integer rotorIndex,
*                                  integer rotorBladeIndex,
*                                  integer segmentIndex,
*                                  real eta,
*                                  real chordPtr,
*                                  integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[in]  segmentIndex      Index of the segment of the referenced wing definition
* @param[in]  eta               Relative spanwise segment coordinate: eta in the range 0.0 <= eta <= 1.0; eta = 0 for inner section , eta = 1 for outer section
* @param[out] chordPtr          Pointer to the local chord length
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no point was found
*   - TIGL_INDEX_ERROR if rotorIndex, rotorBladeIndex or segmentIndex are invalid
*   - TIGL_NULL_POINTER if chordPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetLocalChord(TiglCPACSConfigurationHandle cpacsHandle,
                                                              int rotorIndex,
                                                              int rotorBladeIndex,
                                                              int segmentIndex,
                                                              double eta,
                                                              double* chordPtr);

/**
* @brief Returns the local twist angle [deg] of a rotor blade.
*
* Returns the twist angle in degrees of the section at the relative spanwise coordinate eta of the segment segmentIndex of the rotor blade with the 
* index rotorBladeIndex attached to the rotor with the index rotorIndex.
*
* <b>Fortran syntax:</b>
*
* tigl_get_rotor_blade_local_twist_angle(integer cpacsHandle,
*                                        integer rotorIndex,
*                                        integer rotorBladeIndex,
*                                        integer segmentIndex,
*                                        real eta,
*                                        real twistAnglePtr,
*                                        integer returnCode)
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  rotorIndex        Index of the rotor
* @param[in]  rotorBladeIndex   Index of the rotor blade
* @param[in]  segmentIndex      Index of the segment of the referenced wing definition
* @param[in]  eta               Relative spanwise segment coordinate: eta in the range 0.0 <= eta <= 1.0; eta = 0 for inner section , eta = 1 for outer section
* @param[out] twistAnglePtr     Pointer to the local twist angle
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no point was found
*   - TIGL_INDEX_ERROR if rotorIndex, rotorBladeIndex or segmentIndex are invalid
*   - TIGL_NULL_POINTER if twistAnglePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglRotorBladeGetLocalTwistAngle(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int rotorIndex,
                                                                   int rotorBladeIndex,
                                                                   int segmentIndex,
                                                                   double eta,
                                                                   double* twistAnglePtr);

/*@}*/  // end of doxygen group RotorBladeFunctions
/*****************************************************************************************************/


/**
  \defgroup BooleanFunctions Functions for boolean calculations
    Function for boolean calculations on wings/fuselages.
    
    These function currently only implement intersection algorithms between two shapes
    defined in cpacs ot a shape and a plane. Shapes or geometries are identified with 
    their cpacs uid.
    
    Currently only wings, wing segments, fuselages, and fuselage segments can be used
    in the intersection routines.
 */
/*@{*/

/**
* @brief tiglIntersectComponents computes the intersection line(s) between two shapes
* specified by their CPACS uid. It returns an intersection ID for further computations
* on the result. To query points on the intersection line, ::tiglIntersectGetPoint has
* to be called.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  componentUidOne The UID of the first component
* @param[in]  componentUidTwo The UID of the second component
* @param[out] intersectionID  A unique identifier that is associated with the computed intersection.
*
*
* @return
*   - TIGL_SUCCESS if an intersection could be computed
*   - TIGL_NOT_FOUND if the cpacs handle is not valid
*   - TIGL_NULL_POINTER if either componentUidOne, componentUidTwo, or intersectionID are NULL pointers
*   - TIGL_UID_ERROR if componentUidOne or componentUidTwo can not be found in the CPACS file
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectComponents(TiglCPACSConfigurationHandle cpacsHandle,
                                                          const char*  componentUidOne,
                                                          const char*  componentUidTwo,
                                                          char** intersectionID);

/**
* @brief tiglIntersectWithPlane computes the intersection line(s) between a shape and a plane. 
* It returns an intersection ID for further computations on the result. 
* To query points on the intersection line, ::tiglIntersectGetPoint has
* to be called.
*
* The shape has to be specified by its CPACS UID.
* The plane is specified by a central point p on the plane and a normal vector n, which is
* perpendicular to the plane. The normal vector must not be zero!
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  componentUid    The UID of the CPACS shape
* @param[in]  px              X Coordinate of the plane center point
* @param[in]  py              Y Coordinate of the plane center point
* @param[in]  pz              Z Coordinate of the plane center point
* @param[in]  nx              X value of the plane normal vector
* @param[in]  ny              Y value of the plane normal vector
* @param[in]  nz              Z value of the plane normal vector
* @param[out] intersectionID  A unique identifier that is associated with the computed intersection.
*
*
* @return
*   - TIGL_SUCCESS if an intersection could be computed
*   - TIGL_NOT_FOUND if the cpacs handle is not valid
*   - TIGL_NULL_POINTER if either componentUid or intersectionID are NULL pointers
*   - TIGL_UID_ERROR if componentUid can not be found in the CPACS file
*   - TIGL_MATH_ERROR if the normal vector is zero
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectWithPlane(TiglCPACSConfigurationHandle cpacsHandle,
                                                         const char*  componentUid,
                                                         double px, double py, double pz,
                                                         double nx, double ny, double nz,
                                                         char** intersectionID);

/**
* @brief tiglIntersectWithPlaneSegment computes the intersection line(s) between a shape
* and a plane segment. The plane segment is defined by p(u,v) = P1*(1-u) + P2*u + w*v, with
* u in [0,1] (see image).
*
* @image html intersectPlaneSegment.png "The dashed line connecting the points P1 and P2 is projected along w onto the shape (blue dashed line)."
* @image latex intersectPlaneSegment.pdf "The dashed line connecting the points P1 and P2 is projected along w onto the shape (blue dashed line)." width=10cm
*
* It returns an intersection ID for further computations on the result.
* To query points on the intersection line, ::tiglIntersectGetPoint has
* to be called.
*
* The shape has to be specified by its CPACS UID.
* The plane segment is specified by two points p1 and p2  and a normal vector w, which
* (ideally) is perpendicular to (p2 - p1). The normal vector w must not be zero!
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  componentUid    The UID of the CPACS shape
* @param[in]  p1x             X Coordinate of the first point P1
* @param[in]  p1y             Y Coordinate of the first point P1
* @param[in]  p1z             Z Coordinate of the first point P1
* @param[in]  p2x             X Coordinate of the second point P2
* @param[in]  p2y             Y Coordinate of the second point P2
* @param[in]  p2z             Z Coordinate of the second point P2
* @param[in]  wx              X value of the normal vector w
* @param[in]  wy              Y value of the normal vector w
* @param[in]  wz              Z value of the normal vector w
* @param[out] intersectionID  A unique identifier that is associated with the computed intersection.
*
*
* @return
*   - TIGL_SUCCESS if an intersection could be computed
*   - TIGL_NOT_FOUND if the cpacs handle is not valid
*   - TIGL_NULL_POINTER if either componentUid or intersectionID are NULL pointers
*   - TIGL_UID_ERROR if componentUid can not be found in the CPACS file
*   - TIGL_MATH_ERROR if the normal vector is zero, p1 and p2 are equal or if w and (p2-p1) are linearly dependent.
*
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectWithPlaneSegment(TiglCPACSConfigurationHandle cpacsHandle,
                                                                const char*  componentUid,
                                                                double p1x, double p1y, double p1z,
                                                                double p2x, double p2y, double p2z,
                                                                double wx, double wy, double wz,
                                                                char** intersectionID);

/**
* @brief tiglGetCurveIntersection computes intersection point of two (intersection) curves.
* Both curves are the result of a previous intersection (e.g. Fuselage-Wing Intersection).
*
* The curves must be specified by their intersectionIDs and an index of the wire in the
* intersection. For each calculated intersection, the distance between the point on the first curve
* to the point on the second curve is smaller than the tolerance specified by the user.
*
*
* It returns an intersection ID for further computations on the result.
* To query the number of intersection points, call ::tiglGetCurveIntersectionCount.
* To query intersection points, ::tiglGetCurveIntersectionPoint or ::tiglGetCurveParameter
* has to be called.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  curvesID1       ID of the first intersection
* @param[in]  curve1Idx       Index of the curve in the first intersection
* @param[in]  curvesID2       ID of the second intersection
* @param[in]  curve2Idx       Index of the curve in the second intersection
* @param[in]  tolerance       tolerance to specify the required accuracy of the intersection point
* @param[out] intersectionID  A unique identifier that is associated with the computed intersection.
*
*
* @return
*   - TIGL_SUCCESS if an intersection could be computed
*   - TIGL_NOT_FOUND if the cpacs handle is not valid
*   - TIGL_NULL_POINTER if either curveID or the intersectionID is a NULL pointer
*   - TIGL_INDEX_ERROR if either curve index is invalid
*   - TIGL_MATH_ERROR if the tolerance is not positive
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveIntersection(TiglCPACSConfigurationHandle cpacsHandle,
                                                           const char* curvesID1, int curve1Idx,
                                                           const char* curvesID2, int curve2Idx,
                                                           double tolerance,
                                                           char** intersectionID);

/**
* @brief tiglIntersectGetLineCount returns the number of intersection lines computed by
* ::tiglIntersectComponents, ::tiglIntersectWithPlane or ::tiglIntersectWithPlaneSegment
* for the given intersectionID.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  intersectionID  The intersection identifier returned by ::tiglIntersectComponents or ::tiglIntersectWithPlane
* @param[out] lineCount       Number of intersection lines computed by ::tiglIntersectComponents or ::tiglIntersectWithPlane.
*                             If no intersection could be computed, line count is 0.
*
* @return
*   - TIGL_SUCCESS if no error occured
*   - TIGL_NOT_FOUND if the cpacs handle  or the intersectionID is not valid
*   - TIGL_NULL_POINTER if lineCount is a NULL pointer
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectGetLineCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                            const char* intersectionID,
                                                            int* lineCount);

/**
* @brief tiglGetCurveIntersectionCount returns the number of intersection points computed by
* ::tiglGetCurveIntersection for the given intersectionID.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  intersectionID  The intersection identifier returned by ::tiglGetCurveIntersection
* @param[out] pointCount      Number of intersection points computed by ::tiglGetCurveIntersection
*                             If no intersection could be computed, pointCount is 0.
*
* @return
*   - TIGL_SUCCESS if no error occured
*   - TIGL_NOT_FOUND if the cpacs handle is not valid
*   - TIGL_NULL_POINTER if either intersectionID or pointCount is a NULL pointer
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveIntersectionCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                const char* intersectionID,
                                                                int* pointCount);

/**
* @brief tiglIntersectGetPoint samples a point on an intersection line calculated by
* ::tiglIntersectComponents, ::tiglIntersectWithPlane or ::tiglIntersectWithPlaneSegment.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  intersectionID  The intersection identifier returned by ::tiglIntersectComponents or ::tiglIntersectWithPlane
* @param[in]  lineIdx         Line index to sample from. To get the number of lines, call ::tiglIntersectGetLineCount.
*                             1 <= lineIdx <= lineCount.
* @param[in]  eta             Parameter on the curve that determines the point position, with 0 <= eta <= 1.
* @param[out] pointX          X coordinate of the resulting point.
* @param[out] pointY          Y coordinate of the resulting point.
* @param[out] pointZ          Z coordinate of the resulting point.
*
* @return
*   - TIGL_SUCCESS if no error occured
*   - TIGL_NOT_FOUND if the cpacs handle  or the intersectionID is not valid
*   - TIGL_NULL_POINTER if pointX, pointY, or pointZ are NULL pointers
*   - TIGL_INDEX_ERROR if lineIdx is not in valid range
*   - TIGL_MATH_ERROR if eta is not in range 0 <= eta <= 1
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglIntersectGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                        const char* intersectionID,
                                                        int lineIdx,
                                                        double eta,
                                                        double* pointX,
                                                        double* pointY,
                                                        double* pointZ);

/**
* @brief tiglGetCurveIntersectionPoint returns an intersection point calculated by ::tiglGetCurveIntersection
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  intersectionID  The intersection identifier returned by ::tiglGetCurveIntersection
* @param[in]  pointIdx        Index of the intersection point. To get the number of intersection points,
*                             call ::tiglGetCurveIntersectionCount with 1 <= pointIdx <= pointCount.
* @param[out] pointX          X coordinate of the resulting point.
* @param[out] pointY          Y coordinate of the resulting point.
* @param[out] pointZ          Z coordinate of the resulting point.
*
* @return
*   - TIGL_SUCCESS if no error occured
*   - TIGL_NOT_FOUND if the cpacs handle is not valid
*   - TIGL_NULL_POINTER if pointX, pointY, or pointZ or the intersectionID are NULL pointers
*   - TIGL_INDEX_ERROR if pointIdx is not in valid range
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveIntersectionPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                                const char* intersectionID,
                                                                int pointIdx,
                                                                double* pointX,
                                                                double* pointY,
                                                                double* pointZ);

/**
@brief tiglGetCurveParameter  projects a point onto a curve of an intersection line.
The intersection line is specified by a curveID. The curveID can be calculated using
* ::tiglIntersectComponents, ::tiglIntersectWithPlane or ::tiglIntersectWithPlaneSegment.
* The function returns the parameter eta of the projected point on the curve.

*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  curveID         The id of the intersection line.
* @param[in]  curveIdx        The index of curve in the intersection.
* @param[in]  pointX          X coordinate of the point to be projected.
* @param[in]  pointY          Y coordinate of the point to be projected.
* @param[in]  pointZ          Z coordinate of the point to be projected.
* @param[out] eta             The parameter along the first curve.
*
* @return
*   - TIGL_SUCCESS if no error occured
*   - TIGL_NOT_FOUND if the cpacs handle is not valid
*   - TIGL_NULL_POINTER if intersectionID, curveID or eta are NULL pointers
*   - TIGL_INDEX_ERROR if curveIdx is not in valid range
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglGetCurveParameter (TiglCPACSConfigurationHandle cpacsHandle,
                                                         const char* curveID,
                                                         int curveIdx,
                                                         double pointX,
                                                         double pointY,
                                                         double pointZ,
                                                         double* eta);



/*@}*/
/*****************************************************************************************************/

/**
  \defgroup Export Export Functions
            Functions for export of wings/fuselages.


# VTK-Export #
    There a various different VTK exports functions in TIGL. All functions starting with 'tiglExportVTK[Fuselage|Wing]...' are exporting
    a special triangulation with no duplicated points into a VTK file formated in XML (file extension .vtp) with some custom
    informations added to the file.

    In addition to the triangulated geometry, additional data are written to the VTK file. These data currently include:
    - uID: The UID of the fuselage or wing component segment on which the triangle exists.
    - segmentIndex: The segmentIndex of the fuselage or wing component segment on which the triangle exists. Kind of redundant to the UID.
    - eta/xsi: The parameters in the surface parametrical space of the triangle.
    - isOnTop: Flag that indicates whether the triangle is on the top of the wing or not. Please see the cpacs documentation how "up"
               is defined for wings.

    Please note that at this time these information are only valid for wings!

    There are two ways, how these additional data are attached to the VTK file. The first is the official VTK way to declare additional
    data for each polygon/triangle. For each data entry, a \<DataArray\> tag is added under the xpath
    @verbatim
    /VTKFile/PolyData/Piece/CellData
    @endverbatim

    Each CellData contains a vector(list) of values, each of them corresponding the data of one triangle. For example the data
    entry for the wing segment eta coordinates for 4 triangles looks like.
    @verbatim
    <DataArray type="Float64" Name="eta" NumberOfComponents="1"
        format="ascii" RangeMin="0.000000" RangeMax="1.000000">
            0.25 0.5 0.75 1.0
    </DataArray>
    @endverbatim

    The second way these data are stored is by using the "MetaData" mechanism of VTK. Here, a \<MetaData\> tag is added under the xpath
    @verbatim
    /VTKFile/PolyData/Piece/Polys
    @endverbatim

    A typical exported MetaData tag looks like the following:
    @verbatim
    <MetaData elements="uID segmentIndex eta xsi isOnTop">
      "rootToInnerkink" 1 3.18702 0.551342 0
      "rootToInnerkink" 1 2.93939 0.581634 0
      "rootToInnerkink" 1 4.15239 0.520915 0
      ...
    </MetaData>
    @endverbatim

    The 'elements' attribute indicates the number and the names of the additional information tags as a whitespace separated list. In
    this example you could see 5 information fields with the name.
 */

/*@{*/

/**
* @brief Sets options for the geometry export
* 
* Generic options for all exporters:
*   - ApplySymmetries (Values: "true", "false"): Whether symmetric cpacs objects (e.g. wings) should be written.
*   - IncludeFarfield (Values: "true", "false"): Whether to include the far field into the export or not.
*   - ShapeGroupMode  (Values: "WHOLE_SHAPE", "NAMED_COMPOUNDS", "FACES"): Adjust, how shapes are grouped.
* 
* Exporter-specific options:
*  - VTK:
*    - WriteNormals (Values: "true", "false"): Whether to write normal vectors or not.
*      To avoid duplicate vertices, normals should be disabled.
*    - MultiplePieces (Values: "true", "false"): Whether to export the shapes into multiple vtk pieces.
*    - WriteMetaData (Values: "true", "false"): Whether to add meta data (e.g. wing segments etc...)
*  - IGES:
*    - IGES5.3 (Values: "true", "false"): Whether to use IGES 5.3 format, that supports shells and solids.
*      Note: Some software do not yet implement this standard. E.g. Catia might only load this, when set to false!
*
* Example: The IGES export normally does only write half-models. It does not apply symmetries.
* to change this, just call 
  @verbatim
  tiglSetExportOptions("iges", "ApplySymmetries", "true");
  @endverbatim
* 
* @param[in] exporter_name File format of the export. E.g. "vtk", "iges", "step", "collada", "brep" or "stl"
* @param[in] option_name   Name of the option to be set
* @param[in] option_value  Value of the options to be set
*
* @return 
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if the specified exporter or the option does not exist
*   - TIGL_NULL_POINTER if exporter_name, option_name or option_value is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglSetExportOptions(const char* exporter_name, const char* option_name, const char* option_value);

/**
* @brief Exports a geometric component (e.g. a wing, a fuselage etc.)
* 
* The component to be exported is definied via its uid. The export format
* is specified with the file extension of fileName.
* 
* The export can be configured in more detail using ::tiglSetExportOptions.
* 
* Supported file formats are: STEP (*.stp), IGES (*.igs), VTK (*.vtp), Collada (*.dae), STL (*.stl) and BREP (*.brep)
* 
* Example: Export a wing to IGES format
  @verbatim
  tiglExportComponent(handle, "MyWingUID", "wing.igs", 0.);
  @endverbatim
* 
* @param[in] cpacsHandle Handle for the CPACS configuration
* @param[in] uid         Uid of the component to be exported
* @param[in] fileName    File name of the exported file
* @param[in] deflection  Deflection parameter. This is only used for meshed exports (e.g. VTK, STL, Collada).
*
* @return 
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no exporter is found for the specified file name
*   - TIGL_NULL_POINTER if uid or fileName is a null pointer
*   - TIGL_WRITE_FAILED if the file could not be written
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportComponent(TiglCPACSConfigurationHandle cpacsHandle,
                                                      const char* uid,
                                                      const char* fileName,
                                                      double deflection);


/**
* @brief Exports the whole configuration into a file.
* 
* It can be specified, whether the configuration should be exported as a fused geometry (i.e.
* all shapes are trimmed with each other). The export format is specified with the
* file extension of fileName.
* 
* The export can be configured in more detail using ::tiglSetExportOptions.
* 
* Supported file formats are: STEP (*.stp), IGES (*.igs), VTK (*.vtp), Collada (*.dae), STL (*.stl) and BREP (*.brep)
* 
* Example: Export the fused configuration to STEP format
  @verbatim
  tiglExportConfiguration(handle, "aircraft.stp", TIGL_TRUE, 0.);
  @endverbatim
*
* @param[in] cpacsHandle   Handle for the CPACS configuration
* @param[in] fileName      File name of the exported file
* @param[in] fuseAllShapes Whether to fuse the geometry of not. Fusing can take a lot of time!
* @param[in] deflection    Deflection parameter. This is only used for meshed exports (e.g. VTK, STL, Collada).
*
* @return 
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no exporter is found for the specified file name
*   - TIGL_NULL_POINTER if fileName is a null pointer
*   - TIGL_WRITE_FAILED if the file could not be written
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportConfiguration(TiglCPACSConfigurationHandle cpacsHandle,
                                                          const char* fileName,
                                                          TiglBoolean fuseAllShapes,
                                                          double deflection);


/**
* @brief Exports the geometry of a CPACS configuration to IGES format.
*
* To maintain compatibility with CATIA, the file suffix should be ".igs".
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filenamePtr Pointer to an IGES export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportIGES(TiglCPACSConfigurationHandle cpacsHandle,
                                                 const char* filenamePtr);

/**
* @brief Exports the boolean fused geometry of a CPACS configuration to IGES format.
*
* To maintain compatibility with CATIA, the file suffix should be ".igs".
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filenamePtr Pointer to an IGES export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedWingFuselageIGES(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  const char* filenamePtr);


// ***************
//       STEP
// ***************

/**
* @brief Exports the geometry of a CPACS configuration to STEP format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filenamePtr Pointer to an STEP export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportSTEP(TiglCPACSConfigurationHandle cpacsHandle,
                                                 const char* filenamePtr);

/**
* @brief Exports the fused/trimmed geometry of a CPACS configuration to STEP format.
*
* In order to fuse the geometry, boolean operations are performed. Depending on the
* complexity of the configuration, the fusing can take several minutes.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filenamePtr Pointer to an STEP export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedSTEP(TiglCPACSConfigurationHandle cpacsHandle,
                                                      const char* filenamePtr);


// ***************
//       STL
// ***************
/**
* @brief Exports the boolean fused geometry of a wing meshed to STL format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  wingIndex   Index of the Wing to export
* @param[in]  filenamePtr Pointer to an STL export file name
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingSTL(TiglCPACSConfigurationHandle cpacsHandle, 
                                                          int wingIndex,
                                                          const char* filenamePtr, 
                                                          double deflection); 

/**
* @brief Exports the boolean fused geometry of a wing meshed to STL format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  wingUID     UID of the Wing to export
* @param[in]  filenamePtr Pointer to an STL export file name
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingSTLByUID(TiglCPACSConfigurationHandle cpacsHandle, 
                                                               const char* wingUID,
                                                               const char* filenamePtr, 
                                                               double deflection); 



/**
* @brief Exports the boolean fused geometry of a fuselage meshed to STL format.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex Index of the Fuselage to export
* @param[in]  filenamePtr   Pointer to an STL export file name
* @param[in]  deflection    Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageSTL(TiglCPACSConfigurationHandle cpacsHandle, 
                                                              int fuselageIndex,
                                                              const char* filenamePtr, 
                                                              double deflection);


/**
* @brief Exports the boolean fused geometry of a fuselage meshed to STL format.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageUID   UID of the Fuselage to export
* @param[in]  filenamePtr   Pointer to an STL export file name
* @param[in]  deflection    Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageSTLByUID(TiglCPACSConfigurationHandle cpacsHandle, 
                                                                   const char* fuselageUID,
                                                                   const char* filenamePtr, 
                                                                   double deflection);

/**
* @brief Exports the boolean fused geometry of the whole configuration meshed to STL format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filenamePtr Pointer to an STL export file name
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometrySTL(TiglCPACSConfigurationHandle cpacsHandle, 
                                                              const char* filenamePtr,
                                                              double deflection);

/**
* @brief Exports the boolean fused geometry of a wing (selected by id) meshed to VTK format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  wingIndex   Index of the Wing to export
* @param[in]  filenamePtr Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                                 const int wingIndex,
                                                                 const char* filenamePtr, 
                                                                 const double deflection);



/**
* @brief Exports the boolean fused geometry of a wing (selected by UID) meshed to VTK format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  wingUID     UID of the Wing to export
* @param[in]  filenamePtr Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if the wing UID does not exists
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                               const char* wingUID,
                                                               const char* filenamePtr, 
                                                               double deflection);



/**
* @brief Exports the boolean fused geometry of a fuselage (selected by index) meshed to VTK format.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageIndex Index of the Fuselage to export
* @param[in]  filenamePtr   Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param[in]  deflection    Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                                     const int fuselageIndex,
                                                                     const char* filenamePtr, 
                                                                     double deflection);


/**
* @brief Exports the boolean fused geometry of a fuselage (selected by uid) meshed to VTK format.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageUID   UID of the Fuselage to export
* @param[in]  filenamePtr   Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param[in]  deflection    Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageUID does not exists
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                                   const char* fuselageUID,
                                                                   const char* filenamePtr, 
                                                                   double deflection);


/**
* @brief Exports the boolean fused geometry of the whole configuration meshed to VTK format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filenamePtr Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTK(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                              const char* filenamePtr,
                                                              double deflection);





/**
* @brief Exports the boolean fused geometry of a wing (selected by UID) meshed to VTK format.
*
* This function does only a very simple, but also very fast meshing on the wing segments and exports them to
* a VTK file. No additional CPACS relevant information are computed.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  wingUID     UID of the Wing to export
* @param[in]  filenamePtr Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if the wing UID does not exists
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedWingVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                                     const char* wingUID,
                                                                     const char* filenamePtr, 
                                                                     double deflection);




/**
* @brief Exports the boolean fused geometry of the whole configuration meshed to VTK format.
*
* This function does only a very simple, but also very fast meshing on the geometry and exports them to
* a VTK file. No additional CPACS relevant information are computed.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filenamePtr Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param[in]  deflection  Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportMeshedGeometryVTKSimple(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                                    const char* filenamePtr,
                                                                    double deflection);

/**
* @brief Exports the boolean fused geometry of a fuselage (selected by uid) meshed to Collada (*.dae) format.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  fuselageUID   UID of the Fuselage to export
* @param[in]  filename      Filename of the resulting collada file. It should contain the .dae file name ending.
* @param[in]  deflection    Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filename is a null pointer
*   - TIGL_INDEX_ERROR if fuselageUID does not exists
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportFuselageColladaByUID(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                                 const char* fuselageUID, 
                                                                 const char* filename, 
                                                                 double deflection);


/**
* @brief Exports the boolean fused geometry of a wing (selected by uid) meshed to Collada (*.dae) format.
*
*
* @param[in]  cpacsHandle   Handle for the CPACS configuration
* @param[in]  wingUID       UID of the Wing to export
* @param[in]  filename      Filename of the resulting collada file. It should contain the .dae file name ending.
* @param[in]  deflection    Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filename is a null pointer
*   - TIGL_INDEX_ERROR if fuselageUID does not exists
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportWingColladaByUID(const TiglCPACSConfigurationHandle cpacsHandle, 
                                                             const char* wingUID, 
                                                             const char* filename, 
                                                             double deflection);

/**
* @brief Exports the fused/trimmed geometry of a CPACS configuration to BREP format.
*
* In order to fuse the geometry, boolean operations are performed. Depending on the
* complexity of the configuration, the fusing can take several minutes.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  filename    BREP export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportFusedBREP(TiglCPACSConfigurationHandle cpacsHandle,
                                                      const char* filename);

/**
* @brief Exports the boolean fused geometry of a fuselage (selected by uid) to BREP format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  fuselageUID UID of the Fuselage to export
* @param[in]  filename    BREP export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportFuselageBREPByUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                              const char* fuselageUID,
                                                              const char* filename);

/**
* @brief Exports the boolean fused geometry of a wing (selected by uid) to BREP format.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  wingUID UID of the Fuselage to export
* @param[in]  filename    BREP export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglExportWingBREPByUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                              const char* wingUID,
                                                              const char* filename);

/*@}*/
/*****************************************************************************************************/

/**
  \defgroup Material Material functions
    Functions to query material information of wings/fuselages. Materials are currently ony implemented for the wing component segment.
    Here, materials for the lower and upper wing surface can be queried, which can be a material for the whole skin/surface
    or a material defined inside a wing cell. A wing cell material overwrites the global skin material, i.e. if the whole
    wing skin material is aluminum and the trailing edge is made of radar absorbing material, only the absorbing material
    is returned by the querying functions.
 */
/*@{*/


/**
* @brief Returns the number of materials defined at a point on the wing component segment surface.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  compSegmentUID  UID of the component segment
* @param[in]  structureType   Type of structure, where the materials are queried
* @param[in]  eta             eta in the range 0.0 <= eta <= 1.0
* @param[in]  xsi             xsi in the range 0.0 <= xsi <= 1.0
* @param[out] materialCount   Number of materials defined at the given coordinate
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if compSegmentUID or materialCount is a null pointer
*   - TIGL_INDEX_ERROR if compSegmentUID or materialIndex is invalid
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetMaterialCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                           const char *compSegmentUID,
                                                                           TiglStructureType structureType,
                                                                           double eta, double xsi,
                                                                           int * materialCount);
 
/**
* @brief Returns one of the material UIDs of a given point on the wing component segment surface.
* The number of materials on that point has to be first queried using ::tiglWingComponentSegmentGetMaterialCount.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  compSegmentUID  UID of the component segment
* @param[in]  structureType   Type of structure, where the materials are queried
* @param[in]  eta             eta in the range 0.0 <= eta <= 1.0
* @param[in]  xsi             xsi in the range 0.0 <= xsi <= 1.0
* @param[in]  materialIndex   Index of the material to query (1 <= index <= materialCount)
* @param[out] uid             Material uid at the given coordinate
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if compSegmentUID is a null pointer
*   - TIGL_INDEX_ERROR if compSegmentUID or materialIndex is invalid
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetMaterialUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                         const char *compSegmentUID,
                                                                         TiglStructureType structureType,
                                                                         double eta, double xsi,
                                                                         int materialIndex,
                                                                         char ** uid);

/**
* @brief Returns one of the material thicknesses of a given point on the wing component segment surface.
* The number of materials on that point has to be first queried using ::tiglWingComponentSegmentGetMaterialCount.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  compSegmentUID  UID of the component segment
* @param[in]  structureType   Type of structure, where the materials are queried
* @param[in]  eta             eta in the range 0.0 <= eta <= 1.0
* @param[in]  xsi             xsi in the range 0.0 <= xsi <= 1.0
* @param[in]  materialIndex   Index of the material to query (1 <= index <= materialCount)
* @param[out] thickness       Material thickness at the given coordinate. If no thickness is defined, thickness gets a negative value and TIGL_UNINITIALIZED is returned.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if compSegmentUID or thickness is a null pointer
*   - TIGL_INDEX_ERROR if compSegmentUID or materialIndex is invalid
*   - TIGL_UNINITIALIZED if no thickness is defined for the material
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingComponentSegmentGetMaterialThickness(TiglCPACSConfigurationHandle cpacsHandle,
                                                                               const char *compSegmentUID,
                                                                               TiglStructureType structureType,
                                                                               double eta, double xsi,
                                                                               int materialIndex,
                                                                               double * thickness);
     


/*@}*/
/*****************************************************************************************************/

/**
  \defgroup VolumeFunctions Functions for volume calculations
    Function for volume calculations on wings/fuselages.
 */
/*@{*/

/**
* @brief Returns the volume of the fuselage.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   Index of the fuselage to calculate the volume, starting at 1
* @param[out] volumePtr       The volume of the fuselage
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                        int fuselageIndex,
                                                        double *volumePtr);



/**
* @brief Returns the volume of the wing.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       Index of the Wing to calculate the volume, starting at 1
* @param[out] volumePtr       The volume of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetVolume(TiglCPACSConfigurationHandle cpacsHandle, 
                                                    int wingIndex,
                                                    double *volumePtr);



/**
* @brief Returns the volume of a segment of a wing.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] volumePtr       The pointer to a variable for the volume of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if volumePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int wingIndex,
                                                           int segmentIndex,
                                                           double* volumePtr);


/**
* @brief Returns the volume of a segment of a fuselage.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] volumePtr       The pointer to a variable for the volume of the fuselage
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if volumePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                               int fuselageIndex,
                                                               int segmentIndex,
                                                               double* volumePtr);



/*@}*/
/*****************************************************************************************************/

/**
  \defgroup SurfaceAreaFunctions Functions for surface area calculations
    Function for surface area calculations of wings/fuselages.
 */
/*@{*/

/**
* @brief Returns the surface area of the wing. Currently, the area includes also the faces
* on the wing symmetry plane (in case of a symmetric wing). In coming releases, these faces will
* not belong anymore to the surface area calculation.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       Index of the Wing to calculate the area, starting at 1
* @param[out] surfaceAreaPtr  The surface area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                         int wingIndex,
                                                         double *surfaceAreaPtr);


/**
* @brief Returns the surface area of the fuselage. Currently, the area includes also the faces
* on the fuselage symmetry plane (in case of a symmetric wing). This is in particular a problem
* for fuselages, where only one half side is defined in CPACS. In future releases, these faces will
* not belong anymore to the surface area calculation.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   Index of the Fuselage to calculate the area, starting at 1
* @param[out] surfaceAreaPtr  The surface area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                             int fuselageIndex,
                                                             double *surfaceAreaPtr);


/**
* @brief Returns the surface area of a segment of a wing. This includes only the area
* of the upper and lower wing segment surface and does not include the trailing egde
* or any closing faces.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] surfaceAreaPtr  The pointer to a variable for the surface area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                double* surfaceAreaPtr);

/**
* @brief Computes the area of the trimmed upper wing segment surface. This function can be e.g. used
* to determine the area of the wing flaps.
* 
* The computed area does not include the trailing edge or any closing side faces.
* 
* All eta and xsi values must be in the range [0,1]. The trimmed area is defined with the 
* four corner point P1, P2, P3, and P4. The order of the points should be right handed, as
* shown the the image below.
* 
* @image html surfaceAreaTrimmed.png "Location of the four corner points"
* @image latex surfaceAreaTrimmed.pdf "Location of the four corner points" width=10cm
* 
* Each of the points is defined with an eta/xsi coordinate pair in the wing segment system.
* 
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[in]  eta1            Eta value of P1 in range [0,1]
* @param[in]  xsi1            Xsi value of P1 in range [0,1]
* @param[in]  eta2            Eta value of P2 in range [0,1]
* @param[in]  xsi2            Xsi value of P2 in range [0,1]
* @param[in]  eta3            Eta value of P3 in range [0,1]
* @param[in]  xsi3            Xsi value of P3 in range [0,1]
* @param[in]  eta4            Eta value of P4 in range [0,1]
* @param[in]  xsi4            Xsi value of P4 in range [0,1]
* @param[out] surfaceArea     Area of the trimmed upper wing surface
* 
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex ot segmentIndex are not valid
*   - TIGL_NULL_POINTER if surfaceArea is a null pointer
*   - TIGL_ERROR if the eta/xsi coordinates are not in the valid range [0,1] or another error occured
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentUpperSurfaceAreaTrimmed(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int wingIndex,
                                                                            int segmentIndex,
                                                                            double eta1, double xsi1,
                                                                            double eta2, double xsi2,
                                                                            double eta3, double xsi3,
                                                                            double eta4, double xsi4,
                                                                            double* surfaceArea);

/**
* @brief Computes the area of the trimmed lower wing segment surface. This function can be e.g. used
* to determine the area of the wing flaps.
*
* The use of this function is analog to ::tiglWingGetSegmentUpperSurfaceAreaTrimmed.
* 
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingIndex       The index of a wing, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[in]  eta1            Eta value of P1 in range [0,1]
* @param[in]  xsi1            Xsi value of P1 in range [0,1]
* @param[in]  eta2            Eta value of P2 in range [0,1]
* @param[in]  xsi2            Xsi value of P2 in range [0,1]
* @param[in]  eta3            Eta value of P3 in range [0,1]
* @param[in]  xsi3            Xsi value of P3 in range [0,1]
* @param[in]  eta4            Eta value of P4 in range [0,1]
* @param[in]  xsi4            Xsi value of P4 in range [0,1]
* @param[out] surfaceArea     Area of the trimmed lower wing surface
* 
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex ot segmentIndex are not valid
*   - TIGL_NULL_POINTER if surfaceArea is a null pointer
*   - TIGL_ERROR if the eta/xsi coordinates are not in the valid range [0,1] or another error occured
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSegmentLowerSurfaceAreaTrimmed(TiglCPACSConfigurationHandle cpacsHandle,
                                                                            int wingIndex,
                                                                            int segmentIndex,
                                                                            double eta1, double xsi1,
                                                                            double eta2, double xsi2,
                                                                            double eta3, double xsi3,
                                                                            double eta4, double xsi4,
                                                                            double* surfaceArea);

/**
* @brief Returns the surface area of a segment of a fuselage.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  fuselageIndex   The index of a fuselage, starting at 1
* @param[in]  segmentIndex    The index of a segment, starting at 1
* @param[out] surfaceAreaPtr  The pointer to a variable for the surface area of the fuselage-segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglFuselageGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                                    int fuselageIndex,
                                                                    int segmentIndex,
                                                                    double* surfaceAreaPtr);
        
/**
* @brief Returns the reference area of the wing.
*
* The reference area of the wing is calculated by taking account the quadrilateral portions
* of each wing segment by projecting the wing segments into the plane defined by the user.
* If projection should be avoided, use TIGL_NO_SYMMETRY as symPlane argument.
*
*
* @param[in]  cpacsHandle       Handle for the CPACS configuration
* @param[in]  wingIndex         Index of the Wing to calculate the area, starting at 1
* @param[in]  symPlane          Plane on which the wing is projected for calculating the refarea. Values can be:
*                                  - TIGL_NO_SYMMETRY, the wing is not projected but its true 3D area is calculated
*                                  - TIGL_X_Y_PLANE, the wing is projected onto the x-y plane (use for e.g. main wings and HTPs)
*                                  - TIGL_X_Z_PLANE, the wing is projected onto the x-z plane (use for e.g. VTPs)
*                                  - TIGL_Y_Z_PLANE, the wing is projected onto the y-z plane
*
* @param[out] referenceAreaPtr  The reference area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetReferenceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                           int wingIndex, 
                                                           TiglSymmetryAxis symPlane,
                                                           double *referenceAreaPtr);

/**
* @brief Returns the wetted area of the wing.
*
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  wingUID         UID of the Wing to calculate the wetted area
* @param[out] wettedAreaPtr   The wetted area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if wingUID is wrong
*   - TIGL_NULL_POINTER if wingIUD is NULL
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetWettedArea(TiglCPACSConfigurationHandle cpacsHandle, 
                                                        char* wingUID,
                                                        double *wettedAreaPtr);

/*@}*/
/*****************************************************************************************************/
/**
  \defgroup GeometryFunctions General geometry functions.
 */
/*@{*/

/**
* @brief Returns the number of curves the given profile is made of.
* 
* The given profile may be a fuselage profile or a wing profile. Typically,
* wing profiles consist of two curves (lower and upper curve), fuselage profiles
* consist of only one curve.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  profileUID      UID of the profile
* @param[out] curveCount      Number of curves
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if profileUID is wrong
*   - TIGL_NULL_POINTER if profileUID or curveCount NULL
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglProfileGetBSplineCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                             const char* profileUID,
                                                             int* curveCount);

/**
* @brief Returns the B-Spline data sizes for a given curve on a profile. This includes
* size of the knot vector, size of the control point vector and degree of the spline.
* 
* The given profile may be a fuselage profile or a wing profile. Typically,
* wing profiles consist of two curves (lower and upper curve), fuselage profiles
* consist of only one curve.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  profileUID      UID of the profile
* @param[in]  curveid         Index of the curve. Number of curves must be queried 
*                             with ::tiglProfileGetBSplineCount. 1 <= index <= count
* @param[out] degree          Degree of the B-Spline
* @param[out] nControlPoints  Size of the control point vector
* @param[out] nKnots          Size of the knot vector
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if profileUID is wrong
*   - TIGL_INDEX_ERROR if curveid knot in range [1, curveCount]
*   - TIGL_NULL_POINTER if the argument profileUID, degree, ncontrolPoints, or nKnots are NULL
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglProfileGetBSplineDataSizes(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 const char* profileUID,
                                                                 int curveid,
                                                                 int* degree,
                                                                 int* nControlPoints,
                                                                 int* nKnots);
/**
* @brief Returns the B-Spline data of the given profile curve. This includes 
* the knot vector and the control points of the B-Spline.
* 
* The output arrays cpx, cpy, cpz, and knots have to be allocated by the user first.
* The control point vector arrays must have the size nControlPoints. This value must 
* be queried with ::tiglProfileGetBSplineDataSizes first.
* The knot vector array must have the size kKnots. This value has to be queried also
* using the function ::tiglProfileGetBSplineDataSizes.
* 
* The given profile may be a fuselage profile or a wing profile. Typically,
* wing profiles consist of two curves (lower and upper curve), fuselage profiles
* consist of only one curve.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  profileUID      UID of the profile
* @param[in]  curveid         Index of the curve. Number of curves must be queried 
*                             with ::tiglProfileGetBSplineCount. 1 <= index <= count
* @param[in]  nControlPoints  Size of the control point vector. To be queried with ::tiglProfileGetBSplineDataSizes first.
* @param[out] cpx             X-values of the control point vector.
* @param[out] cpy             Y-values of the control point vector.
* @param[out] cpz             Z-values of the control point vector.
* @param[in]  nKnots          Size of the knot vector. To be queried with ::tiglProfileGetBSplineDataSizes first.
* @param[out] knots           Knot vector values.
* 
* @cond
* #annotate out: 4AM(3), 5AM(3), 6AM(3), 8AM(7)#
* @endcond
* 
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_UID_ERROR if profileUID is wrong
*   - TIGL_INDEX_ERROR if curveid knot in range [1, curveCount]
*   - TIGL_NULL_POINTER if the argument profileUID, cpx, cpy, cpz, or knots are NULL
*   - TIGL_ERROR if the values nControlPoints, or nKnots are wrong
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglProfileGetBSplineData(TiglCPACSConfigurationHandle cpacsHandle,
                                                            const char* profileUID,
                                                            int curveid,
                                                            int nControlPoints, double* cpx, double* cpy, double* cpz,
                                                            int nKnots, double* knots);

/*@}*/
/*****************************************************************************************************/
/**
  \defgroup LoggingFunctions Logging functions.
    The following functions are used to customize the behaviour how messages are handled by TiGL. By default,
    only error messages and warnings are printed to the console.

    In addition, TiGL can be told to write message into a log file using the function
    ::tiglLogToFileEnabled and ::tiglLogToFileStreamEnabled. By enabling file logging, other log messages than
    errors and warnings can be inspected. File logging is disabled by default.

    In order to change the verbosity of the TiGL messages printed on console, use ::tiglLogSetVerbosity.
 */
/*@{*/

/**
* @brief Sets up the tigl logging mechanism to send all log messages into a file.
*
* Typically this function has to be called before opening any cpacs configuration. The console logging mechanism remains untouched.
*
*
* @param[in]  filePrefix Prefix of the filename to be created. The filename consists of the prefix, a date and time string and the ending ".log".
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NULL_POINTER if filePrefix is NULL
*   - TIGL_OPEN_FAILED if file can not be opened for writing
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileEnabled(const char* filePrefix);

/**
* @brief Sets up the tigl logging mechanism to send all log messages into an already opened file.
*
* In contrast to ::tiglLogToFileEnabled, the messages are appended
* to an already opened file. This file might be an already opened logging file set
* up by another library or program. The console logging mechanism remains untouched.
*
* Typically this function has to be called before opening any cpacs configuration.
*
*
* @param[in]  fp File pointer to an already opened file.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NULL_POINTER if fp is NULL , i.e. not opened for writing.
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileStreamEnabled(FILE * fp);

/**
* @brief Disabled file logging. If a log file is currently opened by TiGL it will be closed. The log messages
* are printed to console. This is the default logging mechanism of TIGL.
*
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogToFileDisabled();

/**
* @brief Sets the file ending for logging files. Default is "log".
*
* This function has to be called before ::tiglLogToFileEnabled to have the
* desired effect.
*
*
* @param[in]  ending File ending of the logging file. Default is "log".
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NULL_POINTER if ending is NULL
*   - TIGL_ERROR if some error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetFileEnding(const char* ending);

/**
* @brief Enables or disables appending a unique date/time identifier inside
* the log file name (behind the file prefix). By default, the time identifier
* is enabled.
*
* This function has to be called before ::tiglLogToFileEnabled to have the
* desired effect.
*
*
* @param[in]  enabled Set to true, if time identifier should be enabled.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_ERROR if some error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetTimeInFilenameEnabled(TiglBoolean enabled);

/**
* @brief Set the console verbosity level.
*
* This function shall be used change, what kind of logging information is displayed
* on the console. By default, only errors and warnings are printed on console.
*
*
* @param[in]  level Verbosity level for console messages.
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_ERROR if some error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglLogSetVerbosity(TiglLogLevel level);

/*@}*/ // end of doxygen group

/*@}*/
/*****************************************************************************************************/
/**
  \defgroup ComponentUtilityFunctions Generic utility functions.
    Generic utility functions for geometric components that fits not only to wings _or_ fuselages.
 */
/*@{*/

/**
 * @brief Checks whether a point lies inside the given geometric object.
 *
 * This function works only for solid objects!
 *
 * @param[in]  cpacsHandle   Handle for the CPACS configuration
 * @param[in]  px            X coordinate of point
 * @param[in]  py            Y coordinate of point
 * @param[in]  pz            Z coordinate of point
 * @param[in]  componentUID  UID of the component (e.g. a fuselage UID, wing UID, wing segment UID ...)
 * @param[out] isInside      Returns, whether the point lies inside the object or not.
 *
 * @return
 *   - TIGL_SUCCESS if no error occurred.
 *   - TIGL_NOT_FOUND if no configuration was found for the given handle.
 *   - TIGL_UID_ERROR if the component does not exist or is not a geometric object.
 *   - TIGL_NULL_POINTER if componentUID or isInside are null.
 *   - TIGL_ERROR if some error occurred, e.g. the component is not a solid object.
 */
TIGL_COMMON_EXPORT TiglReturnCode tiglCheckPointInside(TiglCPACSConfigurationHandle cpacsHandle,
                                                       double px, double py, double pz,
                                                       const char* componentUID,
                                                       TiglBoolean* isInside);

/**
* @brief Returns a unique HashCode for a geometric component. 
* 
* The component, for example a wing or a fuselage, could be specified via
* its UID. The HashCode is the same as long as the geometry of this component
* has not changed. The HashCode is valid through the current session only!
* The hash value is computed from the value of the underlying shape reference.
*
* @param[in]  cpacsHandle     Handle for the CPACS configuration
* @param[in]  componentUID    The uid of the component for which the hash should be computed
* @param[out] hashCodePtr     The pointer to a hash value to represent this shape
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglComponentGetHashCode(TiglCPACSConfigurationHandle cpacsHandle,
                                                           const char* componentUID,
                                                           int* hashCodePtr);


/**
* @brief Translates an error code into a string
*
*
* @param[in]  errorCode Return value of a tigl function
* 
* @returns Error code as a string.
*/
TIGL_COMMON_EXPORT const char * tiglGetErrorString(TiglReturnCode errorCode);


/**
* @brief Returns the length of the plane
*
* The calculation of the airplane lenght is realized as follows:
*
* All part of the configuration (currently all wing and fuselage segments) are put
* into a bounding box. The length of the plane is returned as the length of the box
* in x-direction.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[out] pLength     Length of plane
*
* @returns Error code
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglConfigurationGetLength(TiglCPACSConfigurationHandle cpacsHandle, double * pLength);

/**
* @brief Returns the span of a wing. 
*
* The calculation of the wing span is realized as follows:
*
* * If the wing is mirrored at a symmetry plane (like the main wing), the wing body and its mirrored counterpart are computed
* and are put into a bounding box. The length of the box in a specific space dimension is returned as the wing span depending
* on the symmetry plane (y direction for x-z planes, z direction for x-y planes, x direction for y-z symmetry planes).
*
* * If no symmetry plane is defined (e.g. for the fins), the largest dimension of the bounding box around the wing
* is returned.
*
*
* @param[in]  cpacsHandle Handle for the CPACS configuration
* @param[in]  wingUID     UID of the Wing
* @param[out] pSpan       Wing span
*
* @returns Error code
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetSpan(TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID, double * pSpan);


/**
* @brief This function calculates location of the quarter of mean aerodynamic chord, and gives the chord lenght as well.
* 
* It uses the classical method that can be applied to trapozaidal wings. This method is used for each segment.
* The values are found by taking into account of sweep and dihedral. But the effect of insidance angle is neglected.
* These values should coinside with the values found with tornado tool.
* 
* @param[in] cpacsHandle Handle for the CPACS configuration
* @param[in] wingUID     UID of the Wing
* @param[out] mac_chord  Mean areadynamic chord length
* @param[out] mac_x, mac_y, mac_z - Position of the MAC
* 
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NULL_POINTER if wingUID, mac_chord, mac_x, mac_y or mac_z are null pointers
*   - TIGL_ERROR In case of an unknown error
*/
TIGL_COMMON_EXPORT TiglReturnCode tiglWingGetMAC(TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID, double *mac_chord, double *mac_x, double *mac_y, double *mac_z);


/*@}*/ // end of doxygen group

#endif /* TIGL_H */

#ifdef __cplusplus
}
#endif
