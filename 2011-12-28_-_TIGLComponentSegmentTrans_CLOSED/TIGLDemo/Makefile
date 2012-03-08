# Set the path first!
CASROOT=/DLR/TIVA/OpenCASCADE6.1.0/ros
LOCAL_INCLUDES=/DLR/TIVA/local/include
LOCAL_LIBS=/DLR/TIVA/local/lib


CC=cc
CXX=gcc
MOC=/DLR/TIVA/qt3/bin/moc
MSG2QM=/usr/bin/msg2qm

X_INCLUDES=
GL_INCLUDES=-I/usr/include -I$(LOCAL_INCLUDES)
QT_INCLUDES=-I/DLR/TIVA/qt3/include -I$(LOCAL_INCLUDES) -I$(CASROOT)/src/GeomAPI -I$(LOCAL_INCLUDES)
OCC_INCLUDES=-I$(CASROOT)/inc  -I$(CASROOT)/src/WOKTclLib -I$(LOCAL_INCLUDES)

X_LDFLAGS=-L/usr/X11R6/lib 
GL_LDFLAGS=-L/usr/lib -L$(LOCAL_LIBS)
QT_LDFLAGS=-L/DLR/TIVA/qt3/lib
OCC_LDFLAGS=-L$(CASROOT)/Linux/lib 

STATION=Linux
SRCDIR=.
RESDIR=.
INCDIR=$(SRCDIR)
MOCDIR=.
OBJDIR=.
BINDIR=.

LIB_OCC= -lTKernel -lTKMath -lTKG2d -lTKG3d -lTKGeomBase -lTKBRep -lTKGeomAlgo -lTKTopAlgo \
	 -lTKPrim -lTKBool -lTKFeat -lTKFillet -lTKOffset -lTKHLR \
	 -lTKService -lTKV2d -lTKV3d -lTKPCAF -lTKCDF -lTKCAF \
         -lPTKernel -lTKIGES -lTKSTEP -lTKSTL -lTKVRML -lTKShHealing \
         -lTKXSBase -lTKPShape -lTKShapeSchema -lTKOpenGl  -lTKBO  \
	 -lTKBool -lTKTopAlgo -lTKPrim -lTKOffset -lTKFillet -lTKBO -lTIXI -lTIXIf -lTIGL -lxml2

CFLAGS=-DCSFDB -DNO_CXX_EXCEPTION -DNo_Exception \
	 -DLIN -DLININTEL  \
	$(GL_INCLUDES) $(OCC_INCLUDES) $(QT_INCLUDES) $(X_INCLUDES)
CXXFLAGS=-DCSFDB -DHAVE_CONFIG_H -DHAVE_WOK_CONFIG_H -Wall -g \
	 -DLIN -DLININTEL  $(GL_INCLUDES) $(OCC_INCLUDES) $(QT_INCLUDES) $(X_INCLUDES)
LDFLAGS=$(GL_LDFLAGS) $(OCC_LDFLAGS) $(QT_LDFLAGS) $(X_LDFLAGS) \
	 -L/usr/X11R6/lib
LIBS= -lm -ldl -lc \
	$(LIB_OCC)

####### Files lists

SOURCES = $(SRCDIR)/*.c

OBJECTS = $(OBJDIR)/tiglDemo.o 


SRCMOC = $(MOCDIR)/moc_Application.cxx \
	 $(MOCDIR)/moc_Document.cxx \
	 $(MOCDIR)/moc_MDIWindow.cxx \
	 $(MOCDIR)/moc_Material.cxx \
	 $(MOCDIR)/moc_Transparency.cxx \
	 $(MOCDIR)/moc_View.cxx

TARGET=$(BINDIR)/tiglDemo

all: $(TARGET)


moc: $(SRCMOC)

msg: $(MESSAGE)

$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGET) $(TARGET).o $(LDFLAGS) $(LIBS)

clean:
	@rm -rf $(TARGET) $(OBJECTS) $(SRCMOC) $(BINDIR)/core core

$(OBJDIR)/Application.o: 	$(SRCDIR)/*.h
	$(CXX) $(SRCDIR)/*.c -c $(CXXFLAGS) 
