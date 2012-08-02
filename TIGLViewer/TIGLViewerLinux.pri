CONFIG(release, release|debug) {
    message ("Building Release Version")
    LIBS += -L$(CASROOT)/Linux/lib \
        -lTIXI \
        -lTIGL \
        -lPTKernel \
        -lTKAdvTools \
        -lTKBin \
        -lTKBinL \
        -lTKBinXCAF \
        -lTKBO \
        -lTKBool \
        -lTKBRep \
        -lTKernel \
        -lTKFeat \
        -lTKGeomAlgo \
        -lTKGeomBase \
        -lTKIGES \
        -lTKLCAF \
        -lTKMath \
        -lTKOpenGl \
        -lTKPrim \
        -lTKPShape \
        -lTKService \
        -lTKShapeSchema \
        -lTKShHealing \
        -lTKStdLSchema \
        -lTKStdSchema \
        -lTKSTL \
        -lTKTopAlgo \
        -lTKV2d \
        -lTKV3d \
        -lTKVRML \
        -lTKXCAF \
        -lTKXCAFSchema \
        -lTKXDEIGES \
        -lTKXDESTEP \
        -lTKXmlXCAF \
        -lTKXSBase

    LIBS += -lxml2 -lxslt -lcurl

# OCC libs not needed but were linked before

#        -lFWOSPlugin \
#        -lTKCAF \
#        -lTKCDF \
#        -lTKHLR \
#        -lTKXml \
#        -lTKXmlL \
#        -lTKSTEP \
#        -lTKSTEP209 \
#        -lTKSTEPAttr \
#        -lTKSTEPBase \
#        -lTKPCAF \
#        -lTKPLCAF \
#        -lTKMesh \
#        -lTKMeshVS \
#        -lTKOffset
#        -lTKFillet \
#        -lTKG2d \
#        -lTKG3d \
}
#################################################################
# Change the lib to libd if you have a debug OpenCASCADE build.	#
#################################################################	
CONFIG(debug, release|debug) { 
	message ("Building Debug Version") 
    LIBS += -L$(CASROOT)/Linux/lib \
        -lTIXI \
        -lTIGL \
        -lPTKernel \
        -lTKAdvTools \
        -lTKBin \
        -lTKBinL \
        -lTKBinXCAF \
        -lTKBO \
        -lTKBool \
        -lTKBRep \
        -lTKernel \
        -lTKFeat \
        -lTKGeomAlgo \
        -lTKGeomBase \
        -lTKIGES \
        -lTKLCAF \
        -lTKMath \
        -lTKOpenGl \
        -lTKPrim \
        -lTKPShape \
        -lTKService \
        -lTKShapeSchema \
        -lTKShHealing \
        -lTKStdLSchema \
        -lTKStdSchema \
        -lTKSTL \
        -lTKTopAlgo \
        -lTKV2d \
        -lTKV3d \
        -lTKVRML \
        -lTKXCAF \
        -lTKXCAFSchema \
        -lTKXDEIGES \
        -lTKXDESTEP \
        -lTKXmlXCAF \
        -lTKXSBase

    LIBS += -lxml2 -lxslt -lcurl
}
		
