/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2013-12-17 Tobias Stollenwerk <Tobias.Stollenwerk@dlr.de>
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
* @brief  Implementation of CPACS wing profile as a CST profile
*
* The wing profile is defined by the (C)lass function / (S)hape function (T)ransformation
* geometry representation method.
*/

#include "CTiglError.h"
#include "CTiglLogging.h"
#include "CTiglTransformation.h"
#include "math.h"

#include "gp_Pnt2d.hxx"
#include "gp_Vec2d.hxx"
#include "gp_Dir2d.hxx"
#include "gp_Pln.hxx"
#include "Bnd_Box.hxx"
#include "Geom2d_Line.hxx"
#include "Geom2d_TrimmedCurve.hxx"
#include "TopoDS.hxx"
#include "TopExp_Explorer.hxx"
#include "TopAbs_ShapeEnum.hxx"
#include "TopoDS_Edge.hxx"
#include "GCE2d_MakeSegment.hxx"
#include "BRep_Tool.hxx"
#include "BRepAdaptor_CompCurve.hxx"
#include "Geom2dAPI_InterCurveCurve.hxx"
#include "GeomAPI_ProjectPointOnCurve.hxx"
#include "GeomAPI.hxx"
#include "gce_MakeDir.hxx"
#include "gce_MakePln.hxx"
#include "BRepTools_WireExplorer.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBndLib.hxx"
#include "ShapeFix_Wire.hxx"
#include "math/tiglmathfunctions.h"
#include "CTiglInterpolateBsplineWire.h"
#include "CTiglInterpolateLinearWire.h"
#include "ITiglWingProfileAlgo.h"
#include "CCPACSWingProfileCST.h"
#include <algorithm>

namespace tigl
{
    // helper function to read in tixi vector
    std::vector<double> readTixiVector (TixiDocumentHandle tixiHandle, const std::string& xpath, const std::string& name, const std::string& funcname)
    {
        std::string fullpath = xpath + name;
        int count;
        if (tixiGetVectorSize(tixiHandle, fullpath.c_str(), &count) != SUCCESS)
        {
            throw CTiglError("Error: XML error while reading vector <" + name + "> in " + funcname, TIGL_XML_ERROR);
        }
        double *tmp;
        if (tixiGetFloatVector(tixiHandle, fullpath.c_str(), &tmp, count) != SUCCESS) 
        {
            throw CTiglError("Error: XML error while reading vector <" + name + "> in " + funcname, TIGL_XML_ERROR);
        }
        std::vector<double> res(tmp, tmp+count);
        return res;
    }
    // helper function to read in tixi double
    double readTixiDouble (TixiDocumentHandle tixiHandle, const std::string& xpath, const std::string& name, const std::string& funcname)
    {
        std::string fullpath = xpath + name;
        double res;
        if (tixiGetDoubleElement(tixiHandle, fullpath.c_str(), &res) != SUCCESS)
        {
            throw CTiglError("Error: XML error while reading double <" + name + "> in " + funcname, TIGL_XML_ERROR);
        }
        return res;
    }

    // CST curve
    double class_function(const double& N1, const double& N2, const double& x)
    {
        return pow(x,N1) * pow(1-x,N2);
    }
    double shape_function(const std::vector<double>& B, const double& x)
    {
        double ret = 0.;
        int order = B.size()-1;
        int i = 0;
        for (std::vector<double>::const_iterator bIT = B.begin(); bIT != B.end(); ++bIT, ++i) {
            ret += *bIT * bernstein_poly(i, order, x);
        }
        return ret;
    }
    double cstcurve(const double& x, const double& N1, const double& N2, const std::vector<double>& B)
    {
            return class_function(N1, N2, x) * shape_function(B, x);
    }


    // Constructor
    CCPACSWingProfileCST::CCPACSWingProfileCST(const std::string& path)
    {
        ProfileDataXPath=path;
        profileWireAlgo = WireAlgoPointer(new CTiglInterpolateBsplineWire);
    }

    // Destructor
    CCPACSWingProfileCST::~CCPACSWingProfileCST(void)
    {
        delete profileWireAlgo;
    }

    // Cleanup routine
    void CCPACSWingProfileCST::Cleanup(void)
    {
        for (CCPACSCoordinateContainer::size_type i = 0; i < coordinates.size(); i++)
        {
            delete coordinates[i];
        }
        coordinates.clear();
        for (CCPACSCoordinateContainer::size_type i = 0; i < upperCoordinates.size(); i++)
        {
            delete upperCoordinates[i];
        }
        upperCoordinates.clear();
        for (CCPACSCoordinateContainer::size_type i = 0; i < lowerCoordinates.size(); i++)
        {
            delete lowerCoordinates[i];
        }
        lowerCoordinates.clear();

        psi.clear();
        upperB.clear();
        lowerB.clear();
    }

    // Read wing profile file
    void CCPACSWingProfileCST::ReadCPACS(TixiDocumentHandle tixiHandle)
    {
        psi=readTixiVector(tixiHandle, ProfileDataXPath, "/psi", "CCPACSWingProfileCST::ReadCPACS"); 
        upperB=readTixiVector(tixiHandle, ProfileDataXPath, "/upperB", "CCPACSWingProfileCST::ReadCPACS"); 
        lowerB=readTixiVector(tixiHandle, ProfileDataXPath, "/lowerB", "CCPACSWingProfileCST::ReadCPACS"); 
        upperN1=readTixiDouble(tixiHandle, ProfileDataXPath, "/upperN1", "CCPACSWingProfileCST::ReadCPACS"); 
        upperN2=readTixiDouble(tixiHandle, ProfileDataXPath, "/upperN2", "CCPACSWingProfileCST::ReadCPACS"); 
        lowerN1=readTixiDouble(tixiHandle, ProfileDataXPath, "/lowerN1", "CCPACSWingProfileCST::ReadCPACS"); 
        lowerN2=readTixiDouble(tixiHandle, ProfileDataXPath, "/lowerN2", "CCPACSWingProfileCST::ReadCPACS"); 

        // create sample points on the wing profile with x-coordinate given by psi
        // make sure, that psi has 0.0 at beginning and 1.0 at end and is bounded by 0.0 and 1.0
        if (psi.size()<=2)
        {
            // ignore psi and set 0.0 and 1.0
            psi.clear(),
            psi.push_back(0.0);
            psi.push_back(1.0);
        }
        else
        {
            // sort psi
            std::sort(psi.begin(), psi.end());
            // get maximal psi value 
            double psimax=psi[psi.size()-1];
            // get minimal psi value 
            double psimin=psi[0];
            if (psimax>1.0)
            {
                throw CTiglError("Error: CCPACSWingProfileCST::ReadCPACS: Psi values greater than 1.0", TIGL_ERROR);
            }
            // force end value of 1.0
            else if (psimax<1.0-Precision::Confusion())
            {
                psi.push_back(1.0);
            }
            if (psimin<0.0)
            {
                throw CTiglError("Error: CCPACSWingProfileCST::ReadCPACS: Negative psi values", TIGL_ERROR);
            }
            // force start value of 0.0
            else if (psimin>0.0+Precision::Confusion())
            {
                psi.insert(psi.begin(),0.0);
            }
        }
        // get upper and lower sample points
        CTiglPoint* point;
        for (size_t i = 0; i<psi.size(); i++)
        {
            point = new CTiglPoint(psi[i], 0.0, cstcurve(psi[i], upperN1, upperN2, upperB));
            upperCoordinates.push_back(point);
            point = new CTiglPoint(psi[i], 0.0, -cstcurve(psi[i], lowerN1, lowerN2, lowerB));
            lowerCoordinates.push_back(point);
        }

        // Loop forwards over lower CST points (leave out last point)
        for (size_t i = psi.size()-1; i > 0; i--)
        {
            coordinates.push_back(upperCoordinates[i]);
        }
        // Loop forwards over lower CST points
        for (size_t i = 0; i < psi.size()-1; i++)
        {
            coordinates.push_back(lowerCoordinates[i]);
        }
        ofstream out;
        out.open("cst.dat");
        for (size_t i = 0; i < coordinates.size(); i++)
        {
            out <<  coordinates[i]->x << "\t" << coordinates[i]->z << endl;
        }
        out.close();
    }

    // Builds the wing profile wire. The returned wire is already transformed by the
    // wing profile element transformation.
    void CCPACSWingProfileCST::BuildWires()
    {
        // Set wire algorithm
        const ITiglWireAlgorithm& wireBuilder = *profileWireAlgo;

        // CCPACSWingSegment::makeSurfaces cannot handle currently 
        // wire with multiple edges. Thus we get problems if we have
        // a linear interpolated wire consting of many edges.
        if(dynamic_cast<const CTiglInterpolateLinearWire*>(&wireBuilder))
        {
            LOG(ERROR) << "Linear Wing Profiles are currently not supported" << endl;
            throw CTiglError("Linear Wing Profiles are currently not supported",TIGL_ERROR);
        }

        // Build closed wire
        ITiglWireAlgorithm::CPointContainer points;
        for (size_t i = 0; i < coordinates.size(); i++)
        {
            points.push_back(coordinates[i]->Get_gp_Pnt());
        }

        wireClosed   = wireBuilder.BuildWire(points, true);
        if (wireClosed.IsNull() == Standard_True)
        {
            throw CTiglError("Error: TopoDS_Wire wireClosed is null in CCPACSWingProfileCST::BuildWire", TIGL_ERROR);
        }
        
        /*
        // direct building of upper and lower wires needs continuous derivative across the leading and trailing 
        // edge points. -> Need to modify profileWireAlgo
        // Build upper wire
        ITiglWireAlgorithm::CPointContainer upperPoints;
        for (size_t i = 0; i < upperCoordinates.size(); i++)
        {
            upperPoints.push_back(upperCoordinates[i]->Get_gp_Pnt());
        }
        upperWire   = wireBuilder.BuildWire(upperPoints, false);
        if (upperWire.IsNull() == Standard_True)
        {
            throw CTiglError("Error: TopoDS_Wire upperWire is null in CCPACSWingProfileCST::BuildWire", TIGL_ERROR);
        }

        // Build lower wire
        ITiglWireAlgorithm::CPointContainer lowerPoints;
        for (size_t i = 0; i < lowerCoordinates.size(); i++)
        {
            lowerPoints.push_back(lowerCoordinates[i]->Get_gp_Pnt());
        }
        lowerWire   = wireBuilder.BuildWire(lowerPoints, false);
        if (lowerWire.IsNull() == Standard_True)
        {
            throw CTiglError("Error: TopoDS_Wire lowerWire is null in CCPACSWingProfileCST::BuildWire", TIGL_ERROR);
        }
        */

        // indirect building of upper and lower wires
        // Get BSpline curve
        Handle_Geom_BSplineCurve curve = BRepAdaptor_CompCurve(wireClosed).BSpline();
        // Get Leading edge parameter on curve
        double lep_par = GeomAPI_ProjectPointOnCurve(lePoint, curve).LowerDistanceParameter();
        
        // upper and lower edges
        TopoDS_Edge upperEdge, lowerEdge;
        upperEdge = BRepBuilderAPI_MakeEdge(curve,curve->FirstParameter(), lep_par);
        lowerEdge = BRepBuilderAPI_MakeEdge(curve,lep_par, curve->LastParameter());

        // Leading/trailing edge points
        gp_Pnt te_up = curve->StartPoint();
        gp_Pnt te_down = curve->EndPoint();

        // Wire builder
        BRepBuilderAPI_MakeWire upperWireBuilder, lowerWireBuilder;

        //check if we have to close upper and lower wing shells
        if(te_up.Distance(te_down) > Precision::Confusion())
        {
            lowerWireBuilder.Add(BRepBuilderAPI_MakeEdge(te_up,te_down));
        }

        upperWireBuilder.Add(upperEdge); 
        lowerWireBuilder.Add(lowerEdge);
        
        upperWire = upperWireBuilder.Wire();
        lowerWire = lowerWireBuilder.Wire();

    }

    // Builds leading and trailing edge points of the wing profile wire.
    void CCPACSWingProfileCST::BuildLETEPoints(void)
    {
        lePoint = upperCoordinates[0]->Get_gp_Pnt();
        tePoint = lowerCoordinates[lowerCoordinates.size()-1]->Get_gp_Pnt();
    }

    // Returns sample points
    std::vector<CTiglPoint*> CCPACSWingProfileCST::GetSamplePoints() const
    {
        return coordinates;
    }

    // get profiles CPACS XML path
    const std::string & CCPACSWingProfileCST::GetProfileDataXPath()
    {
        return ProfileDataXPath;
    }

    // get forced closed wing profile wire
    const TopoDS_Wire & CCPACSWingProfileCST::GetWireClosed()
    {
        return wireClosed;
    }
        
    // get upper wing profile wire
    const TopoDS_Wire & CCPACSWingProfileCST::GetUpperWire()
    {
        return upperWire;
    }
            
    // get lower wing profile wire
    const TopoDS_Wire & CCPACSWingProfileCST::GetLowerWire()
    {
        return lowerWire;
    }

    // get leading edge point();
    const gp_Pnt & CCPACSWingProfileCST::GetLEPoint()
    {
        return lePoint;
    }
        
    // get trailing edge point();
    const gp_Pnt & CCPACSWingProfileCST::GetTEPoint()
    {
        return tePoint;
    }

} // end namespace tigl
