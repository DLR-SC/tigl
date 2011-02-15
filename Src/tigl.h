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
/**
* @file
* @brief  Declaration of the TIGL C interface.
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32_DLL
#   define DLL_EXPORT __declspec (dllexport)
#else
#   define DLL_EXPORT
#endif


#ifndef TIGL_H
#define TIGL_H

#include "tixi.h"

/**
 \defgroup Enums Enumerations
 */

/**
 \ingroup Enums
 Definition of possible error return codes of some functions.
 */
enum TiglReturnCode
{
    TIGL_SUCCESS            = 0,
    TIGL_ERROR              = 1,
    TIGL_NULL_POINTER       = 2,
    TIGL_NOT_FOUND          = 3,
    TIGL_XML_ERROR          = 4,
    TIGL_OPEN_FAILED        = 5,
    TIGL_CLOSE_FAILED       = 6,
    TIGL_INDEX_ERROR        = 7,
    TIGL_STRING_TRUNCATED   = 8,
    TIGL_WRONG_TIXI_VERSION = 9
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
*
*/
typedef enum TiglReturnCode TiglReturnCode;

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
*	- TIGL_X_Z_PLANE
*	- TIGL_Y_Z_PLANE
*
*/
typedef enum TiglSymmetryAxis TiglSymmetryAxis;

/**
 \ingroup Enums Definition of possible algorithms used in calculations. Use
  these constants to define the algorithm to be used e.g. in interpolations.
*/
enum TiglAlgorithmCode
{
    TIGL_INTERPOLATE_LINEAR_WIRE  = 0,          /**< Use a linear interpolation between the points of a wire */
    TIGL_INTERPOLATE_BSPLINE_WIRE = 1,         /**< Use a BSpline interpolation between the points of a wire */
    TIGL_APPROXIMATE_BSPLINE_WIRE = 2          /**< Use a BSpline approximation for the points of a wire */
};


/**
 \ingroup Enums Definition of possible types for geometric components. Used for calculations where
  the type if the component changes the way of behavior.
*/
enum TiglGeometricComponentType
{
    TIGL_COMPONENT_FUSELAGE  = 0,        /**< The Component is a fuselage */
    TIGL_COMPONENT_WING      = 1,        /**< The Component is a wing */
};


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
	TIGL_IMPORTEXPORT_IGES = 0,			/**< Use IGES format for geometry import/export */
	TIGL_IMPORTEXPORT_STEP = 1,			/**< Use STEP format for geometry import/export */
	TIGL_IMPORTEXPORT_STL  = 2,			/**< Use STL format for geometry import/export */
    TIGL_IMPORTEXPORT_VTK  = 3          /**< Use VTK (XML/VTP) format for geometry import/export */
};


/**
 * @brief Definition of the TIGL version number.
 */
#define TIGL_VERSION  "0.9"


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
* <b>Fortran syntax:</b>
*
* tigl_open_cpacs_configuration(integer tixiHandle, character configurationUID, integer cpacsHandlePtr, integer returnCode)
*
*
* @param tixiHandle     (in) : Handle to a TIXI document. The TIXI document should not be closed until the
*                              CPACS configuration is closed. First close the CPACS configuration, then
*                              the TIXI document.
* @param configurationUID (in) : The UID of the configuration that shloud be loaded by TIGL.
* @param cpacsHandlePtr (out): Handle to the CPACS configuration. This handle is used in calls to other TIGL functions.
*
* @return
*   - TIGL_SUCCESS if successfully opened the CPACS configuration
*   - TIGL_XML_ERROR if file is not well-formed or another XML error occurred
*   - TIGL_OPEN_FAILED if some other error occurred
*   - TIGL_NULL_POINTER if cpacsHandlePtr is an invalid null pointer
*   - TIGL_ERROR if some other kind of error occurred
*   - TIGL_INVALID_UID is the UID does not exist or an error orrcured with this configuration
*/
DLL_EXPORT TiglReturnCode tiglOpenCPACSConfiguration(TixiDocumentHandle tixiHandle, char* configurationUID, TiglCPACSConfigurationHandle* cpacsHandlePtr);

/**
* @brief Closes a CPACS configuration and cleans up all memory used by the configuration.
*        After closing a configuration the associated configuration handle is no longer valid.
*        When the CPACS configuration has been closed, the companion tixi document can also be closed.
*
* <b>Fortran syntax:</b>
*
* tigl_close_cpacs_configuration(integer cpacsHandle, integer returnCode)
*
*
* @param cpacsHandle (in): Handle for the CPACS configuration.
*
* @return
*   - TIGL_SUCCESS if successfully opened the CPACS configuration file
*   - TIGL_CLOSE_FAILED if closing of the CPACS configuration failed
*   - TIGL_NOT_FOUND if handle ist not found in handle container
*   - TIGL_ERROR if some other kind of error occurred
*/
DLL_EXPORT TiglReturnCode tiglCloseCPACSConfiguration(TiglCPACSConfigurationHandle cpacsHandle);

/**
* @brief Returns the underlying TixiDocumentHandle for a given CPACS configuration handle.
*
*
* <b>Fortran syntax:</b>
*
* tigl_get_cpacs_tixi_handle(integer cpacsHandle, integer tixiHandlePtr, integer returnCode)
*
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param tixiHandlePtr (out): Handle for the TIXI document associated with the CPACS configuration
*
* @return
*   - TIGL_SUCCESS if the TIXI handle was found
*   - TIGL_NOT_FOUND if the TIXI handle was not found
*   - TIGL_NULL_POINTER if tixiHandlePtr is an invalid null pointer
*   - TIGL_ERROR if some other kind of error occurred
*/
DLL_EXPORT TiglReturnCode tiglGetCPACSTixiHandle(TiglCPACSConfigurationHandle cpacsHandle, TixiDocumentHandle* tixiHandlePtr);

/**
* @brief Checks if a given CPACS configuration handle is a valid.
*
*
* <b>Fortran syntax:</b>
*
* tigl_is_cpacs_configuration_handle_valid(integer cpacsHandle, integer isValidPtr, integer returnCode)
*
*
* @param cpacsHandle (in) : CPACS configuration handle to check
* @param isValidPtr  (out): Contains TIGL_TRUE, if handle is valid, otherwise contains TIGL_FALSE
*
* @return
*   - TIGL_SUCCESS if no error occurred
*/
DLL_EXPORT TiglReturnCode tiglIsCPACSConfigurationHandleValid(TiglCPACSConfigurationHandle cpacsHandle, TiglBoolean* isValidPtr);


/**
* @brief Sets the algorithm for a calculation, i.e. instructs TIGL to use linear
*        or BSpline interpolations between the points of a wing or fuselage profile.
*
*
* <b>Fortran syntax:</b>
*
* tigl_use_algorithm(integer anAlgorithm, integer returnCode)
* #PY:# no outputs
*
* @param anAlgorithm (in): Specifies the algorithm to use, see TiglAlgorithmCode
*
* @return
*   - TIGL_SUCCESS if algorithm was set
*   - TIGL_ERROR   if algorithm code is unknown
*/
DLL_EXPORT TiglReturnCode tiglUseAlgorithm(TiglAlgorithmCode anAlgorithm);


/**
	@brief Returns the version number of this TIGL version.

	<b>Fortran syntax:</b>
	tigl_get_version( character version )

	@return
		- char* A string with the version number.
*/
DLL_EXPORT char* tiglGetVersion();


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
* <b>Fortran syntax:</b>
*
* tigl_get_wing_count(integer cpacsHandle, integer wingCountPtr, integer returnCode)
*
*
* @param cpacsHandle  (in) : Handle for the CPACS configuration
* @param wingCountPtr (out): Pointer to the number of wings
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if wingCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglGetWingCount(TiglCPACSConfigurationHandle cpacsHandle,
                                           int* wingCountPtr);

/**
* @brief Returns the number of segments for a wing in a CPACS configuration.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_segment_count(integer cpacsHandle, integer wingIndex, integer segmentCountPtr, integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of the wing, starting at 1
* @param segmentCountPtr (out): Pointer to the number of segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                  int wingIndex,
                                                  int* segmentCountPtr);

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
* <b>Fortran syntax:</b>
*
* tigl_wing_get_upper_point(integer cpacsHandle,
*                           integer wingIndex,
*                           integer segmentIndex,
*                           real eta,
*                           real xsi,
*                           real pointXPtr,
*                           real pointYPtr,
*                           real pointZPtr,
*                           integer returnCode)
* #PY:4,5,6#
*
* @param cpacsHandle  (in) : Handle for the CPACS configuration
* @param wingIndex    (in) : The index of the wing, starting at 1
* @param segmentIndex (in) : The index of the segment of the wing, starting at 1
* @param eta          (in) : eta in the range 0.0 <= eta <= 1.0; eta = 0 for inner section , eta = 1 for outer section
* @param xsi          (in) : xsi in the range 0.0 <= xsi <= 1.0; xsi = 0 for Leading Edge, xsi =  1 for Trailing Edge
* @param pointXPtr    (out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr    (out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr    (out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetUpperPoint(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_wing_get_lower_point(integer cpacsHandle,
*                           integer wingIndex,
*                           integer segmentIndex,
*                           real eta,
*                           real xsi,
*                           real pointXPtr,
*                           real pointYPtr,
*                           real pointZPtr,
*                           integer returnCode)
* #PY:4,5,6#
*
* @param cpacsHandle  (in) : Handle for the CPACS configuration
* @param wingIndex    (in) : The index of the wing, starting at 1
* @param segmentIndex (in) : The index of the segment of the wing, starting at 1
* @param eta          (in) : eta in the range 0.0 <= eta <= 1.0
* @param xsi          (in) : xsi in the range 0.0 <= xsi <= 1.0
* @param pointXPtr    (out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr    (out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr    (out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occured
*/
DLL_EXPORT TiglReturnCode tiglWingGetLowerPoint(TiglCPACSConfigurationHandle cpacsHandle,
                                                int wingIndex,
                                                int segmentIndex,
                                                double eta,
                                                double xsi,
                                                double* pointXPtr,
                                                double* pointYPtr,
                                                double* pointZPtr);

/**
* @brief Returns the count of wing segments connected to the inner section of a given segment.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_inner_connected_segment_count(integer cpacsHandle,
*                                             integer wingIndex,
*                                             integer segmentIndex,
*                                             integer segmentCountPtr,
*                                             integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param segmentCountPtr (out): Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int* segmentCountPtr);

/**
* @brief Returns the count of wing segments connected to the outer section of a given segment.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_outer_connected_segment_count(integer cpacsHandle,
*                                             integer wingIndex,
*                                             integer segmentIndex,
*                                             integer segmentCountPtr,
*                                             integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param segmentCountPtr (out): Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int* segmentCountPtr);

/**
* @brief Returns the index (number) of the n-th wing segment connected to the inner section
*        of a given segment. n starts at 1.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_inner_connected_segment_index(integer cpacsHandle,
*                                             integer wingIndex,
*                                             integer segmentIndex,
*                                             integer n,
*                                             integer connectedIndexPtr,
*                                             integer returnCode)
*
*
* @param cpacsHandle       (in) : Handle for the CPACS configuration
* @param wingIndex         (in) : The index of a wing, starting at 1
* @param segmentIndex      (in) : The index of a segment, starting at 1
* @param n                 (in) : n-th segment searched, 1 <= n <= tiglWingGetInnerConnectedSegmentCount(...)
* @param connectedIndexPtr (out): Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if wingIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occured
*/
DLL_EXPORT TiglReturnCode tiglWingGetInnerConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int n,
                                                                int* connectedIndexPtr);


/**
* @brief Returns the index (number) of the n-th wing segment connected to the outer section
*        of a given segment. n starts at 1.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_outer_connected_segment_index(integer cpacsHandle,
*                                             integer wingIndex,
*                                             integer segmentIndex,
*                                             integer n,
*                                             integer connectedIndexPtr,
*                                             integer returnCode)
*
*
* @param cpacsHandle       (in) : Handle for the CPACS configuration
* @param wingIndex         (in) : The index of a wing, starting at 1
* @param segmentIndex      (in) : The index of a segment, starting at 1
* @param n                 (in) : n-th segment searched, 1 <= n <= tiglWingGetOuterConnectedSegmentCount(...)
* @param connectedIndexPtr (out): Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if wingIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetOuterConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                int wingIndex,
                                                                int segmentIndex,
                                                                int n,
                                                                int* connectedIndexPtr);


/**
* @brief Returns the section index and section element index of the inner side of a given wing segment.
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_inner_section_and_element_index(integer cpacsHandle,
*                                               integer wingIndex,
*                                               integer segmentIndex,
*                                               integer sectionUIDPtr,
*                                               integer elementUIDPtr,
*                                               integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionIndexPtr (out): The section index of the inner side
* @param elementIndexPtr (out): The section element index of the inner side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 int wingIndex,
                                                                 int segmentIndex,
                                                                 int* sectionIndexPtr,
                                                                 int* elementIndexPtr);

/**
* @brief Returns the section index and section element index of the outer side of a given wing segment.
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_outer_section_and_element_uid(integer cpacsHandle,
*                                               integer wingIndex,
*                                               integer segmentIndex,
*                                               integer sectionIndexPtr,
*                                               integer elementIndexPtr,
*                                               integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionIndexPtr (out): The section index of the outer side
* @param elementIndexPtr (out): The section element index of the outer side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 int wingIndex,
                                                                 int segmentIndex,
                                                                 int* sectionIndexPtr,
                                                                 int* elementIndexPtr);


/**
* @brief Returns the section UID and section element UID of the inner side of a given wing segment.
*        The memory necessary for the two UIDs is allocated by this function and has to be freed
*		 by the user.
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_inner_section_and_element_UID(integer cpacsHandle,
*                                               integer wingIndex,
*                                               integer segmentIndex,
*                                               character sectionUIDPtr,
*                                               character elementUIDPtr,
*                                               integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionUIDPtr   (out): The section UID of the inner side
* @param elementUIDPtr   (out): The section element UID of the inner side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetInnerSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 int wingIndex,
                                                                 int segmentIndex,
                                                                 char** sectionUIDPtr,
                                                                 char** elementUIDPtr);

/**
* @brief Returns the section UID and section element UID of the outer side of a given wing segment.
*        The memory necessary for the two UIDs is allocated by this function and has to be freed
*		 by the user.
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_outer_section_and_element_uid(integer cpacsHandle,
*                                               integer wingIndex,
*                                               integer segmentIndex,
*                                               character sectionUIDPtr,
*                                               character elementUIDPtr,
*                                               integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionUIDPtr   (out): The section UID of the outer side
* @param elementUIDPtr   (out): The section element UID of the outer side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetOuterSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                 int wingIndex,
                                                                 int segmentIndex,
                                                                 char** sectionIndexPtr,
                                                                 char** elementIndexPtr);


/**
* @brief Returns the name of a wing profile. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_profile_name(integer cpacsHandle,
*                            integer wingIndex,
*                            integer sectionIndex,
*                            integer elementIndex,
*                            character*n profileNamePtr,
*                            integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param sectionIndex    (in) : The index of a section, starting at 1
* @param elementIndex    (in) : The index of an element on the section
* @param profileNamePtr  (out): The name of the wing profile
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
DLL_EXPORT TiglReturnCode tiglWingGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_wing_get_uid(integer cpacsHandle,
*                            integer wingIndex,
*                            character*n uIDNamePtr,
*                            integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param uidNamePtr      (out): The uid of the wing
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
DLL_EXPORT TiglReturnCode tiglWingGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 char** uidNamePtr);


/**
* @brief Returns the UID of a segment of a wing. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_segment_uid(integer cpacsHandle,
*                            integer wingIndex,
*                            integer segmentIndex,
*                            character*n uIDNamePtr,
*                            integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param profileNamePtr  (out): The uid of the wing
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
DLL_EXPORT TiglReturnCode tiglWingGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int wingIndex,
                                                 int segmentIndex,
                                                 char** uidNamePtr);


/**
* @brief Returns the UID of a section of a wing. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_section_uid(integer cpacsHandle,
*                            integer wingIndex,
*                            integer sectionIndex,
*                            character*n uIDNamePtr,
*                            integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param sectionIndex    (in) : The index of a section, starting at 1
* @param profileNamePtr  (out): The uid of the wing
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
DLL_EXPORT TiglReturnCode tiglWingGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                int wingIndex,
                                                int sectionIndex,
                                                char** uidNamePtr);


/**
* @brief Returns the Symmetry Enum if the wing has symmetry-axis.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_symmetry(integer cpacsHandle, int wingIndex, integer symmetryAxisPtr, integer returnCode)
*
*
* @param cpacsHandle     (in) :  Handle for the CPACS configuration
* @param wingIndex       (in) :  Index of the Wing to export
* @param symmetryAxisPtr (out) : Returning TiglSymmetryAxis enum pointer
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                              TiglSymmetryAxis* symmetryAxisPtr);


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
* <b>Fortran syntax:</b>
*
* tigl_get_fuselage_count(integer cpacsHandle,
*                         integer fuselageCountPtr,
*                         integer returnCode)
*
*
* @param cpacsHandle      (in) : Handle for the CPACS configuration
* @param fuselageCountPtr (out): Pointer to the number of fuselages
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if fuselageCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglGetFuselageCount(TiglCPACSConfigurationHandle cpacsHandle,
                                               int* fuselageCountPtr);

/**
* @brief Returns the number of segments for a fuselage in a CPACS configuration.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_segment_count(integer cpacsHandle,
*                                 integer fuselageIndex,
*                                 integer segmentCountPtr,
*                                 integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of the fuselage, starting at 1
* @param segmentCountPtr (out): Pointer to the number of segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                      int fuselageIndex,
                                                      int* segmentCountPtr);

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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_point(integer cpacsHandle,
*                         integer fuselageIndex,
*                         integer segmentIndex,
*                         real eta,
*                         real zeta,
*                         real pointXPtr,
*                         real pointYPtr,
*                         real pointZPtr,
*                         integer returnCode)
* #PY:4,5,6#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param zeta          (in) : zeta in the range 0.0 <= zeta <= 1.0
* @param pointXPtr     (out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr     (out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr     (out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetPoint(TiglCPACSConfigurationHandle cpacsHandle,
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
* The angle alpha is calculated in degrees.
* The point is returned in absolute world coordinates.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_point_angle(integer cpacsHandle,
*                               integer fuselageIndex,
*                               integer segmentIndex,
*                               real eta,
*                               real alpha,
*                               real pointXPtr,
*                               real pointYPtr,
*                               real pointZPtr,
*                               integer returnCode)
* #PY:4,5,6#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param alpha         (in) : angle alpha in degrees
* @param pointXPtr     (out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr     (out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr     (out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred, for example if there is no point at the given 
				 eta.
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetPointAngle(TiglCPACSConfigurationHandle cpacsHandle,
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
* The angle alpha is calculated in degrees.
* The parameters x_cs and z_cs must be in absolute world coordinates.
* The point is returned in absolute world coordinates.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_point_angle_translated(integer cpacsHandle,
*                                          integer fuselageIndex,
*                                          integer segmentIndex,
*                                          real eta,
*                                          real alpha,
*                                          real x_cs,
*                                          real z_cs
*                                          real pointXPtr,
*                                          real pointYPtr,
*                                          real pointZPtr,
*                                          integer returnCode)
* #PY:6,7,8#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param alpha         (in) : Angle alpha in degrees
* @param y_cs          (in) : Shifts the origin of the angle alpha in y-direction.
* @param z_cs          (in) : Shifts the origin of the angle alpha in z-direction.
* @param pointXPtr     (out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr     (out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr     (out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred, for example if there is no point at the given 
				 eta and the given shifting parameters y_cs and z_cs.
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetPointAngleTranslated(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_point_on_xplane(integer cpacsHandle,
*                                   integer fuselageIndex,
*                                   integer segmentIndex,
*                                   real eta,
*                                   real xpos,
*                                   integer pointIndex,
*                                   real pointXPtr,
*                                   real pointYPtr,
*                                   real pointZPtr,
*                                   integer returnCode)
* #PY:5,6,7#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param xpos          (in) : x position of a cutting plane
* @param pointIndex    (in) : Defines witch point if more than one.
* @param pointXPtr     (out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr     (out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr     (out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetPointOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_point_on_yplane(integer cpacsHandle,
*                                   integer fuselageIndex,
*                                   integer segmentIndex,
*                                   real eta,
*                                   real ypos,
*                                   integer pointIndex,
*                                   real pointXPtr,
*                                   real pointYPtr,
*                                   real pointZPtr,
*                                   integer returnCode)
* #PY:5,6,7#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param ypos          (in) : Y position
* @param pointIndex    (in) : Defines witch point if more than one.
* @param pointXPtr     (out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr     (out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr     (out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetPointOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_num_points_on_xplane(integer cpacsHandle,
*                                        integer fuselageIndex,
*                                        integer segmentIndex,
*                                        real eta,
*                                        real xpos,
*                                        integer numPoints,
*                                        integer returnCode)
*
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param ypos          (in) : X position
* @param numPointsPtr  (out): Pointer to a interger for the number of intersection points
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnXPlane(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_num_points_on_yplane(integer cpacsHandle,
*                                        integer fuselageIndex,
*                                        integer segmentIndex,
*                                        real eta,
*                                        real ypos,
*                                        integer numPoints,
*                                        integer returnCode)
*
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param ypos          (in) : Y position
* @param numPointsPtr  (out): Pointer to a interger for the number of intersection points
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetNumPointsOnYPlane(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_circumference(integer cpacsHandle,
*                                 integer fuselageIndex,
*                                 integer segmentIndex,
*                                 real eta,
*                                 real circumference,
*                                 integer returnCode)
*
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : The index of the fuselage, starting at 1
* @param segmentIndex  (in) : The index of the segment of the fuselage, starting at 1
* @param eta           (in) : eta in the range 0.0 <= eta <= 1.0
* @param circumference (out): The Circumference of the fuselage at the given position
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetCircumference(TiglCPACSConfigurationHandle cpacsHandle,
                                                        int fuselageIndex,
                                                        int segmentIndex,
                                                        double eta,
                                                        double* circumferencePtr);


/**
* @brief Returns the count of segments connected to the start section of a given fuselage segment.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_start_connected_segment_count(integer cpacsHandle,
*                                                 integer fuselageIndex,
*                                                 integer segmentIndex,
*                                                 integer segmentCountPtr,
*                                                 integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param segmentCountPtr (out): Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                    int fuselageIndex,
                                                                    int segmentIndex,
                                                                    int* segmentCountPtr);

/**
* @brief Returns the count of segments connected to the end section of a given fuselage segment.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_end_connected_segment_count(integer cpacsHandle,
*                                               integer fuselageIndex,
*                                               integer segmentIndex,
*                                               integer segmentCountPtr,
*                                               integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param segmentCountPtr (out): Pointer to the count of connected segments
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if segmentCountPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentCount(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  int fuselageIndex,
                                                                  int segmentIndex,
                                                                  int* segmentCountPtr);

/**
* @brief Returns the index (number) of the n-th segment connected to the start section
*        of a given fuselage segment. n starts at 1.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_start_connected_segment_index(integer cpacsHandle,
*                                                 integer fuselageIndex,
*                                                 integer segmentIndex,
*                                                 integer n,
*                                                 integer connectedIndexPtr,
*                                                 integer returnCode)
*
*
* @param cpacsHandle       (in) : Handle for the CPACS configuration
* @param fuselageIndex     (in) : The index of a fuselage, starting at 1
* @param segmentIndex      (in) : The index of a segment, starting at 1
* @param n                 (in) : n-th segment searched, 1 <= n <= tiglFuselageGetStartConnectedSegmentCount(...)
* @param connectedIndexPtr (out): Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if fuselageIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetStartConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                    int fuselageIndex,
                                                                    int segmentIndex,
                                                                    int n,
                                                                    int* connectedIndexPtr);

/**
* @brief Returns the index (number) of the n-th segment connected to the end section
*        of a given fuselage segment. n starts at 1.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_end_connected_segment_index(integer cpacsHandle,
*                                               integer fuselageIndex,
*                                               integer segmentIndex,
*                                               integer n,
*                                               integer connectedIndexPtr,
*                                               integer returnCode)
*
*
* @param cpacsHandle       (in) : Handle for the CPACS configuration
* @param fuselageIndex     (in) : The index of a fuselage, starting at 1
* @param segmentIndex      (in) : The index of a segment, starting at 1
* @param n                 (in) : n-th segment searched, 1 <= n <= tiglFuselageGetEndConnectedSegmentCount(...)
* @param connectedIndexPtr (out): Pointer to the segment index of the n-th connected segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle or no n-th connected segment was found
*   - TIGL_INDEX_ERROR if fuselageIndex, segmentIndex or n are not valid
*   - TIGL_NULL_POINTER if segmentIndexPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetEndConnectedSegmentIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                  int fuselageIndex,
                                                                  int segmentIndex,
                                                                  int n,
                                                                  int* connectedIndexPtr);


/**
* @brief Returns the section UID and section element UID of the start side of a given fuselage segment.
*        The memory necessary for the two UIDs is allocated by this function and has to be freed
*		 by the user.
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_start_section_and_element_uid(integer cpacsHandle,
*                                                   integer fuselageIndex,
*                                                   integer segmentIndex,
*                                                   character sectionUIDPtr,
*                                                   character elementUIDPtr,
*                                                   integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionUIDPtr   (out): The section UID of the start side
* @param elementUIDPtr   (out): The section element UID of the start side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                     int fuselageIndex,
                                                                     int segmentIndex,
                                                                     char** sectionUIDPtr,
                                                                     char** elementUIDPtr);



/**
* @brief Returns the section UID and section element UID of the end side of a given fuselage segment.
*        The memory necessary for the two UIDs is allocated by this function and has to be freed
*		 by the user.
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_end_section_and_element_uid(integer cpacsHandle,
*                                                 integer fuselageIndex,
*                                                 integer segmentIndex,
*                                                 character sectionUIDPtr,
*                                                 character elementUIDPtr,
*                                                 integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionUIDPtr   (out): The section UID the end side
* @param elementUIDPtr   (out): The section element UID of the end side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                                   int fuselageIndex,
                                                                   int segmentIndex,
                                                                   char** sectionUIDPtr,
                                                                   char** elementUIDPtr);





/**
* @brief Returns the section index and section element index of the start side of a given fuselage segment.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_start_section_and_element_index(integer cpacsHandle,
*                                                   integer fuselageIndex,
*                                                   integer segmentIndex,
*                                                   integer sectionIndexPtr,
*                                                   integer elementIndexPtr,
*                                                   integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionIndexPtr (out): The section UID of the start side
* @param elementIndexPtr (out): The section element UID of the start side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetStartSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
                                                                     int fuselageIndex,
                                                                     int segmentIndex,
                                                                     int* sectionIndexPtr,
                                                                     int* elementIndexPtr);

/**
* @brief Returns the section index and section element index of the end side of a given fuselage segment.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_end_section_and_element_index(integer cpacsHandle,
*                                                 integer fuselageIndex,
*                                                 integer segmentIndex,
*                                                 integer sectionIndexPtr,
*                                                 integer elementIndexPtr,
*                                                 integer returnCode)
* #PY:2,3#
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param sectionIndexPtr (out): The section index UID the end side
* @param elementIndexPtr (out): The section element UID of the end side
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex or segmentIndex are not valid
*   - TIGL_NULL_POINTER if sectionIndexPtr or elementIndexPtr are a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetEndSectionAndElementIndex(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_profile_name(integer cpacsHandle,
*                                integer fuselageIndex,
*                                integer sectionIndex,
*                                integer elementIndex,
*                                character*n profileNamePtr,
*                                integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param sectionIndex    (in) : The index of a section, starting at 1
* @param elementIndex    (in) : The index of an element on the section
* @param profileNamePtr  (out): The name of the wing profile
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
DLL_EXPORT TiglReturnCode tiglFuselageGetProfileName(TiglCPACSConfigurationHandle cpacsHandle,
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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_uid(integer cpacsHandle,
*                            integer fuselageIndex,
*                            character*n uIDNamePtr,
*                            integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param uidNamePtr      (out): The uid of the fuselage
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
DLL_EXPORT TiglReturnCode tiglFuselageGetUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                 int fuselageIndex,
                                                 char** uidNamePtr);


/**
* @brief Returns the UID of a segment of a fuselage. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_segment_uid(integer cpacsHandle,
*                                integer fuselageIndex,
*                                integer segmentIndex,
*                                character*n uIDNamePtr,
*                                integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param profileNamePtr  (out): The uid of the fuselage
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
DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                     int fuselageIndex,
                                                     int segmentIndex,
                                                     char** uidNamePtr);


/**
* @brief Returns the UID of a section of a fuselage. The string returned must not be
* deleted by the caller via free(). It will be deleted when the CPACS configuration
* is closed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_section_uid(integer cpacsHandle,
*                               integer fuselageIndex,
*                               integer sectionIndex,
*                               character*n uIDNamePtr,
*                               integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param sectionIndex    (in) : The index of a section, starting at 1
* @param profileNamePtr  (out): The uid of the fuselage
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
DLL_EXPORT TiglReturnCode tiglFuselageGetSectionUID(TiglCPACSConfigurationHandle cpacsHandle,
                                                    int fuselageIndex,
                                                    int sectionIndex,
                                                    char** uidNamePtr);



/**
* @brief Returns the Symmetry Enum if the fuselage has symmetry-axis.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_symmetry(integer cpacsHandle, int fuselageIndex, integer symmetryAxisPtr, integer returnCode)
*
*
* @param cpacsHandle     (in)  : Handle for the CPACS configuration
* @param fuselageIndex   (in)  : Index of the Wing to export
* @param symmetryAxisPtr (out) : Returning TiglSymmetryAxis enum pointer
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetSymmetry(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                  TiglSymmetryAxis* symmetryAxisPtr);


/*****************************************************************************************************/

/*@}*/
/**
  \defgroup BooleanFunctions Functions for boolean calculations
    Function for boolean calculations on wings/fuselages.
 */
/*@{*/

/**
* @brief Returns a point on the intersection line of two geometric components. Often there are more
* 		 that one intersection line, therefore you need to specify the line.
*
* Returns a point on the intersection line between a surface and a wing in dependence
* of parameter eta which range from 0.0 to 1.0.
* The point is returned in absolute world coordinates.
*
*
* <b>Fortran syntax:</b>
*
* TiglReturnCode tigl_component_intersection_point(integer cpacsHandle,
*														  integer fuselageIndex,
*											 			  integer wingIndex,
*														  real eta,
*														  real* pointXPtr,
*														  real* pointYPtr,
*														  real* pointZPtr,
*														  integer returnCode);
* #PY:4,5,6#
*
* @param cpacsHandle 		(in) : Handle for the CPACS configuration
* @param componentUidOne 	(in) : The UID of the first component
* @param componentUidTwo 	(in) : The UID of the second component
* @param lineID     	  	(in) : The index of the intersection wire, get wire number with "tiglComponentIntersectionLineCount"
* @param eta          		(in) : eta in the range 0.0 <= eta <= 1.0
* @param pointXPtr    		(out): Pointer to the x-coordinate of the point in absolute world coordinates
* @param pointYPtr    		(out): Pointer to the y-coordinate of the point in absolute world coordinates
* @param pointZPtr    		(out): Pointer to the z-coordinate of the point in absolute world coordinates
*
* @return
*   - TIGL_SUCCESS if a point was found
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_NULL_POINTER if pointXPtr, pointYPtr or pointZPtr are null pointers
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglComponentIntersectionPoint(TiglCPACSConfigurationHandle cpacsHandle,
														 char*  componentUidOne,
														 char*  componentUidTwo,
													     int lineID,
													     double eta,
													     double* pointXPtr,
													     double* pointYPtr,
													     double* pointZPtr);


/**
* @brief Returns the number if intersection lines of two geometric components.
*
* <b>Fortran syntax:</b>
*
* TiglReturnCode tigl_component_intersection_line_count(integer cpacsHandle,
*														  integer fuselageIndex,
*											 			  integer* wingIndex,
*														  integer returnCode);
*
*
* @param cpacsHandle  	 (in) : Handle for the CPACS configuration
* @param componentUidOne (in) : The UID of the first component
* @param componentUidTwo (in) : The UID of the second component
* @param numWires    	(out) : The number of intersection lines
*
* @return
*   - TIGL_SUCCESS if there are not intersection lines
*   - TIGL_NOT_FOUND if no point was found or the cpacs handle is not valid
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglComponentIntersectionLineCount(TiglCPACSConfigurationHandle cpacsHandle,
															char*  componentUidOne,
															char*  componentUidTwo,
															int*   numWires);


/*@}*/
/*****************************************************************************************************/

/**
  \defgroup Export Export Functions
    Functions for export of wings/fuselages.


VTK-Export: There a various different VTK exports functions in TIGL. All functions starting with 'tiglExportVTK[Fuselage|Wing]...' are exporting
            a special triangulation with no duplicated points into a VTK file formated in XML (file extension .vtp) with some custom
            informations added to the file.

            The custom metadata is added to an own xml node is looks like the following example:
            @verbatim
            <MetaData elements="uID segmentIndex eta xsi isOnTop">
              "rootToInnerkink" 1 3.18702 0.551342 0
              "rootToInnerkink" 1 2.93939 0.581634 0
              "rootToInnerkink" 1 4.15239 0.520915 0
              ...
            </MetaData>
            @endverbatim

            The 'elements' attribute indicates the number and the names of the additional information tags as a whitespace separated list. In
            this example you could see 5 information fields with the name:
            - uID: The UID of the fuselage or wing component segment on which the triangle exists.
            - segmentIndex: The segmentIndex of the fuselage or wing component segment on which the triangle exists. Kind of redundant to the UID.
            - eta/xsi: The parameters in the surface parametrical space of the triangle.
            - isOnTop: Flag that indicates whether the triangle is on the top of the wing or not. Please see the cpacs documentation how "up"
                       is defined for wings.

            Please note that at this time these information are only valid for wings!
 */

/*@{*/


/**
* @brief Exports the geometry of a CPACS configuration to IGES format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_iges(integer cpacsHandle, character*n filenamePtr, integer returnCode)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param filenamePtr (in) : Pointer to an IGES export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportIGES(TiglCPACSConfigurationHandle cpacsHandle,
                                         char* filenamePtr);


/**
* @brief Exports the boolean fused geometry of a CPACS configuration to IGES format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_fused_wing_fuselage_iges(integer cpacsHandle, character*n filenamePtr, integer returnCode)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param filenamePtr (in) : Pointer to an IGES export file name
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportFusedWingFuselageIGES(TiglCPACSConfigurationHandle cpacsHandle,
												          char* filenamePtr);


/**
* @brief Exports the boolean fused geometry of a wing meshed to STL format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_wing_stl(integer cpacsHandle, int wingIndex, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param wingIndex   (in) : Index of the Wing to export
* @param filenamePtr (in) : Pointer to an STL export file name
* @param deflection  (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedWingSTL(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                                  char* filenamePtr, double deflection); 



/**
* @brief Exports the boolean fused geometry of a fuselage meshed to STL format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_fuselage_stl(integer cpacsHandle, int fuselageIndex, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : Index of the Fuselage to export
* @param filenamePtr   (in) : Pointer to an STL export file name
* @param deflection    (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageSTL(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                      char* filenamePtr, double deflection);


/**
* @brief Exports the boolean fused geometry of the whole configuration meshed to STL format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_geometry_stl(integer cpacsHandle, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param filenamePtr (in) : Pointer to an STL export file name
* @param deflection  (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedGeometrySTL(TiglCPACSConfigurationHandle cpacsHandle, char* filenamePtr,
                                                      double deflection);


/**
* @brief Exports the boolean fused geometry of a wing (selected by id) meshed to VTK format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_wing_vtk_by_index(integer cpacsHandle, int wingIndex, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param wingIndex   (in) : Index of the Wing to export
* @param filenamePtr (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection  (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedWingVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int wingIndex,
                                                         const char* filenamePtr, const double deflection);



/**
* @brief Exports the boolean fused geometry of a wing (selected by UID) meshed to VTK format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_wing_vtk_by_uid(integer cpacsHandle, character*n wingUID, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param wingUID     (in) : UID of the Wing to export
* @param filenamePtr (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection  (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if the wing UID does not exists
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedWingVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                       const char* filenamePtr, const double deflection);



/**
* @brief Exports the boolean fused geometry of a fuselage (selected by index) meshed to VTK format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_fuselage_vtk_by_index(integer cpacsHandle, int fuselageIndex, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageIndex (in) : Index of the Fuselage to export
* @param filenamePtr   (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection    (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByIndex(const TiglCPACSConfigurationHandle cpacsHandle, const int fuselageIndex,
                                                             const char* filenamePtr, const double deflection);


/**
* @brief Exports the boolean fused geometry of a fuselage (selected by uid) meshed to VTK format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_fuselage_vtk_by_uid(integer cpacsHandle, character*n fuselageUID, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageUID   (in) : UID of the Fuselage to export
* @param filenamePtr   (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection    (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageUID does not exists
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID,
                                                           const char* filenamePtr, const double deflection);


/**
* @brief Exports the boolean fused geometry of the whole configuration meshed to VTK format.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_geometry_vtk(integer cpacsHandle, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param filenamePtr (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection  (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedGeometryVTK(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr,
                                                      const double deflection);





/**
* @brief Exports the boolean fused geometry of a wing (selected by UID) meshed to VTK format.
*
* This function does only a very simple, but also very fast meshing on the wing segments and exports them to
* a VTK file. No additional CPACS relevant information are computed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_wing_vtk_simple_by_uid(integer cpacsHandle, character*n wingUID, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param wingUID     (in) : UID of the Wing to export
* @param filenamePtr (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection  (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if the wing UID does not exists
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedWingVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* wingUID,
                                                             const char* filenamePtr, const double deflection);






/**
* @brief Exports the boolean fused geometry of a fuselage (selected by uid) meshed to VTK format.
*
* This function does only a very simple, but also very fast meshing on the fuselage and exports them to
* a VTK file. No additional CPACS relevant information are computed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_fuselage_vtk_simple_by_uid(integer cpacsHandle, character*n fuselageUID, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle   (in) : Handle for the CPACS configuration
* @param fuselageUID   (in) : UID of the Fuselage to export
* @param filenamePtr   (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection    (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if fuselageUID does not exists
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedFuselageVTKSimpleByUID(const TiglCPACSConfigurationHandle cpacsHandle, const char* fuselageUID,
                                                                 const char* filenamePtr, const double deflection);





/**
* @brief Exports the boolean fused geometry of the whole configuration meshed to VTK format.
*
* This function does only a very simple, but also very fast meshing on the geometry and exports them to
* a VTK file. No additional CPACS relevant information are computed.
*
*
* <b>Fortran syntax:</b>
*
* tigl_export_meshed_geometry_vtk_simple(integer cpacsHandle, character*n filenamePtr, integer returnCode, real deflection)
* #PY:#
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param filenamePtr (in) : Pointer to an VTK export file name (*.vtp = polygonal XML_VTK)
* @param deflection  (in) : Maximum deflection of the triangulation from the real surface
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglExportMeshedGeometryVTKSimple(const TiglCPACSConfigurationHandle cpacsHandle, const char* filenamePtr,
                                                            const double deflection);

/*@}*/
/*****************************************************************************************************/

/**
  \defgroup Material Material functions
    Functions to query material information of wings/fuselages.
 */
/*@{*/


/**
* @brief Returns the material UID of a given point on the surface.
*
*
* <b>Fortran syntax:</b>
*
* tigl_get_material_uid(integer cpacsHandle, character*n segmentUID, real eta, real xsi, character*n uidMaterialPtr)
*
*
* @param cpacsHandle (in) : Handle for the CPACS configuration
* @param segmentUID  (in) : UID of the segment
* @param eta         (in) : eta in the range 0.0 <= eta <= 1.0
* @param xsi         (in) : xsi in the range 0.0 <= xsi <= 1.0
* @param uidMaterialPtr (out): Pointer to the material uid at the given coordinate
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if filenamePtr is a null pointer
*   - TIGL_INDEX_ERROR if segmentUID is invalid
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglGetMaterialUID(const TiglCPACSConfigurationHandle cpacsHandle, char* segmentUID,
                                             double eta, double xsi, char** uidMaterialPtr);




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
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_volume(integer cpacsHandle, int fuselageIndex, real volumePtr, integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : Index of the fuselage to calculate the volume, starting at 1
* @param volumenPtr      (out): The volume of the fuselage
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
                                                double *volumePtr);



/**
* @brief Returns the volume of the wing.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_volume(integer cpacsHandle, int wingIndex, real volumePtr, integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : Index of the Wing to calculate the volume, starting at 1
* @param volumenPtr      (out): The volume of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetVolume(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
                                            double *volumePtr);



/**
* @brief Returns the volume of a segment of a wing.
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_segment_volume(integer cpacsHandle,
*                              integer wingIndex,
*                              integer segmentIndex,
*                              real volumePtr,
*                              integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param volumePtr       (out): The pointer to a variable for the volume of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if volumePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                   int wingIndex,
                                                   int segmentIndex,
                                                   double* volumePtr);


/**
* @brief Returns the volume of a segment of a fuselage.
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_segment_volume(integer cpacsHandle,
*                                  integer fuselageIndex,
*                                  integer segmentIndex,
*                                  real volumePtr,
*                                  integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param volumePtr       (out): The pointer to a variable for the volume of the fuselage
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if volumePtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentVolume(TiglCPACSConfigurationHandle cpacsHandle,
                                                   int fuselageIndex,
                                                   int segmentIndex,
                                                   double* volumePtr);



/*@}*/
/*****************************************************************************************************/

/**
  \defgroup SurfaceAreaFunctions Functions for surface area calculations
    Function for surface area calculations off wings/fuselages.
 */
/*@{*/

/**
* @brief Returns the surface area of the wing.
*
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_surface_area(integer cpacsHandle, int wingIndex, real surfaceAreaPtr, integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : Index of the Wing to calculate the area, starting at 1
* @param surfaceAreaPtr  (out): The surface area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int wingIndex,
																				double *surfaceAreaPtr);


/**
* @brief Returns the surface area of the fuselage.
*
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_surface_area(integer cpacsHandle, int fuselageIndex, real surfaceAreaPtr, integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : Index of the Fuselage to calculate the area, starting at 1
* @param surfaceAreaPtr  (out): The surface area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex is less or equal zero
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle, int fuselageIndex,
																					double *surfaceAreaPtr);


/**
* @brief Returns the surface area of a segment of a wing.
*
* <b>Fortran syntax:</b>
*
* tigl_wing_get_segment_surface_area(integer cpacsHandle,
*                                    integer wingIndex,
*                                    integer segmentIndex,
*                                    real surfaceAreaPtr,
*                                    integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param wingIndex       (in) : The index of a wing, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param surfaceAreaPtr  (out): The pointer to a variable for the surface area of the wing
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if wingIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglWingGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
                                                   int wingIndex,
                                                   int segmentIndex,
                                                   double* surfaceAreaPtr);


/**
* @brief Returns the surface area of a segment of a fuselage.
*
* <b>Fortran syntax:</b>
*
* tigl_fuselage_get_segment_surface_area(integer cpacsHandle,
*                                        integer fuselageIndex,
*                                        integer segmentIndex,
*                                        real surfaceAreaPtr,
*                                        integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param fuselageIndex   (in) : The index of a fuselage, starting at 1
* @param segmentIndex    (in) : The index of a segment, starting at 1
* @param surfaceAreaPtr  (out): The pointer to a variable for the surface area of the fuselage-segment
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_INDEX_ERROR if fuselageIndex, sectionIndex or elementIndex are not valid
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglFuselageGetSegmentSurfaceArea(TiglCPACSConfigurationHandle cpacsHandle,
															int fuselageIndex,
															int segmentIndex,
															double* surfaceAreaPtr);


/*@}*/
/*****************************************************************************************************/
/**
  \defgroup ComponentUtilityFunctions Generic utility functions.
    Generic utility functions for geometric components that fits not only to wings _or_ fuselages.
 */
/*@{*/


/**
* @brief Returns a unique HashCode for a geometric components. The component, for example a wing or a fuselage,
* 		 could be specified via its UID. The HashCode is the same as long as the geometie of this component
* 		 has not changed.
* 		 Computes a hash value to represent this shape.
* 		 This value is computed from the value of the underlying shape reference and the
* 		 location. Orientation is not taken into account.
*
* <b>Fortran syntax:</b>
*
* tigl_component_get_hash_code(integer cpacsHandle,
*                              character componentUID,
*                              integer hashCodePtr,
*                              integer returnCode)
*
*
* @param cpacsHandle     (in) : Handle for the CPACS configuration
* @param componentUID    (in) : The uid of the component for which the hash should be computed
* @param hashCodePtr  	 (out): The pointer to a hash value to represent this shape
*
* @return
*   - TIGL_SUCCESS if no error occurred
*   - TIGL_NOT_FOUND if no configuration was found for the given handle
*   - TIGL_NULL_POINTER if surfaceAreaPtr is a null pointer
*   - TIGL_ERROR if some other error occurred
*/
DLL_EXPORT TiglReturnCode tiglComponentGetHashCode(TiglCPACSConfigurationHandle cpacsHandle,
												   char* componentUID,
												   int* hashCodePtr);

#endif /* TIGL_H */

#ifdef __cplusplus
}
#endif
