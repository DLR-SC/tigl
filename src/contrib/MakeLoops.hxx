/*
* Copyright (C) 2015 German Aerospace Center (DLR/SC)
*
* This file was created by OpenCASCADE for the German Aerospace Center.
* It will reside in TiGL, until this Algorithm becomes part of
* OpenCASCADE.
*
* For now, the file will be licensed under the Apache License, Version 2.0.
*    http://www.apache.org/licenses/LICENSE-2.0
*/

#ifndef _MakeLoops_HeaderFile
#define _MakeLoops_HeaderFile

#include "tigl_internal.h"
#include "tigl.h"

#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_HArray2OfShape.hxx>
#include <NCollection_DataMap.hxx>

/**
 * \brief Return code of make loops algorithm
 * MAKELOOPS_OK : Algorithm successful
 * MAKELOOPS_FAIL_STARTINGPOINT : Algorithm failed while searching starting point
 * MAKELOOPS_FAIL_NODATA : Algorithm failed due to unsufficient data
 * MAKELOOPS_FAIL_FIRSTEDGE : Algorithm failed while searching for first edge
 * MAKELOOPS_FAIL_SECONDEDGE : Algorithm failed while searching for second edge
 * MAKELOOPS_FAIL_THIRDEDGE : Algorithm failed while searching for third edge
 * MAKELOOPS_FAIL_FOURTHEDGE : Algorithm failed while searching for fourth edge
 * MAKELOOPS_FAIL : Algorithm failed 
 */
enum MakeLoopsStatus
{
    MAKELOOPS_OK                    = 0,
    MAKELOOPS_FAIL_STARTINGPOINT    = 1,
    MAKELOOPS_FAIL_NODATA           = 2,
    MAKELOOPS_FAIL_FIRSTEDGE        = 3,
    MAKELOOPS_FAIL_SECONDEDGE       = 4,
    MAKELOOPS_FAIL_THIRDEDGE        = 5,
    MAKELOOPS_FAIL_FOURTHEDGE       = 6,
    MAKELOOPS_FAIL                  = 7
};

/**
 * Implementation of algorithm for building closed 4-sides loops
 * from grid consisting of guide and section edges
 */
class MakeLoops
{

public:

    void* operator new(size_t,void* anAddress)
    {
        return anAddress;
    }
    void* operator new(size_t size)
    {
        return Standard::Allocate(size);
    }
    void  operator delete(void* anAddress)
    {
        if (anAddress) {
            Standard::Free((Standard_Address&)anAddress);
        }
    }
// Methods PUBLIC
//

    /**
     * Empty constructop
     */
    TIGL_EXPORT MakeLoops();

    /**
     * Constructor with initialisation of data
     * @param theNetwork
     * grid consisting of guide and section (profile) edges.
     * Edges must have common (shared) vertices if they touch each other
     * @param GuidesEdges
     * Map that contains only guide edges
     * @param ProfileEdges
     * Map that contains only profile (section) edges
     */

    TIGL_EXPORT MakeLoops(const TopoDS_Shape& theNetWork,
                          const TopTools_MapOfShape& GuidesEdges,
                          const TopTools_MapOfShape& ProfileEdges);

    /**
     * Method for initialisation of data
     * @param theNetwork
     * grid consisting of guide and section (profile) edges.\n
     * Edges must have common (shared) vertices if they touch each other
     * @param GuidesEdges
     * Map that contains only guide edges
     * @param ProfileEdges
     * Map that contains only profile (section) edges
     */

    TIGL_EXPORT   void Init(const TopoDS_Shape& theNetWork,
                            const TopTools_MapOfShape& GuidesEdges,
                            const TopTools_MapOfShape& ProfileEdges);

    /**
     * Method implements algorithm of building loops
     */
    TIGL_EXPORT   void Perform() ;

    /**
     * Method returns result loops stored in 2d array\n
     * Number of colums is number of sections minus one.\n
     * Number of rows is number of guides minus one if sections are open\n
     * or number of guides if sections are closed\n
     * First index of array correspons guide edge,\n
     * second index corresponds of section edge\n
     */
    TIGL_EXPORT  const Handle(TopTools_HArray2OfShape)& Cells() const;

    /**
     * \brief Return the mapping of resulting patches to a continuity
     *        The guide and profile curves run through each of the 
     *        four patch corner points with a certain continuity (C0, C1 or C2)
     *        So each of the 4 corners has a certain continuity in guide 
     *        direction as well as in profile direction.
     *        The continuity of the patch is chosen as the highest continuity
     *        at the corners in profile or guide direction.
     *
     *        E.g.
     *
     * 
     *                             guide direction
     *                              ------------>
     *                           
     *
     * 
     *                          C2                   C2
     *                           |                   |
     *                           |                   |
     *                 C0 ------------------------------------ C2
     *                           |                   |
     *     ^                     |                   |
     *     |                     |                   |
     *     | profile             |                   |
     *     | direction           |                   |
     *     |                     |                   |
     *                           |                   |
     *                           |                   |
     *                 C0 ------------------------------------ C1
     *                           |                   |
     *                           |                   |
     *                           
     *                           C1                 C2
     *                           
     *        Here, the highest continuity is C2. Therefore the patch surface
     *        continuity should also be set to C2.
     *        
     * @returns Continuity of the resulting patch surface
     */
    TIGL_EXPORT  const NCollection_DataMap<TopoDS_Shape, TiglContinuity>& Continuities() const;

    /**
     * Returns status of calculations
     * 0 - success, 1 - algorithm failed
     */

    TIGL_EXPORT Standard_Integer GetStatus() const;



protected:

// Methods PROTECTED
//


// Fields PROTECTED
//


private:

// Methods PRIVATE
//


// Fields PRIVATE
//
    TopoDS_Shape                    myGrid;
    TopTools_MapOfShape             myGuideEdges;
    TopTools_MapOfShape             myProfileEdges;
    Handle(TopTools_HArray2OfShape) myCellGrid;
    TopTools_ListOfShape            myCells;
    Standard_Integer                myStatus;

    NCollection_DataMap<TopoDS_Shape, TiglContinuity> myContinuities;

};





// other Inline functions and methods (like "C++: function call" methods)
//


#endif
