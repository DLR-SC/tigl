#include <iostream>
#include <sys/time.h>

#include <BRepPrimAPI_MakeTorus.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>


int main()
{
	std::cout << "Torus: R=6.0, r=3.0" <<std::endl;

	timeval tstart, tstop;
	double processTime;

	gettimeofday(&tstart, NULL);

	BRepPrimAPI_MakeTorus aTorus(6.0, 3.0);

	GProp_GProps aSystemV, aSystemS;
	BRepGProp::VolumeProperties (aTorus.Shape(), aSystemV);
	BRepGProp::SurfaceProperties(aTorus.Shape(), aSystemS);

	Standard_Real aVolume  = aSystemV.Mass();
	Standard_Real aSurface = aSystemS.Mass();

	gettimeofday(&tstop, NULL);

	std::cout <<"Volume  value: "<< aVolume  <<std::endl;
	std::cout <<"Surface value: "<< aSurface <<std::endl;

	processTime =  (tstop.tv_sec  - tstart.tv_sec)  * 1000.0;      
	processTime += (tstop.tv_usec - tstart.tv_usec) / 1000.0;  

	std::cout<<"Process time: "<< processTime << "ms\n";

	return 0;
}
