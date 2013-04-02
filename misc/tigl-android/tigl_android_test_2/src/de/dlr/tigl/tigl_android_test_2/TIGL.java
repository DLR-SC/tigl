package de.dlr.tigl.tigl_android_test_2;

import android.util.Log;

public class TIGL {

    // load crosscompiles oce libs and the very basic wrapper for the tigl library (just calling tigl as a demo that it works, no real functionality yet). 
  static 
  {
   try {
        System.loadLibrary("TKernel");
        System.loadLibrary("TKMath");
        System.loadLibrary("TKG2d");
        System.loadLibrary("TKG3d");
        System.loadLibrary("TKGeomBase");
        System.loadLibrary("TKBRep");
        System.loadLibrary("TKGeomAlgo");
        System.loadLibrary("TKTopAlgo");
        System.loadLibrary("TKPrim");
        System.loadLibrary("TKBool");
        System.loadLibrary("tigl_binary_wrapper");
        
   } catch (UnsatisfiedLinkError use) {
        Log.e("TIGL", "WARNING: Could not load libraries");
     }
  }

  public native double result( double v1, double v2 );

}
