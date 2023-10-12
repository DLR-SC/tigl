#pragma once
#include "gmsh.h"
#include <string>
#include "TopoDS_Shape.hxx"
#include <unordered_map>
#include <map>

namespace tigl {


class Mesher 
{
public: 

    struct FieldOptions 
    {
        double min_element_size;
        double field_number;
    };
    
    struct Options
    {
        int dim; 

        double minelementsize;
        double minelementsize2;
        double minelementsize3;
        double maxelementsize;
        double mindistance;
        double maxdistance;

        std::map<std::string, FieldOptions> field_options;

    };

    

    Mesher();
    ~Mesher();
    
    FieldOptions& get_field_options(std::string const& field_name);

    void set_dimension(int dimension);
    void set_minMeshSize(double min);
    void set_maxMeshSize(double max);
    void set_minDistance(double min_dist);
    void set_maxDistance(double max_dist);
    void mesh() const;
    void import(TopoDS_Shape shape) const;

    void refine() const;
    void write(std::string myName) const;

    void refineEdge( std::vector<double> Edge, std::string field_name);
    void refineSurface( std::vector<double> Surface, std::string field_name);
    void mesh_fields() const;

    std::vector<double> Fields;
    double num = 1;

    Options options;
    
};
}