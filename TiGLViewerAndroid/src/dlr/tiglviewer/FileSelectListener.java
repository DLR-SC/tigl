package dlr.tiglviewer;

import android.os.Environment;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Toast;
 
public class FileSelectListener implements OnItemSelectedListener {
 
  public void onItemSelected(AdapterView<?> parent, View view, int pos,long id) {
    
    
    if (!parent.getItemAtPosition(pos).toString().equals("Choose File")) {
     
    String dataPath = Environment.getExternalStorageDirectory().getPath() + "/Tiglviewer/";
    if(parent.getItemAtPosition(pos).toString().contains("xml")) {
        TiGLViewerNativeLib.addObjectFromCPACS(dataPath + parent.getItemAtPosition(pos).toString());
    }
    else if(parent.getItemAtPosition(pos).toString().contains("vtp")) {
        TiGLViewerNativeLib.addObjectFromVTK(dataPath + parent.getItemAtPosition(pos).toString());
    }
    
    Toast.makeText(parent.getContext(), 
               "File : " + parent.getItemAtPosition(pos).toString() + " has been Loaded",
               Toast.LENGTH_SHORT).show();
    }
    
  }
 
  @Override
  public void onNothingSelected(AdapterView<?> arg0) {
    
  }
 
}