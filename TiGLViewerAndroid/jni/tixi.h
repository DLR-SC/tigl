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
#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32)
  #if defined (TIXI_EXPORTS)
    #define DLL_EXPORT __declspec (dllexport)
  #else
    #define DLL_EXPORT 
  #endif
#else
    #define DLL_EXPORT
#endif

/* \mainpage TIXI Manual
<b>Contents:</b>
  - \link Glossary Glossary\endlink
  - \link  XPathExamples XPath Examples\endlink
  - \link UsageExamples Usage\endlink
  - <a HREF="modules.html"  class="el">Function Documentation</a>
 */
/*
 @file   tixi.h
 @author Hans-Peter Kersken <Hans-Peter.Kersken@dlr.de>
		 Markus Litz <Markus.Litz@dlr.de>,
         Markus Kunde <Markus.Kunde@dlr.de>,
         Arne Bachmann <Arne.Bachmann@dlr.de>,
 @date   Tue Feb 02 14:28:05 2009

 @brief  Definition of  <b>TIXI</b> the <b>TI</b>VA - <b>XML</b> - <b>I</b>nterface
*/
/*! \page Glossary Glossary

 XML-file: file containing an XML-document

 XML-document: abstraction which either refers to the contents of an
 XML-file or to the data structure held in memory.
*/
/*! \page XPathExamples XPath Examples

 The string to describe a path to an element has to be an XPath
 expression but only a very restricted subset of XPath expressions
 are allowed: On retrieving information the specified path has to be
 unique. If an element name appears more than once inside the same
 parent element (see wings/wing in the example) the expression has
 to be contain an index to denominate exactly which element has to
 be used. As long as these restrictions are met any syntactically
 correct XPath expression is allowed. When adding an element the
 complete path has to be specified.

 Example:
 @verbatim
 <?xml version="1.0" encoding="UTF-8"?>
 <plane>
     <name>Junkers JU 52</name>
     <wings>
         <wing position='left'>
             <centerOfGravity referenceSystem='relative'>
                 <x unit="m">30.0</x>
                 <y unit="m">10.0</y>
                 <z unit="m">5.0</z>
             </centerOfGravity>
         </wing>
         <wing position='right'>
             <centerOfGravity referenceSystem='relative'>
                 <x unit="m">30.0</x>
                 <y unit="m">-10.0</y>
                 <z unit="m">5.0</z>
             </centerOfGravity>
         </wing>
     </wings>
     <coordinateOrigin>
         <x unit="m">0.0</x>
         <y unit="m">0.0</y>
         <z unit="m">0.0</z>
     </coordinateOrigin>
 </plane>
@endverbatim

 The path to the x coordinate value of the center of gravity of the
 plane is:

 /plane/centerOfGravity/x

 The path to the x coordinate value of the center of gravity of the
 first wing element is:

 /plane/wings/wing[1]/centerOfGravity/x

 The path to the name can either by expressed as

 /plane/name

 or

 //name

 which would select all elements called name but as long as this element name is
 unique in the document it is valid in the context of TIXI.

 None valid expressions are:

 /plane/wings/wing/centerOfGravity/z is not unique because there are two wing elements in wings.

 /plane/coordinateOrigin specifies an element with children containing text.

 //x is not unique because it would select all elements named x as there are:
  -  /plane/wings/wing[1]/centerOfGravity/x
  -  /plane/wings/wing[2]/centerOfGravity/x
  -  /plane/centerOfGravity/x
*/
/**
 @page UsageExamples Usage

 The following exmaple assume the XML example from section \ref XPathExamples to be stored in a file named ju.xml.

 - Retrieve the position of the first wing.

@verbatim
 #include "tixi.h"

 char* xmlFilename = "ju.xml";
 TixiDocumentHandle handle = -1;
 char * elementPath = "/plane/wings/wing[1]";
 char* attributeName = "position";

 char* attributeValue;

 tixiOpenDocument( xmlFilename, &handle );
 tixiGetTextAttribute( handle, elementPath, attributeName, &attributeValue );
 tixiCloseDocument( handle );
@endverbatim

 - Retrieve x value of the coordinate origin.

@verbatim
 #include "tixi.h"

 char* xmlFilename = "ju.xml";
 TixiDocumentHandle handle = -1;
 char * elementPath = "/plane/coordinateOrigin/x";
 double x = 0.;

 tixiOpenDocument( xmlFilename, &handle );
 tixiGetDoubleElement( handle, elementPath, &x );
 tixiCloseDocument( handle );
@endverbatim
*/
/**
 \page Fortran Notes for Fortran programming

  The Fortran interface is implemented by calls to subroutines.

  It assumes the follwing mapping for the basic types:

  real is real*8 corresponds double

  integer is integer*4 corresponds to int

  character corresponds char

  Character strings are to be passed as variables of type
  character*N. If a string is returned by a subroutine call the
  variable holding the result must be large enough to hold the result.
  Otherwise the result is truncated and the return code is set to
  STRING_TRUNCATED.

  NOTE: In view of these restrictions an implementation using
  character arrays should be considered.

  The return codes returned in the last argument corresponds to their
  position in ::ReturnCode starting with 0. A routine will be supplied
  to directly access the meaning of a return code as a string.

  When the C interface requires to pass a NULL-pointer, e.g. to choose
  the default format "%g" when writing floating point elements, the
  respective argument in the Fortran interfaces is the empty string
  constant "". This is the only way to represent a string of length
  zero. Passing a variable with all characters set to "" will, via the
  interface transformed into an emtpy C-string "\0", which is of
  length 1, and not to a NULL-pointer.

*/


/**
 @internal
 @section ImplementationIssuues Implementation issues

  - Memory allocated by TIXI and associated with a document is released
   when closing the document.

  - TixiDocumentHandle is an integer used as index to access an TIXI
   internal data structure.

 @todo Add for final version:
 @todo - addNamespace
 @todo - check for attribute/elemet name not NULL in arguments passed to respective routines
 @todo - Fortran90, FORTRAN77 interface testing and return code checking

 @todo - when adding elements, created necessary parent elements which do not already exist too.
*/

#ifndef TIXI_H
#define TIXI_H

/**
   Datatype for TixiDocumentHandle.
*/
typedef int TixiDocumentHandle;


/**
  \defgroup Enums Enumerations
*/

/**
    \ingroup Enums
    ReturnCode return code of TIXI-routines.
     Has a typedef to ReturnCode.
*/
  enum ReturnCode
  {
    SUCCESS,                    /*!< 0: No error occurred                         */

    FAILED,                     /*!< 1: Unspecified error                         */

    INVALID_XML_NAME,           /*!< 2: Non XML standard compliant name specified */

    NOT_WELL_FORMED,            /*!< 3: Document is not well formed               */

    NOT_SCHEMA_COMPLIANT,       /*!< 4: Document is not schema compliant          */

    NOT_DTD_COMPLIANT,          /*!< 5: Document is not DTD compliant             */

    INVALID_HANDLE,             /*!< 6: Document handle is not valid              */

    INVALID_XPATH,              /*!< 7: XPath expression is not valid             */

    ELEMENT_NOT_FOUND,          /*!< 8: Element does not exist in document        */

    INDEX_OUT_OF_RANGE,         /*!< 9: Index supplied as argument is not
                                   inside the admissible range               */

    NO_POINT_FOUND,             /*!< 10: No point element found a given XPath      */

    NOT_AN_ELEMENT,             /*!< 11: XPath expression does not point to an
                                   XML-element node                          */

    ATTRIBUTE_NOT_FOUND,        /*!< 12: Element does not have the attribute       */

    OPEN_FAILED,                /*!< 13: Error on opening the file                 */

    OPEN_SCHEMA_FAILED,         /*!< 14: Error on opening the schema file          */

    OPEN_DTD_FAILED,            /*!< 15: Error on opening the DTD file             */

    CLOSE_FAILED,               /*!< 16: Error on closing the file                 */

    ALREADY_SAVED,              /*!< 17: Trying to modify already saved document   */

    ELEMENT_PATH_NOT_UNIQUE,    /*!< 18: Path expression can not be resolved
                                   unambiguously                             */

    NO_ELEMENT_NAME,            /*!< 19: Element name argument is NULL             */

    NO_CHILDREN,                /*!< 20: Node has no children                      */

    CHILD_NOT_FOUND,            /*!< 21: Named child is not child of element
                                   specified                                 */

    EROROR_CREATE_ROOT_NODE,    /*!< 22: Error when adding root node to new
                                   document                                  */

    DEALLOCATION_FAILED,        /*!< 23: On closing a document the
                                   deallocation of allocated memory fails    */

    NO_NUMBER,                  /*!< 24: No number specified                       */

    NO_ATTRIBUTE_NAME,          /*!< 25: No attribute name specified               */

    STRING_TRUNCATED,           /*!< 26: String variable supplied is to small to
                                   hold the result, Fortran only             */

    NON_MATCHING_NAME,          /*!< 27: Row or column name specified do not
                                   match the names used in the document      */

    NON_MATCHING_SIZE,          /*!< 28: Number of rows or columns specified do
                                   not match the sizes of the matrix in the
                                   document                                  */

    MATRIX_DIMENSION_ERROR,     /*!< 29: if nRows or nColumns or both are
                                   less than 1 */

    COORDINATE_NOT_FOUND,       /*!< 30: missing coordinate inside a point element */

    UNKNOWN_STORAGE_MODE,        /*!< 31: storage mode specified is neither
                                   ROW_WISE nor COLUMN_WISE  */

    UID_NOT_UNIQUE,       		 /*!< 32: One or more uID's are not unique */

    UID_DONT_EXISTS,       		 /*!< 33: A given uID's does not exist */

    UID_LINK_BROKEN       		 /*!< 33: A node the is specified as a Link has no correspoding uid in that data set */

  };

  typedef enum ReturnCode ReturnCode;

 /**

 \ingroup Enums
      Strorage mode of arrays This enum indicates how a matrix is stored
      in an array on input or should be retrieved into an array on
      output. If ROW_WISE is specified the order will be (1,1), (1,2),
      ..., (2,1), (2,2), (2,3)... If COLUMN_WISE is specified the order
      will be (1,1), (2,1), (3,1), .., (1,2),(2,2),(3,2) ...

  Has a typedef to StorageMode.
   */
  enum StorageMode
  {
    ROW_WISE,                   /*!< row wise order                             */
    COLUMN_WISE                 /*!< column wise                                */
  };


  typedef enum StorageMode StorageMode;


 /**

 \ingroup Enums
      Open mode of a xml file
      This enum indicates how a xml file is opend.
      If OpenMode is OPENMODE_PLAIN, the xml file is open "normal" and just the given
      file is opend. If OpenMode is OPENMODE_RECURSIVE, then all external files
      specified in a <externaldata> node are opend and replaced in the xml tree.

  Has a typedef to OpenMode.
   */
  enum OpenMode
  {
    OPENMODE_PLAIN,                   /*!< Open just the xml file        */
    OPENMODE_RECURSIVE                 /*!< Open with external files     */
  };


  typedef enum OpenMode OpenMode;


/**
	@brief Returns the version number of this TIXI version.

	<b>Fortran syntax:</b>
	tixi_get_version( character version )

	@return
		- char* A string with the version number.
*/
  DLL_EXPORT char* tixiGetVersion();



/**
  \defgroup FileHandling File Handling Functions
    Function to open, create, close, and save XML-files.
 */
/*@{*/
/**
    @brief Open an XML-file for reading.

    Opens an XML-file specified by xmlFilename for reading and checks
    if it is well formed. To validate the document against a XML-Schema
    or DTD use ::tixiSchemaValidateFromFile, ::tixiSchemaValidateFromString or ::tixiDTDValidate.


    <b>Fortran syntax:</b>

    tixi_open_document( character*n xml_filename, integer handle, integer error )

    @param xmlFilename (in) name of the XML-file to be opened
    @param handle (out) handle to the XML-document. This handle is used in
                  calls to other TIXI functions.
    @return

     - SUCCESS if successfully opened the XML-file
     - NOT_WELL_FORMED if opening the XML-file succeeds but test for
                            well-formedness fails
     - OPEN_FAILED if opening of the XML-file failed
 */

  DLL_EXPORT ReturnCode tixiOpenDocument (const char *xmlFilename, TixiDocumentHandle * handle);



  /**
      @brief Open an XML-file for reading. It acts like tixiOpenDocument.

      If OpenMode is OPENMODE_PLAIN, only the given xml is opened. If OpenMode is
    OPENMODE_RECURSIVE, external xml files will be integrated into the xml tree
    by linking them into the main xml file. The user now has only one big xml file.
	In the path node has be be a valid URI. This uri could adress a relativ or absolut file path,
	or a http url. Example values for the path node are:
	- absolute local directory: "file:///tmp/" or "file:///c:/windws/"
	- relative local directory: "file://relativeDirectory/" or "file://../anotherRelativeDirectory/"
	- remote http ressource: "http://www.someurl.de/"

	Examples for the externaldata node:
    @code{.xml}
    <wings>
        <airfoils>
                <externaldata>
                    <path>file://aDirectory/</path>
                    <filename>VFW614-W-1.xml</filename>
                    <filename>VFW614-W-2.xml</filename>
                    ...
                </externaldata>
            <airfoil>
                <name>VFW614 Seitenleitwerksprofil</name>
                <coordinates>
                    <point><x>1.0000000</x><y>0.0000000</y><z>0.0000000</z></point>
                    <point><x>0.9795687</x><y>0.0023701</y><z>0.0000000</z></point>
                    ...
                </coordinates>
            </airfoil>
        </airfoils>
    </wings>
    @endcode

    <b>Fortran syntax:</b>

    tixi_open_document_recursive( character*n xml_filename, integer handle, integer openmode, integer error )

    @param xmlFilename (in) name of the XML-file to be opened
    @param handle (out) handle to the XML-document. This handle is used in
                  calls to other TIXI functions.
    @param oMode (in) Enum of the mode to open (OPENMODE_PLAIN / OPENMODE_RECURSIVE).
    @return

     - SUCCESS if successfully opened the XML-file
     - NOT_WELL_FORMED if opening the XML-file succeeds but test for
                            well-formedness fails
     - OPEN_FAILED if opening of the XML-file failed
 */
  DLL_EXPORT ReturnCode tixiOpenDocumentRecursive (const char *xmlFilename, TixiDocumentHandle * handle, OpenMode oMode);


  /**
      @brief Open an XML-file for reading from a http web resource.

      Opens an XML-file specified by httpURL for reading and checks
      if it is well formed. To validate the document against a XML-Schema
      or DTD use ::tixiSchemaValidateFromFile, ::tixiSchemaValidateFromString or ::tixiDTDValidate.


      <b>Fortran syntax:</b>

      tixi_open_document_from_http( character*n xml_httpurl, integer handle, integer error )

      @param httpURL (in) url of the XML-file to be opened
      @param handle (out) handle to the XML-document. This handle is used in
                    calls to other TIXI functions.
      @return

       - SUCCESS if successfully opened the XML-file
       - NOT_WELL_FORMED if opening the XML-file succeeds but test for
                              well-formedness fails
       - OPEN_FAILED if opening of the XML-file failed
   */

    DLL_EXPORT ReturnCode tixiOpenDocumentFromHTTP (const char *httpURL, TixiDocumentHandle * handle);

/**

    @brief Create an XML-document.

    Initializes a data structure to hold an XML-document. 

    <b>Fortran syntax:</b>

    tixi_create_document( character*n root_element_name, integer handle, integer error )

    @param rootElementName (in) name of the root element of the XML-document
    @param handle          (out) handle to an XML-document. This handle is
                           used in calls to other TIXI functions.


    @return
     - SUCCESS if data structure is set-up successfully
     - FAILED if data structure could not created
*/

  DLL_EXPORT ReturnCode tixiCreateDocument (const char *rootElementName, TixiDocumentHandle * handle);

/**
    @brief Write XML-document to disk.

    The document is written into a file specified by xmlFilename. The
    user should validate the document before it is written to
    disk. Memory allocated internally for processing this document has to
    be released by ::tixiCloseDocument.

    If the file was opened with OpenMode=OPENMODE_RECURSIVE and external file where linked into
    the main XML tree, please note that these files are saved back into the external files. The external
    nodes will not be removed from the main xml-document.

    <b>Fortran syntax:</b>

    tixi_save_document( integer  handle, character*n xml_filename, integer error )


    @param handle (in) document handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
    @param xmlFilename (in) name of the file to be created.

    @return
    - SUCCESS              if the file is successfully written and validated
    - INVALID_HANDLE if  handle not found in list of man
    - FAILED         if writing and closing the XML-file failed
*/

  DLL_EXPORT ReturnCode tixiSaveDocument (TixiDocumentHandle handle, const char *xmlFilename);


/**
	  @brief Write XML-document with all external data to disk.

	  The document is written into a file specified by xmlFilename. The
	  user should validate the document before it is written to
	  disk. Memory allocated internally for processing this document has to
	  be released by ::tixiCloseDocument.

	  If the file was opened with OpenMode=OPENMODE_RECURSIVE and external file where linked into
	  the main XML document, these additional nodes will be saved in the xml file as well. The external
	  nodes will not be removed from the main xml-document.

	  <b>Fortran syntax:</b>

	  tixi_save_complete_document( integer  handle, character*n xml_filename, integer error )


	  @param handle (in) document handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
	  @param xmlFilename (in) name of the file to be created.

	  @return
	  - SUCCESS              if the file is successfully written and validated
	  - INVALID_HANDLE if  handle not found in list of man
	  - FAILED         if writing and closing the XML-file failed
*/

	DLL_EXPORT ReturnCode tixiSaveCompleteDocument (TixiDocumentHandle handle, const char *xmlFilename);


/**
		  @brief Write XML-document with all external data to disk.

	  The document is written into a file specified by xmlFilename. The
	  user should validate the document before it is written to
	  disk. Memory allocated internally for processing this document has to
	  be released by ::tixiCloseDocument.

	  If the file was opened with OpenMode=OPENMODE_RECURSIVE and external file where linked into
	  the main XML document, these additional nodes will be saved in the xml file as well. The external
	  nodes will be removed from the main xml-document.

	  <b>Fortran syntax:</b>

	  tixi_save_and_remove_document( integer  handle, character*n xml_filename, integer error )


	  @param handle (in) document handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
	  @param xmlFilename (in) name of the file to be created.

	  @return
	  - SUCCESS              if the file is successfully written and validated
	  - INVALID_HANDLE if  handle not found in list of man
	  - FAILED         if writing and closing the XML-file failed
  */

        DLL_EXPORT ReturnCode tixiSaveAndRemoveDocument (TixiDocumentHandle handle, const char *xmlFilename);


/**
    @brief Close an XML-document.

    Closes an XML-document. This routine should be called after the
    processing of an XML-document is completed. After calling this
    routine the handle is invalid and no further processing of the
    document is possible. It must be called after ::tixiSaveDocument.

    <b>Fortran syntax:</b>

    tixi_close_document( integer  handle, integer error )

    @param handle (in) file handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString

    @return
     - SUCCESS if successfully closed the XML-file
     - INVALID_HANDLE if  handle not found in list of man
     - CLOSE_FAILED if closing  the XML-file failed
 */

  DLL_EXPORT ReturnCode tixiCloseDocument (TixiDocumentHandle handle);


/**
    @brief Close all open documents.

      Closes all XML-documents. This routine could be called at the
      of a program to free allocated memory. After calling this
      routine all current handles are invalid and no further processing of the
      document is possible.

      <b>Fortran syntax:</b>

      tixi_close_all_documents( integer error )

      @return
       - SUCCESS if successfully closed the XML-file
       - CLOSE_FAILED if closing  the XML-files failed
 */

  DLL_EXPORT ReturnCode tixiCloseAllDocuments ();

  /**
    @brief Closes the xml2 library and frees its allocated variables
  */
  DLL_EXPORT ReturnCode tixiCleanup ();


  /**
    @brief Returns the Document as one text.

    Returns the text content of the document specified by handle.
    If an error occurs text is set to NULL. On successful return the memory used for text is allocated
    internally and must not be released by the user. The deallocation
    is handle when the document referred to by handle is closed.
    If OpenMode=OPENMODE_RECURSIVE, text will be one big string with all external
    xml files included.

    <b>Fortran syntax:</b>

    tixi_export_document_as_string( integer handle, character*n text, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString

    @param text (out) text content of the document

    @return
     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
 */
  DLL_EXPORT ReturnCode tixiExportDocumentAsString (const TixiDocumentHandle handle, char **text);


  /**
    @brief Imports a char-string into a new tixi-document.

    Creates a new TIXI-document with the content of the string and checks
    if it is well formed. To validate the document against a XML-Schema
    or DTD use ::tixiSchemaValidateFromFile, ::tixiSchemaValidateFromString or ::tixiDTDValidate.

    <b>Fortran syntax:</b>

    tixi_import_from_string( character*n xmlImportString, integer handle, integer error )

    @param xmlImportString (in) the string with the xml-content
    @param handle (out) handle to the XML-document. This handle is used in
                  calls to other TIXI functions.
    @return

     - SUCCESS if successfully imported the string
     - NOT_WELL_FORMED if importing of the string succeeds but test for well-formedness fails
 */

  DLL_EXPORT ReturnCode tixiImportFromString (const char *xmlImportString, TixiDocumentHandle * handle);

/*@}*/
/**
  \defgroup Validation Validation Functions
    Function to validate document with respect to DTD or Schemas.
 */
/*@{*/


/**
    @brief Validate XML-document against an XML-schema.

    Validates an XML-document against an XML-schema specified by
    xsdFilename. This routine should be called after opening a
    document by ::tixiOpenDocument and before ::tixiSaveDocument if the
    validation against an XML-schema is desired for the input file and
    the output file, respectively.

    <b>Fortran syntax:</b>

    tixi_schema_validate_from_file( integer  handle, character*n xsd_filename, integer error )

    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond
    
    @param xsdFilename (in) name of the XML-schema-file to be used.

    @param handle (in) handle to the XML-document.

    @return

     - SUCCESS              if the document is successfully validated
     - NOT_WELL_FORMED      if the XML-document is not well formed
     - NOT_SCHEMA_COMPLIANT if the XML-document is well-formed
                            but validating against the given XML-schema fails
     - OPEN_SCHEMA_FAILED   if opening of the XML-schema-file failed
     - FAILED                  for all internal errors
 */

  DLL_EXPORT ReturnCode tixiSchemaValidateFromFile (const TixiDocumentHandle handle, const char *xsdFilename);

  /**
      @brief Validate XML-document against an XML-schema.

      Validates an XML-document against an XML-schema specified by
      xsdString. This routine should be called after opening a
      document by ::tixiOpenDocument and before ::tixiSaveDocument if the
      validation against an XML-schema is desired for the input file and
      the output file, respectively.
      The complete schema has to be in the string xsdString before running
      this function.

      <b>Fortran syntax:</b>

      tixi_schema_validate_from_string( integer  handle, character*n xsd_string, integer error )
      @cond
      #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
      @endcond

      @param xsdString (in) char array witch is holding a schema.

      @param handle (in) handle to the XML-document.

      @return

       - SUCCESS              if the document is successfully validated
       - NOT_WELL_FORMED      if the XML-document is not well formed
       - NOT_SCHEMA_COMPLIANT if the XML-document is well-formed
                              but validating against the given XML-schema fails
       - OPEN_SCHEMA_FAILED   if opening of the XML-schema-file failed
       - FAILED                  for all internal errors
   */

    DLL_EXPORT ReturnCode tixiSchemaValidateFromString (const TixiDocumentHandle handle, const char *xsdString);


/**
    @brief Validate XML-document against a DTD.

    Validates an XML-document against a DTD specified by
    DTDFilename. This routine is to be called after opening a
    document by ::tixiOpenDocument and before ::tixiSaveDocument if the
    validation against a DTD is desired for the input file and the
    output file, respectively.

    <b>Fortran syntax:</b>

    tixi_dtd_validate( integer  handle, character*n dtd_filename, integer error )

    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param DTDFilename (in) name of the DTD-file to be used

    @param handle (in) handle to the XML-document to be validated.

    @return

     - SUCCESS              if the document is successfully validated
     - NOT_WELL_FORMED      if the XML-document is not well formed
     - NOT_DTD_COMPLIANT    if the XML-document is well-formed but validating
                            against the given DTD fails
     - OPEN_DTD_FAILED      if opening of the DTD-file failed
 */

  DLL_EXPORT ReturnCode tixiDTDValidate (const TixiDocumentHandle handle, const char *DTDFilename);
/*@}*/



/**
  \defgroup Elements Element handling functions
  Functions to get the content of an element as a string or a number, functions
  to create and manipulate its content, and a funtion to remove an element are described
  in this section.
 */

/*@{*/
/**
    @brief Retrieve text content of an element.

    Returns the text content of the element specified by elementPath in the
    document specified by handle. elementPath must refer to exactly one
    element which has only a text node and zero or more attributes but
    no further children with text nodes. If an error occurs text is set
    to NULL. On successful return the memory used for text is allocated
    internally and must not be released by the user. The deallocation
    is handle when the document referred to by handle is closed.

    <b>Fortran syntax:</b>

    tixi_get_text_element( integer handle, character*n element_path,  character*n text, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param text (out) text content of the element specified by elementPath

    @return

     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
 */

  DLL_EXPORT ReturnCode tixiGetTextElement (const TixiDocumentHandle handle,
                                            const char *elementPath, char **text);


/**
    @brief Retrieve integer content of an element.

    Returns the content of the element specified by elementPath in the
    document specified by handle as an integer. elementPath must refer to exactly one
    element which has only a text node and zero or more attributes but
    no further children with text nodes. If an error occurs text is set
    to NULL. On successful return the memory used for text is allocated
    internally and must not be released by the user. The deallocation
    is handle when the document referred to by handle is closed.

    <b>Fortran syntax:</b>

    tixi_get_integer_element( integer  handle, character*n element_path, int* number, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param number (out)  content of the element specified by elementPath interpreted as an integer number

    @return

     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
 */

  DLL_EXPORT ReturnCode tixiGetIntegerElement (const TixiDocumentHandle handle, const char *elementPath, int *number);

 /**
    @brief Retrieve floating point content of an element.

    Returns the content of the element specified by elementPath in the
    document specified by handle as a floating point
    number. elementPath must refer to exactly one element which has
    only a text node and zero or more attributes but no further
    children with text nodes. If an error occurs number is set to
    NULL.

    <b>Fortran syntax:</b>

    tixi_get_double_element( integer  handle, character*n element_path, real number, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param number (out)  content of the element specified by elementPath interpreted as a floating point number

    @return

     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
 */

  DLL_EXPORT ReturnCode tixiGetDoubleElement (const TixiDocumentHandle handle, const char *elementPath, double *number);


  /**
     @brief Retrieve boolean content of an element.

     Returns the content of the element specified by elementPath in the
     document specified by handle as an integer number with the values 0=false and 1=true.
     elementPath must refer to exactly one element which has
     only a text node and zero or more attributes but no further
     children with text nodes. If an error occurs boolean is set to
     NULL. In the XML file, a boolean value could be defined with "1"/"0" or
     (case sensitive) "true"/"false".

     <b>Fortran syntax:</b>

     tixi_get_boolean_element( integer  handle, character*n element_path, integer boolean, integer error )

     @cond
    #PY:2#
     @endcond

     @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

     @param elementPath (in) an XPath compliant path to an element in the document
                       specified by handle (see section \ref XPathExamples above).

     @param boolean (out)  content of the element specified by elementPath interpreted as a integer containing
						   boolean values.

     @return
      - SUCCESS if successfully retrieve the text content of a single element
      - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
      - INVALID_XPATH if elementPath is not a well-formed XPath-expression
      - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
      - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
  */

   DLL_EXPORT ReturnCode tixiGetBooleanElement (const TixiDocumentHandle handle, const char *elementPath, int *boolean);


  /**
      @brief Updates the text content of an element.

      Update the text content of the element specified by elementPath in the
      document specified by handle. elementPath must refer to exactly one
      element which has only a text node and zero or more attributes but
      no further children with text nodes.

      <b>Fortran syntax:</b>

      tixi_update_text_element( integer handle, character*n element_path,  character*n text, integer error )

      @cond
      #PY:# no output
      @endcond

      @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

      @param elementPath (in) an XPath compliant path to an element in the document
                        specified by handle (see section \ref XPathExamples above).

      @param text (in) text content of the element to update the element specified by elementPath

      @return

       - SUCCESS if successfully retrieve the text content of a single element
       - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
       - INVALID_XPATH if elementPath is not a well-formed XPath-expression
       - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
       - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
       - FAILED if node is no textNode
   */
    DLL_EXPORT ReturnCode tixiUpdateTextElement (const TixiDocumentHandle handle, const char *elementPath, const char *text);


    /**
        @brief Updates the double content of an element.

        Update the double content of the element specified by elementPath in the
        document specified by handle. elementPath must refer to exactly one
        element which has only a text node and zero or more attributes but
        no further children with text nodes.

        <b>Fortran syntax:</b>

        tixi_update_double_element( integer handle, character*n element_path,  real number, character*n format, integer error )

        @cond
        #PY:# no output
        @endcond

        @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

        @param elementPath (in) an XPath compliant path to an element in the document
                          specified by handle (see section \ref XPathExamples above).

        @param number (in) double content of the element to update the element specified by elementPath
        @param format (in) format used to convert number into a string. If format is NULL "%g" will be used to format the string.

        @return
         - SUCCESS if successfully retrieve the text content of a single element
         - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
         - INVALID_XPATH if elementPath is not a well-formed XPath-expression
         - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
         - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
         - FAILED if node is no textNode
     */
      DLL_EXPORT ReturnCode tixiUpdateDoubleElement (const TixiDocumentHandle handle, const char *elementPath, double number, const char *format);

      /**
         @brief Updates the double content of an element.

         Update the integer content of the element specified by elementPath in the
         document specified by handle. elementPath must refer to exactly one
         element which has only a text node and zero or more attributes but
         no further children with text nodes.

         <b>Fortran syntax:</b>

         tixi_update_integer_element( integer handle, character*n element_path,  integer number, character*n format, integer error )
         @cond
         #PY:# no output
         @endcond

         @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

         @param elementPath (in) an XPath compliant path to an element in the document
                           specified by handle (see section \ref XPathExamples above).

         @param number (in) integer content of the element to update the element specified by elementPath
         @param format (in) format used to convert number into a string. If format is NULL "%g" will be used to format the string.

         @return
          - SUCCESS if successfully retrieve the text content of a single element
          - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
          - INVALID_XPATH if elementPath is not a well-formed XPath-expression
          - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
          - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
          - FAILED if node is no textNode
      */
       DLL_EXPORT ReturnCode tixiUpdateIntegerElement (const TixiDocumentHandle handle, const char *elementPath, int number, const char *format);

   /**
      @brief Updates the boolean content of an element.

      Update the boolean content of the element specified by elementPath in the
      document specified by handle. elementPath must refer to exactly one
      element which has only a text node and zero or more attributes but
      no further children with text nodes.

      <b>Fortran syntax:</b>

      tixi_update_boolean_element( integer handle, character*n element_path,  integer boolean, character*n format, integer error )
      @cond
      #PY:# no output
      @endcond

      @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP

      @param elementPath (in) an XPath compliant path to an element in the document
                        specified by handle (see section \ref XPathExamples above).

      @param boolean (in) boolean content of the element to update the element specified by elementPath. The value of boolean has to be "0" or "1".

      @return
       - SUCCESS if successfully retrieve the text content of a single element
       - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
       - INVALID_XPATH if elementPath is not a well-formed XPath-expression
       - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
       - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
       - FAILED if node is no textNode
   */
    DLL_EXPORT ReturnCode tixiUpdateBooleanElement (const TixiDocumentHandle handle, const char *elementPath, int boolean);


/**
@brief Creates an element holding text.

Creates an element specified by the elementPath expression and
insert text into the element. Elements with the same name can be
added multiple times.

<b>Fortran syntax:</b>

tixi_add_text_element( integer  handle, character*n parent_path, character*n element_name, character*n text, integer error )
@cond
#PY:# no output params (explicit mark necessary, otherwise assuming last = output)
@endcond

@param handle (in) file handle as returned by ::tixiCreateDocument

@param parentPath (in) an XPath compliant path to an element in the document
                       specified by handle (see section \ref XPathExamples above)
                       into which the new element is to be inserted. The parent
                       element has to exist already.

@param elementName (in) name of the element to be inserted into the parent element

@param text (in) text to be placed inside the element pointed to by elementPath. If
                 text is NULL an empty element will be created.

@return

    - SUCCESS if successfully added the text element
    - INVALID_XML_NAME if elementName is not a valid XML-element name
    - INVALID_HANDLE if the handle is not valid
    - INVALID_XPATH if elementPath is not a well-formed XPath-expression
    - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
    - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
    - ALREADY_SAVED if element should be added to an already saved document
*/
  DLL_EXPORT ReturnCode tixiAddTextElement (const TixiDocumentHandle handle, const char *parentPath, const char *elementName, const char *text);


  /**
  @brief Creates an element holding text at a given index.

  Creates an element specified by the elementPath expression and
  insert text into the element. Elements with the same name can be
  added multiple times, so in this version of the function an index could be
  provides to specifiy a exact position.

  <b>Fortran syntax:</b>

  tixi_add_text_element_at_index( integer  handle, character*n parent_path, character*n element_name, character*n text, integer index, integer error )
  @cond
  #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
  @endcond

  @param handle (in) file handle as returned by ::tixiCreateDocument

  @param parentPath (in) an XPath compliant path to an element in the document
                         specified by handle (see section \ref XPathExamples above)
                         into which the new element is to be inserted. The parent
                         element has to exist already.

  @param elementName (in) name of the element to be inserted into the parent element

  @param text (in) text to be placed inside the element pointed to by elementPath. If
                   text is NULL an empty element will be created.

  @param index (in) the position index where the new node should be created.

  @return

      - SUCCESS if successfully added the text element
      - INVALID_XML_NAME if elementName is not a valid XML-element name
      - INVALID_HANDLE if the handle is not valid
      - INVALID_XPATH if elementPath is not a well-formed XPath-expression
      - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
      - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
      - ALREADY_SAVED if element should be added to an already saved document
  */

    DLL_EXPORT ReturnCode tixiAddTextElementAtIndex (const TixiDocumentHandle handle, const char *parentPath, const char *elementName, const char *text, int index);


  /**

  @brief Creates an element and setting the value to "true" or "false".

  Creates an element specified by the elementPath expression and
  insert one of the boolean values "true" or "false" into the element.
  Elements with the same name can be added multiple times.

  <b>Fortran syntax:</b>

  tixi_add_boolean_element( integer  handle, character*n parent_path, character*n element_name, integer boolean, integer error )
  @cond
  #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
  @endcond

  @param handle (in) file handle as returned by ::tixiCreateDocument

  @param parentPath (in) an XPath compliant path to an element in the document
                         specified by handle (see section \ref XPathExamples above)
                         into which the new element is to be inserted. The parent
                         element has to exist already.

  @param elementName (in) name of the element to be inserted into the parent element

  @param boolean (in) boolean value to be placed inside the element pointed to by elementPath. The
					  value of boolean has to be "0" or "1".

  @return

      - SUCCESS if successfully added the text element
      - INVALID_XML_NAME if elementName is not a valid XML-element name
      - INVALID_HANDLE if the handle is not valid
      - INVALID_XPATH if elementPath is not a well-formed XPath-expression
      - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
      - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
      - ALREADY_SAVED if element should be added to an already saved document
  */

    DLL_EXPORT ReturnCode tixiAddBooleanElement (const TixiDocumentHandle handle, const char *parentPath, const char *elementName, int boolean);

/**

@brief Creates an element which holds a floating point number.

Creates an element specified by the elementPath expression. Elements
with the same name can be added multiple times.

<b>Fortran syntax:</b>

tixi_add_double_element( integer  handle, character*n parent_path, character*n element_name, real number, character*n format, integer error )
@cond
#PY:# no output params (explicit mark necessary, otherwise assuming last = output)
@endcond

@param handle (in) file handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

@param parentPath (in) an XPath compliant path to an element in the document
specified by handle (see section \ref XPathExamples above).

@param elementName (in) name of the element to be inserted into the parent element

@param number (in) floating point number to be placed inside the element pointed to by
elementPath. If number is NULL an empty element will be created.

@param format (in) format used to convert number into a string. If format is
NULL "%g" will be used to format the string.

@return

    - SUCCESS if successfully retrieve the text content
    - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
    - INVALID_XPATH if elementPath is not a well-formed XPath-expression
    - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
    - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements

*/

  DLL_EXPORT ReturnCode tixiAddDoubleElement (const TixiDocumentHandle handle,
                                              const char *parentPath, const char *elementName,
                                              double number, const char *format);

/**

@brief Creates an element which holds an integer.

Creates an element specified by the elementPath expression holding
an integer number. Elements with the same name can be added multiple times.

<b>Fortran syntax:</b>

tixi_add_integer_element( integer  handle, character*n parent_path, character*n element_name, integer number, character*n format, integer error )
@cond
#PY:# no output params (explicit mark necessary, otherwise assuming last = output)
@endcond

@param handle (in) file handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

@param parentPath (in) an XPath compliant path to an element in the document
                    specified by handle (see section \ref XPathExamples above).

@param elementName (in) name of the element to be inserted into the parent element

@param number (in) integer number to be placed inside the element pointed to by
                    elementPath. If number is NULL an empty element will be created.

@param format (in) format used to convert number into a string. If format is
                    NULL "%d" will be used to format the string.

@return

    - SUCCESS if successfully retrieve the text content
    - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
    - INVALID_XPATH if elementPath is not a well-formed XPath-expression
    - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
    - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
*/

  DLL_EXPORT ReturnCode tixiAddIntegerElement (const TixiDocumentHandle handle,
                                               const char *parentPath, const char *elementName,
                                               int number, const char *format);


  /**

  @brief Creates an element holding an vector.

  Creates an element specified by the elementPath expression and
  insert the vector elements into the element. Vector with the same name can be
  added multiple times.


  <b>Fortran syntax:</b>

  tixi_add_float_vector( integer handle, character*n parent_path, character*n element_name, real array, integer numElements, integer error )
  @cond
  #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
  @endcond

  @param handle (in) file handle as returned by ::tixiCreateDocument

  @param parentPath (in) an XPath compliant path to an element in the document
                         specified by handle (see section \ref XPathExamples above)
                         into which the new element is to be inserted. The parent
                         element has to exist already.

  @param elementName (in) name of the element to be inserted into the parent element

  @param vector (in) The Vector to be placed inside the element pointed to by elementPath. If
                      Vector is NULL an empty element will be created.

  @param numElements (in) the Number of vector-elements to be inserted in the new element.

  @return
      - SUCCESS if successfully added the text element
      - FAILED for internal errors
      - INVALID_XML_NAME if elementName is not a valid XML-element name
      - INVALID_HANDLE if the handle is not valid
      - INVALID_XPATH if elementPath is not a well-formed XPath-expression
      - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
      - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
      - ALREADY_SAVED if element should be added to an already saved document
  */

    DLL_EXPORT ReturnCode tixiAddFloatVector (const TixiDocumentHandle handle, const char *parentPath, const char *elementName, double *vector, const int numElements);


    /**
    @brief Creates an empty element.

    Creates an empty element specified by the elementPath expression
    Elements with the same name can be added multiple times.

    <b>Fortran syntax:</b>

    tixi_create_element( integer  handle, character*n parent_path, character*n element_name, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) file handle as returned by ::tixiCreateDocument

    @param parentPath (in) an XPath compliant path to an element in the document
                           specified by handle (see section \ref XPathExamples above)
                           into which the new element is to be inserted. The parent
                           element has to exist already.

    @param elementName (in) name of the element to be inserted into the parent element


    @return
        - SUCCESS if successfully added the text element
        - INVALID_XML_NAME if elementName is not a valid XML-element name
        - INVALID_HANDLE if the handle is not valid
        - INVALID_XPATH if elementPath is not a well-formed XPath-expression
        - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
        - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
        - ALREADY_SAVED if element should be added to an already saved document
    */
      DLL_EXPORT ReturnCode tixiCreateElement (const TixiDocumentHandle handle, const char *parentPath, const char *elementName);


      /**
	  @brief Creates an empty element at a given index.

	  Creates an empty element specified by the elementPath expression. In this function you need to provide an index > 0
	  	  on which position the new element should be created.
	  Elements with the same name can be added multiple times.

	  <b>Fortran syntax:</b>

	  tixi_create_element_at_index( integer  handle, character*n parent_path, character*n element_name, integer index, integer error )
	  @cond
	  #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
	  @endcond

	  @param handle (in) file handle as returned by ::tixiCreateDocument

	  @param parentPath (in) an XPath compliant path to an element in the document
							 specified by handle (see section \ref XPathExamples above)
							 into which the new element is to be inserted. The parent
							 element has to exist already.

	  @param elementName (in) name of the element to be inserted into the parent element

	  @param index		 (in) position of the new created element

	  @return
		  - SUCCESS if successfully added the text element
		  - INVALID_XML_NAME if elementName is not a valid XML-element name
		  - INVALID_HANDLE if the handle is not valid
		  - INVALID_XPATH if elementPath is not a well-formed XPath-expression
		  - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
		  - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
		  - ALREADY_SAVED if element should be added to an already saved document
	  */
		DLL_EXPORT ReturnCode tixiCreateElementAtIndex (const TixiDocumentHandle handle, const char *parentPath, const char *elementName, int index);


/**
    @brief Removes an element.

    Removes an element from the document. It is not an error to remove
    a non existing element.

    <b>Fortran syntax:</b>

    tixi_remove_element( integer  handle, character*n element_path, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) file handle as returned by ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the
                       document specified by handle (see section \ref XPathExamples above).

    @return

     - SUCCESS if successfully removed the element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
 */

  DLL_EXPORT ReturnCode tixiRemoveElement (const TixiDocumentHandle handle, const char *elementPath);

/**
    @brief Returns the number of children elements with the same name.

    <b>Fortran syntax:</b>

    tixi_get_named_children_count( integer handle, character*n element_path, character*n child_name, int* count, integer error )

    @param handle (in) handle as returned by ::tixiCreateDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP
    @param elementPath elementPath (in) the path to an element in the document
                      specified by handle (see section \ref XPathExamples above).
    @param childName (in) name of children to be counted
    @param count (out) number of children with name childName.
                       0 is returned if either the element specified by elementPath has no
               children at all or has no children with name childName.

    @return

     - SUCCESS if a count is computed
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
     - NO_CHILD_NAME if childName is NULL

 */

  DLL_EXPORT ReturnCode tixiGetNamedChildrenCount (const TixiDocumentHandle handle,
                                                   const char *elementPath, const char *childName,
                                                   int *count);

/**
  @brief Returns the name of a child node beneath a given path.

  <b>Fortran syntax:</b>

  tixi_get_child_node_name( integer handle, character*n element_path, int* index, character*n child_name_array, integer error )

  @param handle (in) handle as returned by ::tixiCreateDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP
  @param parentElementPath (in) the path to the parent element in the document
                    specified by handle (see section \ref XPathExamples above).
  @param index (in) number index of the child-element of the given path.
  @param name (out) String containing the name of the child node. If the node is not a normal node, the name variable will contain:
                     - #text - in case of a text node
                     - #comment - in case of a comment node
                     - #cdata-section - in case of a CDATA section node

  @return

   - SUCCESS if a count is computed
   - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
   - INVALID_XPATH if elementPath is not a well-formed XPath-expression
   - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
   - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                             to a list of elements
*/
  DLL_EXPORT ReturnCode tixiGetChildNodeName (const TixiDocumentHandle handle,
                                              const char *parentElementPath, int index, char **name);

/**
  @brief Returns the number of child elements beneath a given path.

  <b>Fortran syntax:</b>

  tixi_get_number_of_childs( integer handle, character*n element_path, int* nchilds, integer error )

  @param handle (in) handle as returned by ::tixiCreateDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP
  @param elementPath elementPath (in) an XPath compliant path to an element in the document
                    specified by handle (see section \ref XPathExamples above).
  @param nChilds (out) Number of child elements beneath the given elementPath.

  @return

   - SUCCESS if a count is computed
   - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
   - INVALID_XPATH if elementPath is not a well-formed XPath-expression
   - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
   - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                             to a list of elements
*/
  DLL_EXPORT ReturnCode tixiGetNumberOfChilds(const TixiDocumentHandle handle, const char *elementPath, int* nChilds);

/*@}*/

/**
  \defgroup Attributes Attribute Handling Functions
  Functions to get the content of an element attribute as a string or a number,
  functions to create and manipulate attributes, and a function to remove attributes
  are described in this section.
 */
/*@{*/

/**
    @brief Retrieves value of an element's attribute as a string.

    Returns the value of an attribute specified by attributeName of the
    element, specified by elementPath, in the document specified by
    handle. On successful return the memory used for value is allocated
    internally and must not be released by the user. The memory is
    deallocated when the document referred to by handle is closed.

    <b>Fortran syntax:</b>

    tixi_get_text_attribute( integer  handle, character*n element_path, character*n attribute_name, character*n text, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be get from the element

    @param text (out) value of the specified attribute as a string

    @return

     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ATTRIBUTE_NOT_FOUND if the element has no attribute attributeName
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
 */
  DLL_EXPORT ReturnCode tixiGetTextAttribute (const TixiDocumentHandle handle,
                                              const char *elementPath, const char *attributeName,
                                              char **text);

/**
    @brief Retrieves value of an element's attribute as an integer.

    Returns the value of an attribute specified by attributeName of the
    element, specified by elementPath, in the document specified by
    handle. On successful return the memory used for value is allocated
    internally and must not be released by the user. The memory is
    deallocated when the document referred to by handle is closed.

    <b>Fortran syntax:</b>

    tixi_get_integer_attribute( integer  handle, character*n element_path, character*n attribute_name, integer *number, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be added to the element

    @param number (out)  value of the specified attribute as an integer value

    @return

     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ATTRIBUTE_NOT_FOUND if the element has no attribute attributeName
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
 */

  DLL_EXPORT ReturnCode tixiGetIntegerAttribute (const TixiDocumentHandle handle,
                                                 const char *elementPath, const char *attributeName,
                                                 int *number);

/**
    @brief Retrieves value of an element's attribute as an boolean.

    Returns the value of an attribute specified by attributeName of the
    element, specified by elementPath, in the document specified by
    handle. On successful return the memory used for value is allocated
    internally and must not be released by the user. The memory is
    deallocated when the document referred to by handle is closed.

    <b>Fortran syntax:</b>

    tixi_get_boolean_attribute( integer  handle, character*n element_path, character*n attribute_name, integer boolean, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be added to the element

    @param boolean (out)  value of the specified attribute as an boolean value

    @return

     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ATTRIBUTE_NOT_FOUND if the element has no attribute attributeName
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
 */

  DLL_EXPORT ReturnCode tixiGetBooleanAttribute (const TixiDocumentHandle handle,
                                                 const char *elementPath, const char *attributeName,
                                                 int *boolean);

/**
    @brief Retrieves value of an element's attribute as a floating point number.

    Returns the value of an attribute specified by attributeName of the
    element, specified by elementPath, in the document specified by
    handle. On successful return the memory used for value is allocated
    internally and must not be released by the user. The memory is
    deallocated when the document referred to by handle is closed.

    <b>Fortran syntax:</b>

    tixi_get_double_attribute( integer  handle, character*n element_path, character*n attribute_name, real *number, integer error )

    @param handle (in) handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be added to the element

    @param number (out) value of the specified attribute as a floating point value

    @return

     - SUCCESS if successfully retrieve the text content of a single element
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ATTRIBUTE_NOT_FOUND if the element has no attribute attributeName
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
 */

  DLL_EXPORT ReturnCode tixiGetDoubleAttribute (const TixiDocumentHandle handle,
                                                const char *elementPath, const char *attributeName,
                                                double *number);




/**

    @brief Adds an attribute with a string value to an element.

    Adds an attribute with name attributeName and value attributeValue
    to an element specified by the elementPath expression. If the
    attribute already exists its previous value is replaced by text.

    <b>Fortran syntax:</b>

    tixi_add_text_attribute( integer  handle, character*n element_path, character*n attribute_name, character*n attribute_value, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) file handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be added to the element

    @param attributeValue (in) text to assigned to the attribute. If attributeValue is
                               NULL the empty string will be assigned to the attribute.

    @return

     - SUCCESS if successfully retrieve the text content
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - NO_ATTRIBUTE_NAME if attributeName is NULL
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
     - ALREADY_SAVED if element should be added to an already saved document
     - INVALID_XML_NAME if attributeName is not a valid XML-element name
 */

  DLL_EXPORT ReturnCode tixiAddTextAttribute (const TixiDocumentHandle handle,
                                              const char *elementPath, const char *attributeName,
                                              const char *attributeValue);


/**
    @brief Adds an attribute with a floating point number value to an element.

    Adds an attribute with name attributeName and a floating point
    number value to an element specified by the elementPath
    expression. If the attribute already exists its previous value is
    replaced.

    <b>Fortran syntax:</b>

    tixi_add_double_attribute( integer  handle, character*n element_path, character*n attribute_name, real number, character*n format, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) file handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be added to the element

    @param number (in) floating point value to be assigned to the attribute. If
                       number is NULL an error is return and the attribute
                       is not created.

    @param format (in) format used to convert attributeValue into a string. If format is
                       NULL "%g" will be used to format the string.

    @return

     - SUCCESS if successfully retrieve the text content
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - NO_NUMBER if number is NULL
     - NO_ATTRIBUTE_NAME if attributeName is NULL
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
 */
  DLL_EXPORT ReturnCode tixiAddDoubleAttribute (const TixiDocumentHandle handle,
                                                const char *elementPath, const char *attributeName,
                                                double number, const char *format);


/**
    @brief Adds an attribute with an integer number value to an element.

    Adds an attribute with name attributeName and a integer number
    value to an element specified by the elementPath expression. If the
    attribute already exists its previous value is replaced.

    <b>Fortran syntax:</b>

    tixi_add_integer_attribute( integer  handle, character*n element_path, character*n attribute_name, integer number, character*n format, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) file handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the
                       document specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be added to the element

    @param number (in) integer value to be assigned to the attribute. If
                       number is NULL an error is return and the attribute
                       is not created.

    @param format (in) format used to convert attributeValue into a string. If format is
                       NULL "%d" will be used to format the string.

    @return

     - SUCCESS if successfully retrieve the text content
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - NO_NUMBER if number is NULL
     - NO_ATTRIBUTE_NAME if attributeName is NULL
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
 */

  DLL_EXPORT ReturnCode tixiAddIntegerAttribute (const TixiDocumentHandle handle,
                                                 const char *elementPath, const char *attributeName,
                                                 int number, const char *format);

/**
    @brief Removes an attribute

    Removes an attribute from an element. It is not an error to remove
    an non existing attribute.

    <b>Fortran syntax:</b>

    tixi_remove_attribute( integer handle, character*n element_path, character*n attribute_name, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) file handle as returned by ::tixiOpenDocument or ::tixiCreateDocument

    @param elementPath (in) an XPath compliant path to an element in the
                       document specified by handle (see section \ref XPathExamples above).

    @param attributeName (in) name of the attribute to be added to the element

    @return

     - SUCCESS if successfully removed the attribute
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - NO_ATTRIBUTE_NAME if attributeName is NULL
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
 */

  DLL_EXPORT ReturnCode tixiRemoveAttribute (const TixiDocumentHandle handle,
                                             const char *elementPath, const char *attributeName);
    

  /**
    @brief Returns the number of attributes  of a given node.
  
    <b>Fortran syntax:</b>
  
    tixi_get_number_of_attributes( integer handle, character*n element_path, int* nattr, integer error )
  
    @param handle (in) handle as returned by ::tixiCreateDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP
    @param elementPath elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).
    @param nAttributes (out) Number of attributes of a given node.
  
    @return
    
     - SUCCESS if a count is computed
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
  */
    DLL_EXPORT ReturnCode tixiGetNumberOfAttributes(const TixiDocumentHandle handle, const char *elementPath, int* nAttributes);
    
    
  /**
    @brief Returns the name of an attribute beneath a given path.
  
    <b>Fortran syntax:</b>
  
    tixi_get_attribute_name( integer handle, character*n element_path, int* index, character*n attr_name_array, integer error )
  
    @param handle (in) handle as returned by ::tixiCreateDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP
    @param elementPath elementPath (in) an XPath compliant path to an element in the document
                      specified by handle (see section \ref XPathExamples above).
    @param attrIndex (in) number index of the attribute of the given path (counting from 1...tixiGetNumberOfAttributes)
    @param attrName (out) String containing the attribute name.
  
    @return
  
     - SUCCESS if a count is computed
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if elementPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but
                               to a list of elements
  */
    DLL_EXPORT ReturnCode tixiGetAttributeName(const TixiDocumentHandle handle, const char *elementPath, int attrIndex, char** attrName);
  
/*@}*/

/**
  \defgroup MiscFunctions Miscellaneous Functions
    These function simply do not fit into one of the other categories.
 */
/*@{*/


/**
    @brief Add a name of an external file as a url.

    <b>Fortran syntax:</b>

    tixi_add_external_link( integer  handle, character*n parent_path, character*n url, character*n file_format, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) as returned by ::tixiCreateDocument
    @param parentPath (in) path to the element into which the element holding the url
                           should be inserted.
    @param url (in) an url to specify an additional output file not in XML-format
    @param fileFormat (in) an optional attribute (may be NULL) to specify a file format,
                            e.g. CNGS, netcdf, ...
    @return

        - SUCCESS if successfully added the header
        - FAILED if an internal error occured
        - INVALID_HANDLE if the handle is not valid
        - ALREADY_SAVED if the header should be added to an already saved document
*/
  DLL_EXPORT ReturnCode tixiAddExternalLink (const TixiDocumentHandle handle, const char *parentPath,
                                             const char *url, const char *fileFormat);


/**
    @brief Add header to XML-file.

    Inserts a header containing information on the tool used to create the file, its
    version and the user. Additionally, the TIXI version is inserted. If an strings
    equals to NULL an empty element is inserted. This routine shold be called right after
    ::tixiCreateDocument.

    <b>Fortran syntax:</b>

    tixi_add_header( integer handle, character*n tool_name, character*n version, character*n author_name, integer error )
    @cond
    #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
    @endcond

    @param handle (in) as returned by ::tixiCreateDocument
    @param toolName (in) name of the tool used to write the file
    @param authorName (in) string to identify the creator of the file
    @param version (in) string to identify the version of the tool

    @return

        - SUCCESS if successfully added the header
        - FAILED if an internal error occured
        - INVALID_HANDLE if the handle is not valid
        - ALREADY_SAVED if the header should be added to an already saved document
*/
  DLL_EXPORT ReturnCode tixiAddHeader (const TixiDocumentHandle handle, const char *toolName,
                                       const char *version, const char *authorName);



/**
@brief Add CPACS header to XML-file.

Inserts a header containing information on the data set in this file.
If an strings equals to NULL an empty element is inserted. This routine should be called right after
::tixiCreateDocument. An timestamp is automaticly added to the header.

<b>Fortran syntax:</b>

tixi_add_cpacs_header( integer handle, character*n name, character*n creator, character*n version, character*n descripton, character*n cpacs_version, integer error )
@cond
#PY:# no output params (explicit mark necessary, otherwise assuming last = output)
@endcond

@param handle (in) as returned by ::tixiCreateDocument
@param name (in) name of the data set
@param creator (in) string to identify the creator of the file
@param version (in) string to identify the version of the file
@param description (in) optional string to set a description to the file
@param cpacsVersion (in) CPACS version number

@return

- SUCCESS if successfully added the header
- FAILED if an internal error occured
- INVALID_HANDLE if the handle is not valid
- ALREADY_SAVED if the header should be added to an already saved document */
DLL_EXPORT ReturnCode tixiAddCpacsHeader (const TixiDocumentHandle handle, const char *name, const char *creator,
                                          const char *version, const char *description, const char * cpacsVersion);

  
/**
  @brief Checks if the given element exists.

  @cond
  #PY:# no output params (explicit mark necessary, otherwise assuming last = output)
  @endcond

  @param handle (in) handle as returned by ::tixiCreateDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP
  @param elementPath (in) an XPath compliant path to an element in the document
                          specified by handle (see section \ref XPathExamples above).

 @return
    - SUCCESS if the element exists
    - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
    - FAILED internal error
    - INVALID_XPATH  if elementPath is not a well-formed XPath-expression
    - ELEMENT_NOT_FOUND if the elementPath does not point to an existing element
 */

  DLL_EXPORT ReturnCode tixiCheckElement (const TixiDocumentHandle handle, const char *elementPath);


/** @brief Checks for validity of a document handle


    <b>Fortran syntax:</b>

    tixi_check_handle( integer handle )

    @param handle (in)  handle as returned by ::tixiCreateDocument, ::tixiOpenDocumentRecursive or ::tixiOpenDocumentFromHTTP
    @return

    - SUCCESS if handle is valid
    - INVALID_HANDLE if the handle is not valid, i.e. does not or no longer exist

*/
  DLL_EXPORT ReturnCode tixiCheckDocumentHandle (const TixiDocumentHandle handle);

  /**
      @brief Sets Pretty print on or off.

      Set pretty print on or off. This is used when saing a document to a file,
      or when exporting to a string.
      0 turns pretty print off, 1 turns pretty print on.
      By default, pretty print is turned on.h

      <b>Fortran syntax:</b>

      tixi_use_pretty_print( integer  handle, integer use_pretty_print, integer error )
      @cond
      #PY:#
      @endcond

      @param handle  (in) handle as returned by ::tixiOpenDocument
      @param usePrettyPrint (in) flag if output should be pretty printed

      @return
       - SUCCESS if a count is computed
       - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
       - FAILED for internal errors or if usePrettyPrint had wrong value
  */
  DLL_EXPORT ReturnCode tixiUsePrettyPrint(TixiDocumentHandle handle, int usePrettyPrint);


/*@}*/

/**
  \defgroup HighLevelFunctions High Level Functions
    These functions operate on more complex data structures than the elementary get/add function so.
 */
/*@{*/

/**
    @brief High level routine to write a list of elements with attributes.

    Adds a list of elements with the same name containd in an element with the name listName to the element
    specified by parentPath.

    <b>Fortran syntax:</b>

    tixi_add_double_list_with_attributes( integer  handle, character*n parent_path, character*n list_name, character*n child_name, character*n child_attribute_name, real values, character*n format, character*n attributes, integer n_values, integer error )
    @cond
    #PY:#
    @endcond

    @param handle (in) file handle as returned by ::tixiCreateDocument

    @param parentPath (in) an XPath compliant path to an element in the
                       document specified by handle (see section \ref XPathExamples above).

    @param listName (in) name of the element containing the list

    @param childName (in) name of the child elements of the element listName

    @param childAttributeName (in) name of the attribute to be assigned to each child

    @param values (in) array of double holding the element values to be added ("text" content between the tags)

    @param format (in) format used to convert the values into strings. If format is
                       NULL "%g" will be used to format the string.

    @param attributes (in) array of pointers to strings holding the attribute values (vs. attribute name which is the same for each list entry)

    @param nValues (in) number of values in the list

    @return
     - SUCCESS if the list has been added successfully
     - FAILED if an internal error occured
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if parentPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if parentPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element

*/

  DLL_EXPORT ReturnCode tixiAddDoubleListWithAttributes (const TixiDocumentHandle handle,
                                                         const char *parentPath,
                                                         const char *listName, const char *childName,
                                                         const char *childAttributeName,
                                                         double *values, const char *format,
                                                         const char **attributes, int nValues);

/**
   @brief Adds an element containing a matrix with empty elements.

   An element containing an n x m matrix with empty entries is added.

   @cond
   #PY:#
   @endcond

  @param handle (in) as returned by ::tixiCreateDocument
  @param parentPath (in) an XPath compliant path to an element into
                         which the matrix element is to be inserted in the document
                         specified by handle (see section \ref XPathExamples).
  @param matrixName (in) name to be used for the matrix element
  @param rowName (in) name to be used for row elements, if NULL "r" is used
  @param columnName (in) name to be used for column elements, if NULL "c" is used
  @param nRows (in) number of rows
  @param nColumns (in) number of columns

  @return

    - SUCCESS if successfully added the matrix element
    - FAILD internal error
    - INVALID_HANDLE if the handle is not valid
    - INVALID_XPATH if parentPath is not a well-formed XPath-expression
    - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
    - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
    - ALREADY_SAVED if element should be added to an already saved document
    - MATRIX_DIMENSION_ERROR if nRows or nColumns or both are less than 1

*/
  DLL_EXPORT ReturnCode tixiCreateMatrix (const TixiDocumentHandle handle, const char *parentPath,
                                          const char *matrixName, const char *rowName,
                                          const char *columnName, const int nRows, const int nColumns);

/**
   @brief Adds an element containing a matrix.

   An element containing an nxm matrix is added. Each matrix entry is
   placed in an element by its own. The matrix element contains n row
   elements each consisting of m column elements. The latter hold the
   matrix entries. The tag names for rows and column elements and the
   matrix name can be chosen by the user. If none are supplied the tag
   r is used for rows and c for column elements.

   Example: 2x2 matrix
     11. 12
   (        )
     21. 22

@verbatim
   double array[4] = {  11., 12.,  21., 22.};
   tixiAddFloatMatrix(  handle, "/parent", "name", NULL, NULL, 2, 2, ROW_WISE, array, "%3.1f" );
@endverbatim

   assumes row wise storage and results in

@verbatim
   <name>
       <r>
           <c>11.</c>
           <c>12.</c>
       </r>
       <r>
       <c>21.</c>
           <c>22.</c>
       </r>
   </name>
@endverbatim

  while
@verbatim
   double array[4] = {  11., 12.,  21., 22.};
   tixiAddFloatMatrix(  handle, "/parent", "name", NULL, NULL, 2, 2, COLUMN_WISE, array, "%3.1f" );
@endverbatim
   assumes column wise storage and gives

@verbatim
   <name>
       <r>
           <c>11.</c>
           <c>21.</c>
       </r>
       <r>
       <c>12.</c>
           <c>22.</c>
       </r>
   </name>
@endverbatim

   If user supplied names are used this may look like
@verbatim
   double array[4] = {  11., 12.,  21., 22.};
   tixiAddFloatMatrix(  handle, "/parent", "name", "blub", "bla", 2, 2, ROW_WISE, array, "%3.1f" );
@endverbatim

@verbatim
   <myMatrix>
       <blub>
           <bla>11.</bla>
           <bla>12.</bla>
       </blub>
       <blub>
       <bla>21.</bla>
           <bla>22.</bla>
       </blub>
   </myMatrix>
@endverbatim



   <b>Fortran syntax:</b>

   tixi_add_float_matrix( integer handle, character*n parentPath,
              character*n matrixName, character*n rowName,
              character*n columnName, integer nRows, integer nColumns,
              integer mode, real array(nRows*nColumns), character*n format, error );

   Fortran example:

   real array
   dimension array(2,2)
   data array /11., 12., 21., 22./
   tixi_add_float_matrix(  handle, "/parent", "name", "", "", 2, 2, ROW_WISE, array, "%3.1f", error )

   results in the output of the first example above and

  tixi_add_float_matrix(  handle, "/parent", "name", "", "", 2, 2, COLUMN_WISE, array, "%3.1f", error )

  gives the output of the second example which conforms to the
  expectation that e.g. array(2,1), which is 12., is stored at the
  position (2,1) in the matrix. Row and column numbers in error
  messages refer to a numbering starting at one.

  @cond
  #PY:#  
  @endcond


  @param handle (in) as returned by ::tixiCreateDocument
  @param parentPath (in) an XPath compliant path to an element into
                         which the matrix element is to be inserted in the document
                         specified by handle (see section \ref XPathExamples).
  @param matrixName (in) name to be used for the matrix element
  @param rowName (in) name to be used for row elements, if NULL "r" is used
  @param columnName (in) name to be used for column elements, if NULL "c" is used
  @param nRows (in) number of rows
  @param nColumns (in) number of columns
  @param mode (in) if mode is ROW_WISE the entries of array are assumed to be in row order
                   if mode is COLUMN_WISE the entries of array are assumed to be in column order
  @param array (in) array holding the matrix entries
  @param format (in) format used to convert the coordinate values into a string. If format is
                       NULL "%g" will be used.
  @return

    - SUCCESS if successfully added the matrix element
    - FAILD internal error
    - INVALID_HANDLE if the handle is not valid
    - INVALID_XPATH if parentPath is not a well-formed XPath-expression
    - ELEMENT_PATH_NOT_UNIQUE if parentPath resolves not to a single element but to a list of elements
    - ELEMENT_NOT_FOUND if parentPath points to a non-existing element
    - ALREADY_SAVED if element should be added to an already saved document
    - MATRIX_DIMENSION_ERROR if nRows or nColumns or both are less than 1

*/
  DLL_EXPORT ReturnCode tixiAddFloatMatrix (const TixiDocumentHandle handle, const char *parentPath,
                                            const char *matrixName, const char *rowName,
                                            const char *columnName, int nRows, int nColumns,
                                            StorageMode mode, double *array,
                                            const char *format);

/**
   @brief Retrieves a matrix.

   An element containing an nxm matrix in the format described in
   ::tixiAddFloatMatrix is read and its contents is stored into an
   user supplied array. If the size is unknown it can be retrieve by
   ::tixiGetMatrixSize. Row and column numbers in error messages refer
   to a numbering starting at one.
   
   @cond
   #PY:6:3;4# # use row * col output array (row = index 3, col = index 4)
   @endcond

  @param handle (in) as returned by ::tixiCreateDocument
  @param matrixPath (in) an XPath compliant path to an element holding the matrix in
                         the document specified by handle (see section \ref XPathExamples).
  @param rowName (in) name of the row elements
  @param columnName (in) name of the column elements
  @param nRows (in) number of rows
  @param nColumns (in) number of columns
  @param mode (in) if mode is ROW_WISE the matrix will be stored in the array in row order
                   if mode is COLUMN_WISE the matrix will be stored in the array in column order
  @param array (out) array holding the matrix entries

  @return
    - SUCCESS if successfully read the matrix element
    - FAILD internal error
    - INVALID_HANDLE if the handle is not valid
    - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
    - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
    - ELEMENT_NOT_FOUND if matrixPath points to a non-existing element
    - NON_MATCHING_NAME if row or column name specified do not match the names used in the document
    - NON_MATCHING_SIZE if nRows and nColumns do not match the sizes of the matrix in the document

*/
  DLL_EXPORT ReturnCode tixiGetFloatMatrix (const TixiDocumentHandle handle, const char *matrixPath,
                                            const char *rowName, const char *columnName,
                                            const int nRows, const int nColumns,
                                            StorageMode mode, double *array);
/**
   @brief Retrieves the size of a matrix.

   The size of a matrix stored in the format described in
   ::tixiAddFloatMatrix is determined for use in
   tixiGetFloatMatrix. Row and column numbers in error messages refer
   to a numbering starting at one.

   @cond
   #PY:3,4#
   @endcond

  @param handle (in) file handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
  @param matrixPath (in) an XPath compliant path to an element holding the matrix in
                         the document specified by handle (see section \ref XPathExamples).
  @param rowName (in) name of the row elements
  @param columnName (in) name of the column elements
  @param nRows (out) number of rows
  @param nColumns (out) number of columns

  @return

    - SUCCESS if successfully read the matrix element
    - FAILD internal error
    - INVALID_HANDLE if the handle is not valid
    - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
    - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
    - ELEMENT_NOT_FOUND if matrixPath points to a non-existing element
    - NON_MATCHING_NAME if row or column name specified do not match the names used in the document

*/
  DLL_EXPORT ReturnCode tixiGetMatrixSize (const TixiDocumentHandle handle, const char *matrixPath,
                                           const char *rowName, const char *columnName,
                                           int *nRows, int *nColumns);



  /**
     @brief Retrieves the size of an Vector.

    Returns the size of semicolon separated elementf in an vector. The node containung
    the vector has to be tagged via the xml attribute <<mapType="vector">>.

    tixi_get_vector_size( integer  handle,  character*n vectorPath, integer nElements, integer error)

    @param handle (in) file handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
    @param vectorPath (in) an XPath compliant path to an element holding the vector in
                           the document specified by handle (see section \ref XPathExamples).
    @param nElements (out) number of vector elements

    @return
      - SUCCESS if successfully read the matrix element
      - FAILED internal error
      - INVALID_HANDLE if the handle is not valid
      - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
      - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
      - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array

  */
    DLL_EXPORT ReturnCode tixiGetVectorSize (const TixiDocumentHandle handle,
                                             const char *vectorPath, int *nElements);


    /**
       @brief Retrieves a vector.

       A vector is read and its contents are stored into an 1D-array. The memory necessary
       for the array is automatically allocated. The number of elements in the vector
       could be read via a call to "tixiGetVectorSize".

       tixi_get_float_vector( integer handle, character*n vectorPath, real array, integer eNumber)

       @cond
       #PY:1:1# unconventional position of output parameter
       @endcond

      @param handle (in) file handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
      @param vectorPath (in) an XPath compliant path to an element holding the vector in
                             the document specified by handle (see section \ref XPathExamples).
      @param vectorArray (out) a pointer address for an array that should hold the vector elements
      @param eNumber (in) maximal number of elements that shout be read from this vector.

      @return
        - SUCCESS if successfully read the vector
        - FAILED internal error
        - INVALID_HANDLE if the handle is not valid
        - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
        - ELEMENT_PATH_NOT_UNIQUE if vectorPath resolves not to a single element but to a list of elements
        - ELEMENT_NOT_FOUND if vectorPath points to a non-existing element
    */
      DLL_EXPORT ReturnCode tixiGetFloatVector (const TixiDocumentHandle handle, const char *vectorPath,
                                                double **vectorArray, const int eNumber);


  /**
    @brief Retrieves the number of dimensions of an Array.

    Returns the number of separate dimensions defined in sub-tags of the array in CPACS.
    For each dimension there is a finite set of allowed values that can be retrieved by
    "tixiGetArrayValues".

    Example:

    @code{.xml}
    <aeroPerformanceMap>
        <machNumber mapType="vector">0.2;0.6</machNumber>
        <reynoldsNumber mapType="vector">10000000;30000000</reynoldsNumber>
        <angleOfYaw mapType="vector">-5.;0;15</angleOfYaw>
        <angleOfAttack mapType="vector">-4.;-2.;0.;2.;4.;6.;8.;10.;12.;14.;16.</angleOfAttack>
        <cfx mapType="array">...</array>
        <cfy mapType="array">...</array>
        <cfz mapType="array"/>
        <cmx mapType="array"/>
        <cmy mapType="array"/>
        <cmz mapType="array"/>
    </aeroPerformanceMap>
    @endcode

    This is a typical array definition, to be found under, e.g., the xpath "//aeroPerformanceMap":
    - All direct child tags with the attribute 'mapType="vector"' define one dimension each, containing several ";" separated values.
    - All direct child tags with the attribute 'mapType="array"' define a list of values, the list having a size of the
        cross product of all dimensions' sizes (in this example 2*2*3*11 = 132 elements per "array" list).

    <b>Fortran syntax:</b>

    tixi_get_array_dimensions( integer handle, character*n arrayPath, integer* dimensions )

    @param handle (in) as returned by ::tixiCreateDocument
    @param arrayPath (in) an XPath-compliant path to an element holding the sub elements that define the dimensions in
                           the document specified by the handle (see section \ref XPathExamples).
    @param dimensions (out) number of array dimensions

    @return
      - SUCCESS if the dimensions have been successfully read
      - FAILED for internal errors
      - INVALID_HANDLE if the handle is not valid
      - INVALID_XPATH if arrayPath is not a well-formed XPath-expression
      - ELEMENT_PATH_NOT_UNIQUE if arrayPath resolves not to a single element but to a list of elements
      - ELEMENT_NOT_FOUND if the rrayPath points to a element that is no array

  */
    DLL_EXPORT ReturnCode tixiGetArrayDimensions (const TixiDocumentHandle handle,
                                                  const char *arrayPath, int *dimensions);


  /**
    @brief Retrieves the sizes of all dimensions of the array definition, and the overall array size (product of all dimensions's sizes).

    For an array use example, please check tixiGetArrayDimensions()

    <b>Fortran syntax:</b>

    tixi_get_array_dimension_sizes ( integer handle, character*n arrayPath, int*n sizes, int* arraySizes )

    @cond
    #PY:1,2:-1,0# unconventional position of output parameter (first is -1: manual, second is 0: no array)
    @endcond

    @param handle (in) as returned by ::tixiCreateDocument
    @param arrayPath (in) an XPath-compliant path to the top XML element holding the sub-tags of the array definition in
                           the document specified by the handle (see section \ref XPathExamples).
    @param sizes (out) an integer array containing the size of each dimension
    @param linearArraySize (out) product over all sizes (for complete array size)

    @return
      - SUCCESS if the dimensions have been successfully read
      - FAILED internal error
      - INVALID_HANDLE if the handle is not valid
      - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
      - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
      - ELEMENT_NOT_FOUND if the arrayPath points to a element that is no array

  */
    DLL_EXPORT ReturnCode tixiGetArrayDimensionSizes (const TixiDocumentHandle handle, const char *arrayPath,
                                                      int *sizes, int *linearArraySize);


    /**
      @brief Retrieves the names of all dimensions.

      For an array use example, please check tixiGetArrayDimensions()

      <b>Fortran syntax:</b>

      tixi_get_array_dimension_names ( integer handle, character*n arrayPath, char*n*m dimensionNames)

      @cond
      #PY:1:-1# one user specified return array (of strings)
      @endcond

      @param handle (in) file handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
      @param arrayPath (in) an XPath compliant path to an element holding the sub-tags of the array definition in
                             the document specified by handle (see section \ref XPathExamples).
      @param dimensionNames (out) array of strings

      @return
        - SUCCESS if successfully read the matrix element
        - FAILED internal error
        - INVALID_HANDLE if the handle is not valid
        - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
        - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
        - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array

    */
      DLL_EXPORT ReturnCode tixiGetArrayDimensionNames (const TixiDocumentHandle handle,
                                                        const char *arrayPath, char **dimensionNames);


    /**
      @brief Retrieves the selected dimension's values (e.g. separate allowed angles etc.).

      For an array use example, please check tixiGetArrayDimensions()

      <b>Fortran syntax:</b>

      tixi_get_array_dimension_values ( integer handle, character*n arrayPath, int* dimension, double *dimensionValues)

      @param handle (in) file handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
      @param arrayPath (in) an XPath compliant path to an element holding the sub-tags of the array definition in
                             the document specified by handle (see section \ref XPathExamples).
      @param dimension (in) which dimension to return meaning values for
      @param dimensionValues (out) all values for this dimension. The return array's Size should be determined by a prior call to tixiGetArrayDimensionSizes()

      @return
        - SUCCESS if successfully read the matrix element
        - FAILED internal error
        - INVALID_HANDLE if the handle is not valid
        - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
        - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
        - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array

    */
      DLL_EXPORT ReturnCode tixiGetArrayDimensionValues (const TixiDocumentHandle handle, const char *arrayPath,
                                                         const int dimension, double *dimensionValues);


      /**
        @brief Retrieves the number of parameters of an array.

        For an array use example, please check tixiGetArrayDimensions()

        Returns the number of different parameters (cutting all dimensions) defined in sub-tags in CPACS.
        Parameters are the tags with the attribute 'mapType="array"'.

        <b>Fortran syntax:</b>

        tixi_get_array_parameters( integer handle, character*n arrayPath, integer* parameters )

        @param handle (in) as returned by ::tixiCreateDocument
        @param arrayPath (in) an XPath compliant path to an element holding the sub elements that define the dimensions in
                               the document specified by handle (see section \ref XPathExamples).
        @param parameters (out) number of array parameters

        @return
          - SUCCESS if successfully read the dimensions
          - FAILED internal error
          - INVALID_HANDLE if the handle is not valid
          - INVALID_XPATH if arrayPath is not a well-formed XPath-expression
          - ELEMENT_PATH_NOT_UNIQUE if arrayPath resolves not to a single element but to a list of elements
          - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array

      */
        DLL_EXPORT ReturnCode tixiGetArrayParameters (const TixiDocumentHandle handle, const char *arrayPath, int *parameters);


        /**
          @brief Retrieves names of all parameters

          For an array use example, please check tixiGetArrayDimensions()

          <b>Fortran syntax:</b>

          tixi_get_array_parameter_names ( integer handle, character*n arrayPath, character*n*m parameterNames)

          @param handle (in) as returned by ::tixiCreateDocument
          @param arrayPath (in) an XPath compliant path to an element holding the sub-tags of the array definition in
                                 the document specified by handle (see section \ref XPathExamples).
          @param parameterNames (out) string array containing names of each parameter

          @return
            - SUCCESS if successfully read the matrix element
            - FAILED internal error
            - INVALID_HANDLE if the handle is not valid
            - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
            - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
            - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array

        */
          DLL_EXPORT ReturnCode tixiGetArrayParameterNames (const TixiDocumentHandle handle,
                                                            const char *arrayPath, char **parameterNames);


      /**
        @brief Reads in an array. The memory management of the array is done by tixi.

        For an array use example, please check ::tixiGetArrayDimensions()

        <b>Fortran syntax:</b>

        tixi_get_array ( integer handle, character*n arrayPath, character*n element, double *values)

        @param handle (in) file handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
        @param arrayPath (in) an XPath compliant path to an element holding the sub-tags of the array definition in
                               the document specified by handle (see section \ref XPathExamples).
        @param elementName (in) name of the sub tag that contains the array
        @param arraySize (in) Total size of the array. Size must be determined by calling ::tixiGetArrayDimensionSizes and must equal
                           the product of all dimensions' sizes
        @param values (out) Pointer to a double array, containg all values for this dimension. The array is allocated and freed
                           by tixi. The size of the array corresponds to the parameter arraySize

        @return
          - SUCCESS if successfully read the matrix element
          - FAILED internal error
          - INVALID_HANDLE if the handle is not valid
          - INVALID_XPATH if matrixPath is not a well-formed XPath-expression
          - ATTRIBUTE_NOT_FOUND if the given sub element has not mapType="array"
          - ELEMENT_PATH_NOT_UNIQUE if matrixPath resolves not to a single element but to a list of elements
          - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array
          - NON_MATCHING_SIZE if arraySize does not match number of read elements in specified array
      */
      DLL_EXPORT ReturnCode tixiGetArray (const TixiDocumentHandle handle, const char *arrayPath,
                                          const char *elementName, int arraySize, double **values);


      /**
        @brief Getter function to take one multidimensionally specified element from a complete array, retrieved earlier.

        For an array use example, please check tixiGetArrayDimensions()

        <b>Fortran syntax:</b>

        tixi_get_array_value ( double*n array, integer*n dimSize, integer*n dimPos, integer *dims)

        @param array (in) the array as returned by ::tixiGetArray()
        @param dimSize (in) the array of dimensions' sizes as returned by ::tixiGetArrayDimensionSizes()
        @param dimPos (in) the index of each dimension to fetch from the array
        @param dims (in) the number of dimensions of the array as returned by ::tixiGetArrayDimensions()

        @return The element fetched
      */
      DLL_EXPORT double tixiGetArrayValue(const double *array, const int *dimSize, const int *dimPos, const int dims);


      /**
        @brief Helper function.

        Returns the number of sub elements in CPACS arrays (either vector or array type).

        For an array use example, please check ::tixiGetArrayDimensions(). The result for elementName="angleOfAttack" would be 11.

        <b>Fortran syntax:</b>

        tixi_get_array_element_count( integer handle, character*n arrayPath, character*n elementName, integer* elements )

        @param handle (in) as returned by ::tixiCreateDocument
        @param arrayPath (in) an XPath compliant path to an element holding the sub elements that define the dimensions in
                               the document specified by handle (see section \ref XPathExamples).
        @param elementName The name of the sub element under the given xpath.
        @param elements (out) number of array parameters (separated by ";")

        @return
          - SUCCESS if successfully read the dimensions
          - FAILED internal error
          - INVALID_HANDLE if the handle is not valid
          - INVALID_XPATH if arrayPath is not a well-formed XPath-expression
          - ELEMENT_PATH_NOT_UNIQUE if arrayPath resolves not to a single element but to a list of elements
          - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array

      */
      DLL_EXPORT ReturnCode tixiGetArrayElementCount (const TixiDocumentHandle handle, const char *arrayPath,
                                                      const char *elementName, int *elements);


        /**
          @brief Helper function.

          Returns the tag names of sub elements of mapType given.

          For an array use example, please check tixiGetArrayDimensions()

          <b>Fortran syntax:</b>

          tixi_get_array_element_names( integer handle, character*n arrayPath, character*n elementName, character*n*m elementNames)

          @param handle (in) as returned by ::tixiCreateDocument
          @param arrayPath (in) an XPath compliant path to an element holding the sub elements that define the dimensions in
                                 the document specified by handle (see section \ref XPathExamples).
          @param elementType (in) mapType to get names for (either "vector" for a dimension or "array" for the data field)
          @param elementNames (out) string names of all tags found for the given type.

          @return
            - SUCCESS if successfully read the dimensions
            - FAILED internal error
            - INVALID_HANDLE if the handle is not valid
            - INVALID_XPATH if arrayPath is not a well-formed XPath-expression
            - ELEMENT_PATH_NOT_UNIQUE if arrayPath resolves not to a single element but to a list of elements
            - ELEMENT_NOT_FOUND if arrayPath points to a element that is no array

        */
      DLL_EXPORT ReturnCode tixiGetArrayElementNames (const TixiDocumentHandle handle, const char *arrayPath,
                                                      const char *elementType, char **elementNames);


/**
   @brief Adds an element containing the 3D cartesian coordinates of a point.

   An element with the following structure is added:

@verbatim
       <x> </x>
       <y> </y>
       <z> </z>
@endverbatim

  <b>Fortran syntax:</b>

  tixi_add_point( integer  handle,  character*n pointParentPath,
                  real x, real y, real z, character*n format, integer error )
  @cond
  #PY:#
  @endcond

  @param handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
  @param pointParentPath (in) an XPath compliant path to an element into
                         which the point elements are to be inserted in the document
                         specified by handle (see section \ref XPathExamples).
  @param x (in) x coordinate of the point
  @param y (in) y coordinate of the point
  @param z (in) z coordinate of the point
  @param format (in) format used to convert the coordinate values into a string. If format is
                       NULL "%g" will be used.

  @return
    - SUCCESS if successfully added the point element
    - INVALID_HANDLE if the handle is not valid
    - INVALID_XPATH if pointParentPath is not a well-formed XPath-expression
    - ELEMENT_PATH_NOT_UNIQUE if pointParentPath resolves not to a single element but to a list of elements
    - ELEMENT_NOT_FOUND if pointParentPath points to a non-existing element
    - ALREADY_SAVED if element should be added to an already saved document

*/
  DLL_EXPORT ReturnCode tixiAddPoint (const TixiDocumentHandle handle, const char *pointParentPath,
                                      double x, double y, double z, const char *format);

/**
   @brief Reads a point element but ignores error if an incomplete point is encountered.

   An element with the following structure is expected as child of the
   element specified by pointParentPath

@verbatim
       <x> </x>
       <y> </y>
       <z> </z>
@endverbatim

    @brief Reads a point element.

   An element with the following structure is expected as child of the element specified by pointParentPath.
   Some or even all coordinate entries may be missing. The value of a
    missing coordiante is unchanged on output. This feature can be used to provide default values
    for non existing coordinate elements.

   The coordinates are returned in the x, y, and z arguments.

  <b>Fortran syntax:</b>

  tixi_get_point(  integer  handle,  character*n pointParentPath, integer index,
                   real x, real y, real z, integer error )

  @cond
  #PY:1,2,3# unconventional position of output parameter
  @endcond

  @param handle (in) as returned by ::tixiOpenDocument
  @param pointParentPath (in) an XPath compliant path to an element containing point elements
                              in the document specified by handle (see section \ref XPathExamples).
  @param x (out) x coordinate of the point
  @param y (out) y coordinate of the point
  @param z (out) z coordinate of the point

  @return
     - SUCCESS if the content of the point element is retrieved successfully
     - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
     - INVALID_XPATH if pointParentPath is not a well-formed XPath-expression
     - ELEMENT_NOT_FOUND if pointParentPath does not point to a node in the XML-document
     - ELEMENT_PATH_NOT_UNIQUE if pointParentPath resolves not to a single element but to a list of elements
*/
  DLL_EXPORT ReturnCode tixiGetPoint (const TixiDocumentHandle handle,
                                      const char *pointParentPath,
                                      double *x, double *y, double *z);



  /**
      @brief Checks the existence of an element's attribute.

      <b>Fortran syntax:</b>

      tixi_check_attribute( integer  handle, character*n element_path, character*n attribute_name, integer error )
      @cond
      #PY:#
      @endcond

      @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString

      @param elementPath (in) an XPath compliant path to an element in the document
                        specified by handle (see section \ref XPathExamples above).

      @param attributeName (in) name of the attribute to be added to the element

      @return
       - SUCCESS if successfully retrieve the text content of a single element
       - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
       - INVALID_XPATH if elementPath is not a well-formed XPath-expression
       - ATTRIBUTE_NOT_FOUND if the element has no attribute attributeName
       - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
       - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
   */
  DLL_EXPORT ReturnCode tixiCheckAttribute(TixiDocumentHandle handle, const char *elementPath, const char *attributeName);


  /*@}*/
  /**
    \defgroup XSL XSLT Functions
      Function to perform XSL transformations.
   */
  /*@{*/

  /**
        @brief Performs a XML transformation and saves the result to resultFilename.

        <b>Fortran syntax:</b>

        tixi_xsl_transformation_to_file( integer  handle, character*n xslFilename, character*n resultFilename, integer error )

        @cond
        #PY:#
        @endcond

        @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
        @param xslFilename (in) The name of a local file with XSL mapping informations.
        @param resultFilename (in) The name of a local file to store the resulting XML document in.
        @return
         - SUCCESS if successfully retrieve the text content of a single element
         - FAILED  is something went wrong, i.e. opening of files
     */
    DLL_EXPORT ReturnCode tixiXSLTransformationToFile(TixiDocumentHandle handle, const char *xslFilename, const char *resultFilename);




    /*@}*/
    /**
      \defgroup XPATH XPath Utility Functions
    	Function for evaluating XPath expressions.
     */
    /*@{*/

    /**
          @brief Evaluates a XPath expression and returns the number of result nodes matching this xpath expression.

          <b>Fortran syntax:</b>

          tixi_xpath_evaluate_node_number( integer handle, character*n xpathExpression, integer number, integer error )

          @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
          @param xPathExpression (in) The XPath Expression to evaluate.
          @param number (out) The number of nodes matching this xpath expression.
          @return
           - SUCCESS if successfully retrieve the text content of a single element.
           - FAILED  is an internal error occured.
       */
      DLL_EXPORT ReturnCode tixiXPathEvaluateNodeNumber(TixiDocumentHandle handle, const char *xPathExpression, int *number);


      /**
            @brief Evaluates a XPath expression and returns the text content of the resultnode matching this xpath expression.

			The element to get is selected by an index number, which should be between 1 and 'tixiXPathEvaluateNodeNumber() (inclusively)'.
			 On successful return the memory used for text is allocated
			 internally and must not be released by the user. The deallocation
			 is handle when the document referred to by handle is closed.


            <b>Fortran syntax:</b>

            tixi_xpath_expression_get_text_by_index( integer handle, character*n xpathERxpression, integer index, character*n text, integer error )

            @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
            @param xPathExpression (in) The XPath Expression to evaluate.
            @param elementNumber (in) The (index)-number to get. Needed if the XPath expression catches more than one node. Index must
             	 	 	 	 	 	 be between 1 and 'tixiXPathEvaluateNodeNumber()', inclusively.
            @param text (out) the text content of the node-value. Works for xml-elements and attributes. NULL if an error occured.
            @return
             - SUCCESS if successfully retrieve the text content of a single element.
             - INVALID_XPATH If something goes wrong on evaluating the xpath expression.
             - FAILED  is an internal error occured.
         */
        DLL_EXPORT ReturnCode tixiXPathExpressionGetTextByIndex(TixiDocumentHandle handle, const char *xPathExpression, int elementNumber, char **text);


/*@}*/
/**
  \defgroup UID UID Helper Functions
	Function for UID management.
 */
/*@{*/

  /**
	  @brief Performs a check over all UIDs and checks for duplicates.

	  <b>Fortran syntax:</b>

	  tixi_uid_check_duplicates( integer  handle, integer error )
      @cond
      #PY:#
      @endcond

	  @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
	  @return
	   - SUCCESS if all UIDs are unique
	   - FAILED if reading of uids does not work
	   - UID_NOT_UNIQUE  is there is a not unique UID
   */
  DLL_EXPORT ReturnCode tixiUIDCheckDuplicates(TixiDocumentHandle handle);


  /**
  	  @brief Performs a check over all nodes with the uID "isLink" and check if the corresponding uid exists in that data set.

  	  <b>Fortran syntax:</b>

  	  tixi_uid_check_links( integer  handle, integer error )
      @cond
        #PY:#
      @endcond

  	  @param handle (in) handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
  	  @return
  	   - SUCCESS if all links are valid
  	   - FAILED if reading of links or uids does not work
  	   - UID_NOT_UNIQUE  is there is a not unique UID
  	   - UID_LINK_BROKEN if there is a broken link.
     */
    DLL_EXPORT ReturnCode tixiUIDCheckLinks(TixiDocumentHandle handle);

  /**
  	  @brief Returns the XPath to given uID.
  	  	On successful return the memory used for text is allocated
    	internally and must not be released by the user. The deallocation
    	is handle when the document referred to by handle is closed.

  	  <b>Fortran syntax:</b>

  	  tixi_uid_get_xpath( integer  handle, character*n uid, character*n text, integer error )

  	  @param handle (in)	handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
	  @param uID (in)		The uid of which the xpath should be returned
	  @param xPath (out)	The XPath of the element with the right uid. Empty string if uid does not exist.
  	  @return
  	   - SUCCESS if all UIDs are unique
	   - UID_NOT_UNIQUE  is there is a not unique UID
  	   - FAILED  for internal error
     */
  DLL_EXPORT ReturnCode tixiUIDGetXPath(TixiDocumentHandle handle, const char *uID, char **xPath);


  /**
  	  @brief Checks if a uID exists.

  	  <b>Fortran syntax:</b>

  	  tixi_uid_check_exists( integer  handle, character*n uid, integer error )

      @cond
      #PY:#  
      @endcond

  	  @param handle (in)	handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
	  @param uID (in)		The uid which should be checked
  	  @return
  	   - SUCCESS if all UIDs are unique
  	   - UID_DONT_EXISTS if the UID do not exist in the document
   	   - UID_NOT_UNIQUE  is there is a not unique UID
  	   - FAILED  for internal error
     */
  DLL_EXPORT ReturnCode tixiUIDCheckExists(TixiDocumentHandle handle, const char *uID);


  /**
  	  @brief Sets a uID attribute to a node, specified via its XPath.

  	  <b>Fortran syntax:</b>

  	  tixi_uid_set_to_xpath( integer  handle, character*n xpath, character*n uid, integer error )
      @cond
      #PY:#
      @endcond

  	  @param handle (in)	handle as returned by ::tixiOpenDocument, ::tixiOpenDocumentRecursive, ::tixiOpenDocumentFromHTTP, ::tixiCreateDocument or ::tixiImportFromString
	  @param xPath (in)		The XPath to the node which should become the uid-attribute
	  @param uID (in)		The uid which should be checked
  	  @return
  	   - SUCCESS if all UIDs are unique
  	   - UID_DONT_EXISTS if the UID do not exist in the document
   	   - UID_NOT_UNIQUE  is there is a not unique UID
	   - INVALID_HANDLE if the handle is not valid, i.e.  does not or no longer exist
	   - INVALID_XPATH if elementPath is not a well-formed XPath-expression
	   - ELEMENT_NOT_FOUND if elementPath does not point to a node in the XML-document
	   - NO_ATTRIBUTE_NAME if attributeName is NULL
	   - ELEMENT_PATH_NOT_UNIQUE if elementPath resolves not to a single element but to a list of elements
  	   - FAILED  for internal error
     */
  DLL_EXPORT ReturnCode tixiUIDSetToXPath(TixiDocumentHandle handle, const char *xPath, const char *uID);


/*@}*/


#endif                          /* TIXI_H */


#ifdef __cplusplus
}
#endif
