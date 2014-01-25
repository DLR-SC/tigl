package de.dlr.sc.tiglviewer.android;
import android.content.res.AssetManager;


public class TiGLViewerNativeLib {
    
    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("tiglviewer-native");
    }

   /**
    * @param width the current view width
    * @param height the current view height
    */
    public static native void       init(int width, int height);
    public static native void       createScene();
    public static native void       setAssetMgr(AssetManager mgr);
    public static native void       step();
    public static native void       addObjectFromVTK(String filepath);
    public static native void       addObjectFromCPACS(String filepath);
    public static native void       removeObjects();
    public static native void       changeCamera(int view);
    public static native void       clearContents();
    public static native void       mouseButtonPressEvent(float x,float y, int button , int view);
    public static native void       mouseButtonReleaseEvent(float x,float y, int button , int view);
    public static native void       mouseMoveEvent(float x,float y , int view);
    public static native void       keyboardDown(int key);
    public static native void       keyboardUp(int key);
    public static native void       setClearColor(int red,int green, int blue);
    public static native int[]      getClearColor();
    public static native void       loadObject(String address);
    public static native void       loadObject(String address,String name);
    public static native void       unLoadObject(int number);
    public static native String[]   getObjectNames();
    public static native String     tiglGetVersion();
}

