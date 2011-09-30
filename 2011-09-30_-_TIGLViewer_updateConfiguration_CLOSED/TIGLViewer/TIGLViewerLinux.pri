CONFIG(release, release|debug) { 
	message ("Building Release Version") 
	LIBS += -L$(CASROOT)/Linux/lib -lBinLPlugin \
		-lTIXI \
		-lTIGL \
		-lBinPlugin \
		-lBinXCAFPlugin \
		-lFWOSPlugin \
		-lPTKernel \
		-lStdLPlugin \
		-lStdPlugin \
		-lTKAdvTools \
		-lTKBin \
		-lTKBinL \
		-lTKBinXCAF \
		-lTKBO \
		-lTKBool \
		-lTKBRep \
		-lTKCAF \
		-lTKCDF \
		-lTKernel \
		-lTKFeat \
		-lTKFillet \
		-lTKG2d \
		-lTKG3d \
		-lTKGeomAlgo \
		-lTKGeomBase \
		-lTKHLR \
		-lTKIGES \
		-lTKLCAF \
		-lTKMath \
		-lTKMesh \
		-lTKMeshVS \
		-lTKOffset \
		-lTKOpenGl \
		-lTKPCAF \
		-lTKPLCAF \
		-lTKPrim \
		-lTKPShape \
		-lTKService \
		-lTKShapeSchema \
		-lTKShHealing \
		-lTKStdLSchema \
		-lTKStdSchema \
		-lTKSTEP \
		-lTKSTEP209 \
		-lTKSTEPAttr \
		-lTKSTEPBase \
		-lTKSTL \
		-lTKTopAlgo \
		-lTKV2d \
		-lTKV3d \
		-lTKVRML \
		-lTKXCAF \
		-lTKXCAFSchema \
		-lTKXDEIGES \
		-lTKXDESTEP \
		-lTKXml \
		-lTKXmlL \
		-lTKXmlXCAF \
		-lTKXSBase \
		-lXCAFPlugin \
		-lXmlLPlugin \
		-lXmlPlugin \
		-lXmlXCAFPlugin
#		-ltk -lXmu
}
#################################################################
# Change the lib to libd if you have a debug OpenCASCADE build.	#
#################################################################	
CONFIG(debug, release|debug) { 
	message ("Building Debug Version") 
	LIBS += -L$(CASROOT)/Linux/lib -lBinLPlugin \
		-lTIGL \
		-lTIXI \
		-lBinPlugin \
		-lBinXCAFPlugin \
		-lFWOSPlugin \
		-lPTKernel \
		-lStdLPlugin \
		-lStdPlugin \
		-lTKAdvTools \
		-lTKBin \
		-lTKBinL \
		-lTKBinXCAF \
		-lTKBO \
		-lTKBool \
		-lTKBRep \
		-lTKCAF \
		-lTKCDF \
		-lTKernel \
		-lTKFeat \
		-lTKFillet \
		-lTKG2d \
		-lTKG3d \
		-lTKGeomAlgo \
		-lTKGeomBase \
		-lTKHLR \
		-lTKIGES \
		-lTKLCAF \
		-lTKMath \
		-lTKMesh \
		-lTKMeshVS \
		-lTKOffset \
		-lTKOpenGl \
		-lTKPCAF \
		-lTKPLCAF \
		-lTKPrim \
		-lTKPShape \
		-lTKService \
		-lTKShapeSchema \
		-lTKShHealing \
		-lTKStdLSchema \
		-lTKStdSchema \
		-lTKSTEP \
		-lTKSTEP209 \
		-lTKSTEPAttr \
		-lTKSTEPBase \
		-lTKSTL \
		-lTKTopAlgo \
		-lTKV2d \
		-lTKV3d \
		-lTKVRML \
		-lTKXCAF \
		-lTKXCAFSchema \
		-lTKXDEIGES \
		-lTKXDESTEP \
		-lTKXml \
		-lTKXmlL \
		-lTKXmlXCAF \
		-lTKXSBase \
		-lXCAFPlugin \
		-lXmlLPlugin \
		-lXmlPlugin \
		-lXmlXCAFPlugin
#		-ltk -lXmu
}
		
