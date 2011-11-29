# Set the occ-path first!
CASROOT=/opt/OpenCASCADE
LOCAL_INCLUDES=/develop/TIXI/include -I/develop/TIXI/local-include -I../../TIXI/Src -I/usr/include/opencascade 
LOCAL_LIBS=-L../../TIXI/lib

CXX=g++

X_INCLUDES=
GL_INCLUDES=-I/usr/include/c++/4.2.1 -I/usr/include -I$(LOCAL_INCLUDES)
QT_INCLUDES=-I/develop/qt3/include -I$(LOCAL_INCLUDES) -I$(CASROOT)/src/GeomAPI -I$(LOCAL_INCLUDES)
OCC_INCLUDES=-I$(CASROOT)/inc  -I$(CASROOT)/src/WOKTclLib -I$(LOCAL_INCLUDES)

OCC_LDFLAGS=-L$(CASROOT)/Linux/lib -L$(CASROOT)/lib -L$(LOCAL_LIBS) 

SRCDIR=.
INCDIR=$(SRCDIR)
OBJDIR=.
BINDIR=.

LIB_OCC= -lTIXI -lxml2 -lm -lPTKernel-6.3.0 -lc -ldl -lTKBO-6.3.0 -lTKBool-6.3.0 -lTKBRep-6.3.0 -lTKCAF-6.3.0 -lTKCDF-6.3.0 -lTKernel-6.3.0 -lTKFeat-6.3.0 -lTKFillet-6.3.0 -lTKG2d-6.3.0 -lTKG3d-6.3.0 -lTKGeomAlgo-6.3.0 -lTKGeomBase-6.3.0 -lTKHLR-6.3.0 -lTKIGES-6.3.0 -lTKLCAF-6.3.0 -lTKMath-6.3.0 -lTKMesh-6.3.0 -lTKOffset-6.3.0 -lTKOpenGl-6.3.0 -lTKPCAF-6.3.0 -lTKPLCAF-6.3.0 -lTKPrim-6.3.0 -lTKPShape-6.3.0 -lTKService-6.3.0 -lTKShapeSchema-6.3.0 -lTKShHealing-6.3.0 -lTKSTEP-6.3.0 -lTKSTEP209-6.3.0 -lTKSTEPAttr-6.3.0 -lTKSTEPBase-6.3.0 -lTKSTL-6.3.0 -lTKTopAlgo-6.3.0 -lTKV2d-6.3.0 -lTKV3d-6.3.0 -lTKVRML-6.3.0 -lTKXSBase-6.3.0 -lpthread 

CXXFLAGS=-DCSFDB -DHAVE_CONFIG_H -DHAVE_WOK_CONFIG_H \
	 -DLIN -DLININTEL $(OCC_INCLUDES) 
LDFLAGS=$(GL_LDFLAGS) $(OCC_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) \
	 -L/usr/X11R6/lib -L/usr/lib64 
LIBS= -lm -ldl -lc \
	$(LIB_X) $(LIB_GL) $(LIB_QT) $(LIB_OCC)

####### Files lists

OBJECTS = $(OBJDIR)/CCPACSConfigurationManager.o \
			$(OBJDIR)/CCPACSConfiguration.o \
			$(OBJDIR)/CTiglAbstractGeometricComponent.o \
			$(OBJDIR)/CTiglUIDManager.o \
			$(OBJDIR)/CCPACSFuselageConnection.o \
			$(OBJDIR)/CCPACSFuselage.o \
			$(OBJDIR)/CCPACSFuselagePositioning.o \
			$(OBJDIR)/CCPACSFuselagePositionings.o \
			$(OBJDIR)/CCPACSFuselageProfile.o \
			$(OBJDIR)/CCPACSFuselageProfiles.o \
			$(OBJDIR)/CCPACSFuselageSectionElement.o \
			$(OBJDIR)/CCPACSFuselageSectionElements.o \
			$(OBJDIR)/CCPACSFuselageSection.o \
			$(OBJDIR)/CCPACSFuselageSections.o \
			$(OBJDIR)/CCPACSFuselageSegment.o \
			$(OBJDIR)/CCPACSFuselageSegments.o \
			$(OBJDIR)/CCPACSFuselages.o \
			$(OBJDIR)/CCPACSHeader.o \
			$(OBJDIR)/CCPACSWingConnection.o \
			$(OBJDIR)/CCPACSWingComponentSegment.o \
			$(OBJDIR)/CCPACSWingComponentSegments.o \
			$(OBJDIR)/CCPACSWing.o \
			$(OBJDIR)/CCPACSWingPositioning.o \
			$(OBJDIR)/CCPACSWingPositionings.o \
			$(OBJDIR)/CCPACSWingProfile.o \
			$(OBJDIR)/CCPACSWingProfiles.o \
			$(OBJDIR)/CCPACSWingSectionElement.o \
			$(OBJDIR)/CCPACSWingSectionElements.o \
			$(OBJDIR)/CCPACSWingSection.o \
			$(OBJDIR)/CCPACSWingSections.o \
			$(OBJDIR)/CCPACSWingSegment.o \
			$(OBJDIR)/CCPACSWingSegments.o \
			$(OBJDIR)/CCPACSWings.o \
			$(OBJDIR)/CTiglAlgorithmManager.o \
			$(OBJDIR)/CTiglApproximateBsplineWire.o \
			$(OBJDIR)/CTiglError.o \
			$(OBJDIR)/CTiglInterpolateBsplineWire.o \
			$(OBJDIR)/CTiglInterpolateLinearWire.o \
			$(OBJDIR)/CTiglIntersectionCalculation.o \
			$(OBJDIR)/CTiglPoint.o \
			$(OBJDIR)/CTiglTransformation.o \
			$(OBJDIR)/CTiglExportIges.o \
			$(OBJDIR)/CTiglExportStl.o \
			$(OBJDIR)/CTiglExportVtk.o \
			$(OBJDIR)/tigl_fortran.o \
			$(OBJDIR)/tigl.o \

TARGET=$(BINDIR)/libTIGL.a

all: $(TARGET)

$(TARGET): $(OBJDIR) $(BINDIR) $(OBJECTS)
	   ar rc libTIGL.a *.o
	   ranlib libTIGL.a

clean:
	@rm -rf $(TARGET) $(OBJECTS) $(SRCMOC) $(BINDIR)/core core
