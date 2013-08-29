
package dlr.tiglviewer;
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import android.app.Dialog;
import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.opengl.GLU;
import android.util.AttributeSet;
import android.util.Log;
import android.view.*;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;
/**
 * A simple GLSurfaceView sub-class that demonstrate how to perform
 * OpenGL ES 1.0 rendering into a GL Surface. Note the following important
 * details:
 *
 * - The class must use a custom context factory to enable 1.0 rendering.
 *   See ContextFactory class definition below.
 *
 * - The class must use a custom EGLConfigChooser to be able to select
 *   an EGLConfig that supports 1.0. This is done by providing a config
 *   specification to eglChooseConfig() that has the attribute
 *   EGL10.ELG_RENDERABLE_TYPE containing the EGL_OPENGL_ES_BIT flag
 *   set. See ConfigChooser class definition below.
 *
 * - The class must select the surface's format, then choose an EGLConfig
 *   that matches it exactly (with regards to red/green/blue/alpha channels
 *   bit depths). Failure to do so would result in an EGL_BAD_MATCH error.
 */
public class EGLview extends GLSurfaceView {

	public EGLview(Context context) {
	    super(context);
	    setRenderer(new Renderer());
	}
	public EGLview(Context context, AttributeSet attrs) {
	    super(context,attrs);
	    setRenderer(new Renderer());
	}
		
	private static class Renderer implements GLSurfaceView.Renderer {
	    	
		int wid = 0;
		int hi = 0;

		public void onDrawFrame(GL10 gl) {		
		osgNativeLib.step();		      
		}
	
	    public void onSurfaceChanged(GL10 gl, int width, int height) 
	    {
	    	wid = width;
	    	hi = height;    
		    osgNativeLib.init(wid, hi);

	    }
	
	    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

	        }
	    }
	  
}

