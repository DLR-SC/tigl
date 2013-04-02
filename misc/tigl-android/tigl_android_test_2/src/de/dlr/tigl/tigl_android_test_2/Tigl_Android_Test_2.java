package de.dlr.tigl.tigl_android_test_2;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.view.View.OnClickListener;


public class Tigl_Android_Test_2 extends Activity {
	
    TIGL occNative;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        occNative = new TIGL();
        
        Button buttonCalc = (Button) findViewById(R.id.buttonCalc);
        
        buttonCalc.setOnClickListener(new OnClickListener() {
        TextView result = (TextView) findViewById(R.id.result);
        EditText value1 = (EditText) findViewById(R.id.value1);
        EditText value2 = (EditText) findViewById(R.id.value2);
        
        public void onClick(View v) {
             double v1 =0;
             double v2 =0;
             double res=0;
             if (value1.length() <=0 ) {
                 value1.setError(getText(R.string.error));
             }
             else if (value2.length() <=0 ) {
                 value2.setError(getText(R.string.error));
             }
             else if ((value1.length() > 0) && (value2.length()) > 0)
             {
                 v1 = Double.parseDouble(value1.getText().toString());
                 v2 = Double.parseDouble(value2.getText().toString());
                 res = occNative.result(v1, v2);
                 result.setText(new Double(res).toString());
             }
         }  
      }); 
   }
}
