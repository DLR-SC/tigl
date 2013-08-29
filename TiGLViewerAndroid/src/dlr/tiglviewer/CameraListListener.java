package dlr.tiglviewer;

import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Toast;
 
public class CameraListListener implements OnItemSelectedListener {
 
  public void onItemSelected(AdapterView<?> parent, View view, int pos,long id) {

	int choice = 0;
	
	if(parent.getItemAtPosition(pos).toString().equals("Prespective View"))
	{
		choice = 0;
		osgNativeLib.changeCamera(choice);

	}
	
	else if(parent.getItemAtPosition(pos).toString().equals("Top View"))
	{	
		choice = 1;
		osgNativeLib.changeCamera(choice);

	}
	else if(parent.getItemAtPosition(pos).toString().equals("Side View"))
	{
		choice = 2;
		osgNativeLib.changeCamera(choice);

	}
	else if(parent.getItemAtPosition(pos).toString().equals("Front View"))
	{	
		choice = 3;
		osgNativeLib.changeCamera(choice);

	}
		
  }
 
  @Override
  public void onNothingSelected(AdapterView<?> arg0) {
	
  }
 
}