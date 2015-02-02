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

#include <TopoDS_Shape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <TopTools_HArray2OfShape.hxx>


#ifndef _Standard_HeaderFile
#include <Standard.hxx>
#endif
#ifndef _Standard_Macro_HeaderFile
#include <Standard_Macro.hxx>
#endif

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
    Standard_EXPORT MakeLoops();

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

    Standard_EXPORT MakeLoops(const TopoDS_Shape& theNetWork,
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

    Standard_EXPORT   void Init(const TopoDS_Shape& theNetWork,
                                const TopTools_MapOfShape& GuidesEdges,
                                const TopTools_MapOfShape& ProfileEdges);

    /**
     * Method implements algorithm of building loops
     */
    Standard_EXPORT   void Perform() ;

    /**
     * Method returns result loops stored in 2d array\n
     * Number of colums is number of sections minus one.\n
     * Number of rows is number of guides minus one if sections are open\n
     * or number of guides if sections are closed\n
     * First index of array correspons guide edge,\n
     * second index corresponds of section edge\n
     */
    Standard_EXPORT  const Handle(TopTools_HArray2OfShape)& Cells() const;





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


};





// other Inline functions and methods (like "C++: function call" methods)
//


#endif
