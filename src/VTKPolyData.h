#ifndef VTKPOLYDATA_H
#define VTKPOLYDATA_H

#include <vector>
#include <map>

inline double SQR(double x){ return x*x; }

class Point{
public:
    Point(){
        x = 0.; y = 0.; z = 0.;
    }

    Point(int X,int Y,int Z){
        x = X; y = Y; z = Z;
    }
    double dist2(const Point& p) const {
        return SQR(x-p.x) + SQR(y-p.y) + SQR(z-p.z);
    }

    void print()const;

    void getMinMax(double & min, double & max) const;

    double x,y,z;
};

struct Polygon{
    Polygon(int num){ myid = num; }

    void addPoint(int index);

    void repair();

    int getNVert(){ return pindex.size();}
    int getPointIndex(int i) { return pindex.at(i); }

private:
    std::vector<int> pindex;

private:
    int myid;
};

// Comparer for gp_Pnts
struct gp_PntEquals
{

    // function should only return true, if lhs < rhs
    bool operator()(const Point lhs, const Point rhs) const
    {

        // if two points lie inside the epsilon environment
        // they should be the same, hence lhs is not smaller than rhs
        if(lhs.dist2(rhs) < 1e-10 )
            return false;


        // as lhs and rhs don't lie inside an epsilon environment
        if(lhs.x < rhs.x)
            return true;
        else if (lhs.x == rhs.x) {
            if(lhs.y < rhs.y)
                return true;
            else if(lhs.y == rhs.y)
                return lhs.z < rhs.z;
            else
                return false;
        }
        else
            return false;
    }
};

typedef std::map<Point, unsigned int, gp_PntEquals> PointMap;

class VTKPolyData{
public:
    VTKPolyData(){
        pointlist.clear();
        polys.clear();
        min_coord = DBL_MAX;
        max_coord = DBL_MIN;
    }
    void addPoint(const Point& p, int polynum);

    void write(const char * filename);
    void print();

private:
    void write_stream(std::ostream&);
    double min_coord;
    double max_coord;
    PointMap pointlist;
    std::vector<Polygon> polys;
};

#endif // VTKPOLYDATA_H
