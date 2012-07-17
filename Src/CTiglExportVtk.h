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
* @brief  Export routines for CPACS configurations.
*/

#ifndef CTIGLEXPORTVTK_H
#define CTIGLEXPORTVTK_H

#include <vector>
#include <map>

#include "CCPACSWings.h"
#include "CCPACSWingProfile.h"
#include "CCPACSFuselages.h"
#include "CCPACSFuselageProfile.h"
#include "ITiglGeometricComponent.h"

#include "Precision.hxx"


namespace tigl {

    // Comparer for gp_Pnts
    struct gp_PntEquals
    {
        bool operator()(const gp_Pnt lhs, const gp_Pnt rhs) const
        {
            return !lhs.IsEqual(rhs, Precision::Confusion());   // must return "isLower"
        }
    };

	typedef std::map<gp_Pnt, unsigned int, gp_PntEquals> PointMapType;

	enum VTK_EXPORT_MODE
        {
        TIGL_VTK_SIMPLE = 0,
        TIGL_VTK_COMPLEX  = 1
        };

	class CTiglExportVtk
	{


	public:
		// Constructor
	    CTiglExportVtk(CCPACSConfiguration & config);

		// Virtual Destructor
		virtual ~CTiglExportVtk(void);

        // Exports a by index selected wing, boolean fused and meshed, as VTK file
        void ExportMeshedWingVTKByIndex(const int wingIndex, const std::string& filename, const double deflection = 0.1);

        // Exports a by UID selected wing, boolean fused and meshed, as VTK file
        void ExportMeshedWingVTKByUID(const std::string wingUID, const std::string& filename, const double deflection = 0.1);

        // Exports a by index selected fuselage, boolean fused and meshed, as VTK file
        void ExportMeshedFuselageVTKByIndex(const int fuselageIndex, const std::string& filename, const double deflection = 0.1);

        // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file
        void ExportMeshedFuselageVTKByUID(const std::string fuselageUID, const std::string& filename, const double deflection = 0.1);

        // Exports a whole geometry, boolean fused and meshed, as VTK file
        void ExportMeshedGeometryVTK(const std::string& filename, const double deflection = 0.1);


        // Simple exports without cpacs information
        // Exports a by UID selected wing, meshed, as VTK file
		// No additional information are computed.
        void ExportMeshedWingVTKSimpleByUID(const std::string wingUID, const std::string& filename, const double deflection = 0.1);

        // Exports a by UID selected fuselage, boolean fused and meshed, as VTK file.
		// No additional information are computed.
        void ExportMeshedFuselageVTKSimpleByUID(const std::string fuselageUID, const std::string& filename, const double deflection = 0.1);

        // Exports a whole geometry, boolean fused and meshed, as VTK file
		// No additional information are computed.
        void ExportMeshedGeometryVTKSimple(const std::string& filename, const double deflection = 0.1);


    private:
		CCPACSConfiguration & myConfig;       /**< TIGL configuration object */

	};


    class VtkWriter
    {


    public:
        // Constructor
        VtkWriter(CCPACSConfiguration & config);

        // No destructor needed
        ~VtkWriter(void) {}

        // Write out the triangulation into a XML-VTK polydata file (*.vtp)
        void Write(const TopoDS_Shape & shape, ITiglGeometricComponent & component, const std::string& filename, const double deflection = 0.01, VTK_EXPORT_MODE mode = TIGL_VTK_SIMPLE);

    private:
        // Finds unique points and assign them to a segment
        unsigned int FindUniquePoints(const TopoDS_Shape & shape, ITiglGeometricComponent & component, const double deflection);

        // Build simple and fast triangulation
        unsigned int SimpleTriangulation(const TopoDS_Shape & shape, ITiglGeometricComponent & component, const double deflection);


        void FindOrCreatePointIndex(std::vector<unsigned int> & list, gp_Pnt point);
        void SetMinValue(double& old, const gp_Pnt& point);
        void SetMaxValue(double& old, const gp_Pnt& point);

        // computes the normal vector of a triangle
        gp_Vec FindNormal(const gp_Pnt P1, const gp_Pnt P2, const gp_Pnt P3);

    private:
		CCPACSConfiguration & myConfig;                         /**< TIGL configuration object                                                  */
        gp_Pnt* pointArray;                                     /**< contains all points already added to a triangle                            */
        PointMapType pointMap;								/**< contains a point index for all points added                                */
        std::vector<unsigned int> triangleList1;          /**< contains point-references per triangle                                     */
        std::vector<unsigned int> triangleList2;          /**< contains point-references per triangle                                     */
        std::vector<unsigned int> triangleList3;          /**< contains point-references per triangle                                     */
        std::vector<std::string> triangleUID;             /**< UIDs of the part the triangle belongs to                                   */
        std::vector<unsigned int> triangleSegment;        /**< segment no. (1..) of the segment the triangle belongs to                   */
        std::vector<double> triangleEta;                  /**< triangle eta of triangle (from root to tip 0..1)                           */
        std::vector<double> triangleXsi;                  /**< triangle xsi of triangle (from leading edge to trailing edge 0..1)         */
        std::vector<int> triangleOnTop;                  /**< indicates if the triangle is located on the top or the bottom of the wing  */
        double pointsMin;                                       /**< contains lowest point value                                                */
        double pointsMax;                                       /**< contains highest point value                                               */

    };

} // end namespace tigl

#endif // CTIGLEXPORTVTK_H
