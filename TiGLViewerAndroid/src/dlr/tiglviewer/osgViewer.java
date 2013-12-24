package dlr.tiglviewer;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ExpandableListActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.PointF;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.os.Environment;
import android.util.FloatMath;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ExpandableListView;
import android.widget.PopupWindow;
import android.widget.PopupWindow.OnDismissListener;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ImageButton;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class osgViewer extends Activity implements View.OnTouchListener {

	enum moveTypes { NONE , DRAG, MDRAG, ZOOM ,ACTUALIZE}
	enum navType { PRINCIPAL , SECONDARY }
	enum lightType { ON , OFF }
		
	moveTypes mode=moveTypes.NONE;
	navType navMode = navType.PRINCIPAL;
	lightType lightMode = lightType.ON;
	
	PointF oneFingerOrigin = new PointF(0,0);
	long timeOneFinger=0;
	PointF twoFingerOrigin = new PointF(0,0);
	long timeTwoFinger=0;
	float distanceOrigin;
	
	int backgroundColor;
	
	private static final String TAG = "OSG Activity";
	
    EGLview mView;
    Button AboutButton;
    Button closeFileButton;
    Spinner cameraList;
    Spinner fileSpinner;
    PopupWindow pw;
    private File file;
    private List<String> fileList;
    boolean popUp = false;
    
    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        osgNativeLib.setAssetMgr(getAssets());
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
            WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.ui_layout_gles); 
        	mView = (EGLview) findViewById(R.id.surfaceGLES1);
        	mView.setOnTouchListener(this);
        	
       AboutButton = (Button) findViewById(R.id.button1);
       		AboutButton.setOnClickListener(uiListenerAbout);
       closeFileButton = (Button) findViewById(R.id.button2);
       		closeFileButton.setOnClickListener(uiListenserCloseFile);

       cameraList = (Spinner) findViewById(R.id.spinner1);
       		cameraList.setOnItemSelectedListener(new CameraListListener());
      
       
  
       	
       	loadFilesAndAddThemToSpinner();
   
    }

public void loadFilesAndAddThemToSpinner()
{
	fileSpinner = (Spinner) findViewById(R.id.spinner2);	   	
    fileList = new ArrayList<String>();   
    
    file = new File(Environment.getExternalStorageDirectory(), "Tiglviewer");
    if (!file.exists()) {
    	file.mkdirs();
		Log.e(TAG," making directory because it didnot exist");
    }
 
           
    File list[] = file.listFiles();

    fileList.add("Choose File");
    
    for( int i=0; i< list.length; i++)
    {
 	   fileList.add( list[i].getName() );
    }
    
    ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this,
    		android.R.layout.simple_spinner_item, fileList);
    	dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    	fileSpinner.setAdapter(dataAdapter);

    	
    	
    	fileSpinner.setOnItemSelectedListener(new FileSelectListener());
    	
}
    
OnClickListener uiListenerAbout = new OnClickListener() {
	
	@Override
	public void onClick(View v) {
	
		AboutButton.setClickable(false);
     	LayoutInflater inflater = (LayoutInflater)osgViewer.this.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
     	pw = new PopupWindow(inflater.inflate(R.layout.about, null, false), 400, 400, false);
		pw.setTouchable(true);
		pw.setOutsideTouchable(true);
        pw.setBackgroundDrawable(new BitmapDrawable());
        pw.setContentView(inflater.inflate(R.layout.about, null, false));
		pw.showAtLocation(findViewById(R.id.root), Gravity.CENTER, 0, 0);
		pw.setTouchInterceptor(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View v, MotionEvent event) {
                    if (event.getAction() == MotionEvent.ACTION_OUTSIDE) {
                        Log.i("Background", "Back Touched");
                        pw.dismiss();
                        AboutButton.setClickable(true);
                        return true;
                    }
                    return false;
                }
            });		
		
		}

};




OnClickListener uiListenserCloseFile = new OnClickListener() {
	
	@Override
	public void onClick(View v) {
		
		osgNativeLib.removeObjects();
		
	}
};


    
	@Override
public boolean onTouch(View v, MotionEvent event) { 
		long time_arrival = event.getEventTime();
    	int n_points = event.getPointerCount();
    	int action = event.getAction() & MotionEvent.ACTION_MASK;
    	
    	
    		
    	switch(n_points){
    	case 3:
    		switch(action)
    		{
    		
    		case MotionEvent.ACTION_POINTER_DOWN:
	    		mode = moveTypes.DRAG;
	    		
	    		
	    		osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , 1);
	    		if(navMode==navType.SECONDARY)
	    		{
	    			osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 2 , 1);
	    		}
	    		else
	    		{
	    			osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 1 , 1);
	    		}
	    		oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
    			break;
    		case MotionEvent.ACTION_CANCEL:
    			
    			
    			switch(mode){
    			case DRAG:
    				osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , 1);
    				if(navMode==navType.SECONDARY)
    	    		{
    	    			osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2 , 1);
    	    		}
    	    		else
    	    		{
    	    			osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , 1);
    	    		}
    				break;
    			default :
    				Log.e(TAG,"There has been an anomaly in touch input 1point/action");
    			}
    			mode = moveTypes.NONE;
    			break;
    		case MotionEvent.ACTION_MOVE:
    			
    			
    			osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , 1);
    			
    			oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
	    		
    			break;
    		case MotionEvent.ACTION_POINTER_UP:
    			
    			
    			switch(mode){
    			case DRAG:
    				if(navMode==navType.SECONDARY)
    	    		{
    	    			osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2 , 1);
    	    		}
    	    		else
    	    		{
    	    			osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , 1);
    	    		}
    				break;
    			default :
    				Log.e(TAG,"There has been an anomaly in touch input 1 point/action");
    			}
    			mode = moveTypes.NONE;
    			break;
    		default :
    			Log.e(TAG,"1 point Action not captured");	
    		}
    		break;
    	case 2:
    		switch (action){
    		case MotionEvent.ACTION_POINTER_DOWN:

    			
    			switch(mode){
    			case DRAG:
    				if(navMode==navType.PRINCIPAL)
    	    		{
    	    			osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , 1);
    	    		}
    	    		else
    	    		{
    	    			osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2 , 1);
    	    		}
    				break;
    			default:
    				break;
    			}
    			mode = moveTypes.ZOOM;
    			distanceOrigin = sqrDistance(event);
    			twoFingerOrigin.x=event.getX(1);
    			twoFingerOrigin.y=event.getY(1);
    			oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
    			
    			osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y , 1);
    			osgNativeLib.mouseButtonPressEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3 , 1);
    			osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y , 1);
    			
    		case MotionEvent.ACTION_MOVE:
    			float distance = sqrDistance(event);
    			float result = distance-distanceOrigin;
    			distanceOrigin=distance;
    			
    			if(result>1||result<-1){
    	    		oneFingerOrigin.y=oneFingerOrigin.y+result;
    				osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y , 1);
    			}
    			
    			break;
    		case MotionEvent.ACTION_POINTER_UP:
    			mode =moveTypes.NONE;
    			osgNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3 , 1);
    			break;
    		case MotionEvent.ACTION_UP:
    			mode =moveTypes.NONE;
    			osgNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3 , 1);
    			break;
    		default :
    			Log.e(TAG,"2 point Action not captured");
    		}
    		break;
    	case 1:
    		switch(action){
    		case MotionEvent.ACTION_DOWN:
    			
    			
	    		mode = moveTypes.DRAG;
	    		
	    		osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , 1);
	    		if(navMode==navType.PRINCIPAL)
	    			osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 1 , 1);
	    		else
	    			//osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 2);
	    		
	    		oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
    			break;
    		case MotionEvent.ACTION_CANCEL:
    			switch(mode){
    			case DRAG:
    				osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , 1);
    				if(navMode==navType.PRINCIPAL)
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , 1);
    				else
    					//osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2);
    				break;
    			default :
    				Log.e(TAG,"There has been an anomaly in touch input 1point/action");
    			}
    			mode = moveTypes.NONE;
    			break;
    		case MotionEvent.ACTION_MOVE:
    			
    			osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , 1);
    			
    			oneFingerOrigin.x=event.getX(0);
	    		oneFingerOrigin.y=event.getY(0);
	    		
    			break;
    		case MotionEvent.ACTION_UP:
    			switch(mode){
    			case DRAG:
    				if(navMode==navType.PRINCIPAL)
    					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , 1);
    				else
    					//osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2);
    				break;
    			default :
    				Log.e(TAG,"There has been an anomaly in touch input 1 point/action");
    			}
    			mode = moveTypes.NONE;
    			break;
    		default :
    			Log.e(TAG,"1 point Action not captured");	
    		}
    		break;
		

    	}
    	
		return true;
}
    // The following commented code is used for touch event when a compsitie viewer with 4 
	// views (Pres, Top, Side and Front) is used.
   
	/* 	else{
    		int view = 1;
    		if(event.getY() <= 400 && event.getX() > 600)
    			view = 4;
    		if(event.getY() > 400 && event.getX() <= 600)
    		    view = 2;
    		if(event.getY() > 400 && event.getX() > 600)
    		    view = 3;
    		
    		
    		switch(n_points){
        	case 1:
        		switch(action){
        		case MotionEvent.ACTION_DOWN:
    	    		mode = moveTypes.DRAG;
    	    		
    	    		osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , view);
    	    		if(navMode==navType.PRINCIPAL)
    	    			osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 1 , view);
    	    		else
    	    			//osgNativeLib.mouseButtonPressEvent(event.getX(0), event.getY(0), 2);
    	    		
    	    		oneFingerOrigin.x=event.getX(0);
    	    		oneFingerOrigin.y=event.getY(0);
        			break;
        		case MotionEvent.ACTION_CANCEL:
        			switch(mode){
        			case DRAG:
        				osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) , view);
        				if(navMode==navType.PRINCIPAL)
        					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , view);
        				else
        					//osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2);
        				break;
        			default :
        				Log.e(TAG,"There has been an anomaly in touch input 1point/action");
        			}
        			mode = moveTypes.NONE;
        			break;
        		case MotionEvent.ACTION_MOVE:
        			
        			osgNativeLib.mouseMoveEvent(event.getX(0), event.getY(0) ,view);
        			
        			oneFingerOrigin.x=event.getX(0);
    	    		oneFingerOrigin.y=event.getY(0);
    	    		
        			break;
        		case MotionEvent.ACTION_UP:
        			switch(mode){
        			case DRAG:
        				if(navMode==navType.PRINCIPAL)
        					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , view);
        				else
        					//osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2);
        				break;
        			default :
        				Log.e(TAG,"There has been an anomaly in touch input 1 point/action");
        			}
        			mode = moveTypes.NONE;
        			break;
        		default :
        			Log.e(TAG,"1 point Action not captured");	
        		}
        		break;
        	case 2:
        		switch (action){
        		case MotionEvent.ACTION_POINTER_DOWN:
        			//Free previous Action
        			switch(mode){
        			case DRAG:
        				if(navMode==navType.PRINCIPAL)
        					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 1 , view);
        				else
        					osgNativeLib.mouseButtonReleaseEvent(event.getX(0), event.getY(0), 2 , view);
        				break;
        			}
        			mode = moveTypes.ZOOM;
        			distanceOrigin = sqrDistance(event);
        			twoFingerOrigin.x=event.getX(1);
        			twoFingerOrigin.y=event.getY(1);
        			oneFingerOrigin.x=event.getX(0);
    	    		oneFingerOrigin.y=event.getY(0);
        			
        			osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y , view);
        			osgNativeLib.mouseButtonPressEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3 , view);
        			osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y , view);
        			
        		case MotionEvent.ACTION_MOVE:
        			float distance = sqrDistance(event);
        			float result = distance-distanceOrigin;
        			distanceOrigin=distance;
        			
        			if(result>1||result<-1){
        	    		oneFingerOrigin.y=oneFingerOrigin.y+result;
        				osgNativeLib.mouseMoveEvent(oneFingerOrigin.x,oneFingerOrigin.y , view);
        			}
        			
        			break;
        		case MotionEvent.ACTION_POINTER_UP:
        			mode =moveTypes.NONE;
        			osgNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3 , view);
        			break;
        		case MotionEvent.ACTION_UP:
        			mode =moveTypes.NONE;
        			osgNativeLib.mouseButtonReleaseEvent(oneFingerOrigin.x,oneFingerOrigin.y, 3 , view);
        			break;
        		default :
        			Log.e(TAG,"2 point Action not captured");
        		}
        		break;    		
        	}
    } */

	// The pervious commented code is used for touch event when a compsitie viewer with 4 
	// views (Pres, Top, Side and Front) is used.

	

private float sqrDistance(MotionEvent event) 
{
        float x = event.getX(0) - event.getX(1);
        float y = event.getY(0) - event.getY(1);
        return (float)(Math.sqrt(x * x + y * y));
     }


}