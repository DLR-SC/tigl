package de.dlr.tigl.tigl_android_test_1;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;


public class Tigl_Android_Test_1 extends Activity {	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        Button callOCC = (Button) findViewById(R.id.callOCC);
        callOCC.setOnClickListener(new OnClickListener() {
        TextView shellOutput = (TextView) findViewById(R.id.shellOutput);
        	   
        public void onClick(View v) {
        	shellOutput.setText(ReadShellOutput());
        }  
      }); 
    }
    
    private String ReadShellOutput() {
     ProcessBuilder shell;
     String result="", s="";  
     try {		  
    	  String[] arg = { "./bin/testOpenCascade" };
          shell = new ProcessBuilder(arg);
          Process proc = shell.start();
          InputStream is = proc.getInputStream();
          InputStreamReader isr = new InputStreamReader(is);
          BufferedReader br = new BufferedReader(isr);
          while ((s = br.readLine()) != null) {
               StringBuffer sb = new StringBuffer(s);
               result += new String(sb+"\n");
          }
    	  br.close();
    	  isr.close();
    	  is.close();
    	  proc.destroy();
     } 
     catch(IOException ex) {
          ex.printStackTrace();
     }
     	return result;
  }
}
