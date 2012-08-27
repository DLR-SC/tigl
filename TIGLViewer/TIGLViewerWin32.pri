# TIGLViewer has to link against the static lib
# as there are no exports for the direct tigl class access

CONFIG(release, release|debug) { 
	message ("Building Release Version") 
	CASLIBS = $(CASROOT)/win32/vc10/lib
	LIBS +=  \
		../Src/Release/TIGL.lib \
		../../tixi-buildWin32/lib/Release/TIXI.lib \
}
		
CONFIG(debug, release|debug) { 	
	message ("Building Debug Version") 
	CASLIBS = $(CASROOT)/win32/vc10/libd
		LIBS +=  \
		../Src/Debug/TIGL.lib \
		../../tixi-buildWin32/lib/Debug/TIXI.lib \
}	

LIBS += \
		$(TIXI_LIB_SHARED_GCC_VC7)/libxml2.lib \
		$(TIXI_LIB_SHARED_GCC_VC7)/libxslt.lib \
		$(TIXI_LIB_SHARED_GCC_VC7)/curllib.lib \
		$$CASLIBS/PTKernel.lib \
		$$CASLIBS/TKAdvTools.lib \
		$$CASLIBS/TKBin.lib \
		$$CASLIBS/TKBinL.lib \
		$$CASLIBS/TKBinXCAF.lib \
		$$CASLIBS/TKBO.lib \
		$$CASLIBS/TKBool.lib \
		$$CASLIBS/TKBRep.lib \
		$$CASLIBS/TKCAF.lib \
		$$CASLIBS/TKCDF.lib \
		$$CASLIBS/TKDCAF.lib \
		$$CASLIBS/TKDraw.lib \
		$$CASLIBS/TKernel.lib \
		$$CASLIBS/TKFeat.lib \
		$$CASLIBS/TKFillet.lib \
		$$CASLIBS/TKG2d.lib \
		$$CASLIBS/TKG3d.lib \
		$$CASLIBS/TKGeomAlgo.lib \
		$$CASLIBS/TKGeomBase.lib \
		$$CASLIBS/TKHLR.lib \
		$$CASLIBS/TKIGES.lib \
		$$CASLIBS/TKLCAF.lib \
		$$CASLIBS/TKMath.lib \
		$$CASLIBS/TKMesh.lib \
		$$CASLIBS/TKMeshVS.lib \
		$$CASLIBS/TKOffset.lib \
		$$CASLIBS/TKOpenGl.lib \
		$$CASLIBS/TKPCAF.lib \
		$$CASLIBS/TKPLCAF.lib \
		$$CASLIBS/TKPrim.lib \
		$$CASLIBS/TKPShape.lib \
		$$CASLIBS/TKService.lib \
		$$CASLIBS/TKShapeSchema.lib \
		$$CASLIBS/TKShHealing.lib \
		$$CASLIBS/TKStdLSchema.lib \
		$$CASLIBS/TKStdSchema.lib \
		$$CASLIBS/TKSTEP.lib \
		$$CASLIBS/TKSTEP209.lib \
		$$CASLIBS/TKSTEPAttr.lib \
		$$CASLIBS/TKSTEPBase.lib \
		$$CASLIBS/TKSTL.lib \
		$$CASLIBS/TKTopAlgo.lib \
		$$CASLIBS/TKTopTest.lib \
		$$CASLIBS/TKV2d.lib \
		$$CASLIBS/TKV3d.lib \
		$$CASLIBS/TKViewerTest.lib \
		$$CASLIBS/TKVRML.lib \
		$$CASLIBS/TKXCAF.lib \
		$$CASLIBS/TKXCAFSchema.lib \
		$$CASLIBS/TKXDEDRAW.lib \
		$$CASLIBS/TKXDEIGES.lib \
		$$CASLIBS/TKXDESTEP.lib \
		$$CASLIBS/TKXml.lib \
		$$CASLIBS/TKXmlL.lib \
		$$CASLIBS/TKXmlXCAF.lib \
		$$CASLIBS/TKXSBase.lib \
		$$CASLIBS/TKXSDRAW.lib 

#		$$CASLIBS/StdLPlugin.lib \
#		$$CASLIBS/StdPlugin.lib \		
#		$$CASLIBS/TKjcas.lib \		
#		$$CASLIBS/BinPlugin.lib \
#		$$CASLIBS/BinXCAFPlugin.lib \
#		$$CASLIBS/FWOSPlugin.lib \
#		$$CASLIBS/mscmd.lib \		
#		$$CASLIBS/XCAFPlugin.lib \
#		$$CASLIBS/XmlLPlugin.lib \
#		$$CASLIBS/XmlPlugin.lib \
#		$$CASLIBS/XmlXCAFPlugin.lib