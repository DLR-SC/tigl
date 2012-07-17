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

LIB_OCC= -lTIXI -lxml2 -lm -lPTKernel -lc -ldl -lTKBO -lTKBool -lTKBRep -lTKCAF -lTKCDF -lTKernel -lTKFeat -lTKFillet -lTKG2d -lTKG3d -lTKGeomAlgo -lTKGeomBase -lTKHLR -lTKIGES -lTKLCAF -lTKMath -lTKMesh -lTKOffset -lTKOpenGl -lTKPCAF -lTKPLCAF -lTKPrim -lTKPShape -lTKService -lTKShapeSchema -lTKShHealing -lTKSTEP -lTKSTEP209 -lTKSTEPAttr -lTKSTEPBase -lTKSTL -lTKTopAlgo -lTKV2d -lTKV3d -lTKVRML -lTKXSBase -lpthread 

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
