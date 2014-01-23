/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package de.dlr.sc.tiglviewer.android;

import java.io.File;
import java.util.ArrayList;


import android.app.Dialog;
import android.content.Context;
import android.content.res.Configuration;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.ActionBarDrawerToggle;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBar.OnNavigationListener;
import android.support.v7.app.ActionBarActivity;
import android.text.util.Linkify;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class TiglViewerActivity extends ActionBarActivity implements OnNavigationListener {
    private DrawerLayout mDrawerLayout;
    private ListView mDrawerList;
    private ActionBarDrawerToggle mDrawerToggle;

    private CharSequence mDrawerTitle;
    private CharSequence mTitle;
    
    // list of possible files to open
    private ArrayList<String> fileList;
	private GLSurfaceView openGlSurface;
	private Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //TiGLViewerNativeLib.setAssetMgr(getAssets());
        
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
                             WindowManager.LayoutParams.FLAG_FULLSCREEN);
        
        //supportRequestWindowFeature(Window.FEATURE_PROGRESS);
        supportRequestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.activity_main);

        mTitle = mDrawerTitle = getTitle();

        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        mDrawerList = (ListView) findViewById(R.id.left_drawer);

        // set a custom shadow that overlays the main content when the drawer opens
        mDrawerLayout.setDrawerShadow(R.drawable.drawer_shadow, Gravity.RIGHT);
        // set up the drawer's list view with items and click listener
        this.addCPACSFilesToDrawer();
        mDrawerList.setOnItemClickListener(new DrawerItemClickListener());

        // enable ActionBar app icon to behave as action to toggle nav drawer
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setLogo(R.drawable.ic_action_logo);
        
        getSupportActionBar().setDisplayShowTitleEnabled(false);
        getSupportActionBar().setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);
       
        String[] navStrings= getResources().getStringArray(R.array.view_mode);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(getSupportActionBar().getThemedContext(),
        		R.layout.spinner_layout,
				android.R.id.text1,
				navStrings);
        
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        
        getSupportActionBar().setListNavigationCallbacks(adapter, this);
        
        // ActionBarDrawerToggle ties together the the proper interactions
        // between the sliding drawer and the action bar app icon
        mDrawerToggle = new ActionBarDrawerToggle(
                this,                  /* host Activity */
                mDrawerLayout,         /* DrawerLayout object */
                R.drawable.ic_drawer,  /* nav drawer image to replace 'Up' caret */
                R.string.drawer_open,  /* "open drawer" description for accessibility */
                R.string.drawer_close  /* "close drawer" description for accessibility */
                ) {
            public void onDrawerClosed(View view) {
                getSupportActionBar().setTitle(mTitle);
                supportInvalidateOptionsMenu(); // creates call to onPrepareOptionsMenu()
            }

            public void onDrawerOpened(View drawerView) {
                getSupportActionBar().setTitle(mDrawerTitle);
                supportInvalidateOptionsMenu(); // creates call to onPrepareOptionsMenu()
            }
        };
        mDrawerLayout.setDrawerListener(mDrawerToggle);
        
        openGlSurface = (GLSurfaceView) findViewById(R.id.surfaceGLES1);
        openGlSurface.setOnTouchListener(new EGLTouchListener());
        setSupportProgressBarIndeterminate(true);
        
        TiGLViewerNativeLib.setAssetMgr(getAssets());
    }
    
    @Override
    protected void onPause() {
    	if (openGlSurface != null) {
    		openGlSurface.onPause();
    	}
    	super.onPause();
    };
    
    @Override
    protected void onResume() {
    	if (openGlSurface != null) {
    		openGlSurface.onResume();
    	}
    	super.onResume();
    };

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);
        return super.onCreateOptionsMenu(menu);
    }

    /* Called whenever we call invalidateOptionsMenu() */
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // If the nav drawer is open, hide action items related to the content view
        //boolean drawerOpen = mDrawerLayout.isDrawerOpen(mDrawerList);
        //menu.findItem(R.id.action_websearch).setVisible(!drawerOpen);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
         // The action bar home/up action should open or close the drawer.
         // ActionBarDrawerToggle will take care of this.
        if (mDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }

        
        // Handle action buttons
        switch(item.getItemId()) {
        case R.id.action_delete:
        	TiGLViewerNativeLib.removeObjects();
        	return true;
        	
        case R.id.action_about:
            AboutDialog about = new AboutDialog(this);
            about.setTitle("About TiGL Viewer");
            about.show();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    /* The click listner for ListView in the navigation drawer */
    private class DrawerItemClickListener implements ListView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            selectItem(position);
            mDrawerLayout.closeDrawer(mDrawerList);
        }
    }

    // is called when item in drawer is selected
    private void selectItem(int position) {
        // update the main content by replacing fragments


        // update selected item and title, then close the drawer
        mDrawerList.setItemChecked(position, true);    
        mDrawerLayout.closeDrawer(mDrawerList);
		
		if(fileList == null || fileList.size() <= position) {
			return;
		}
		final String file = fileList.get(position);		
		final String filename = Environment.getExternalStorageDirectory().getPath() + "/Tiglviewer/" + file;
		setSupportProgressBarIndeterminateVisibility(true);
		Thread thread = new Thread() {
			public void run() {
				if(filename.contains("xml")) {
					TiGLViewerNativeLib.addObjectFromCPACS(filename);
				}
				else if(filename.contains("vtp")) {
					TiGLViewerNativeLib.addObjectFromVTK(filename);
				}
				
				// make success dialog appear
				Runnable r=new Runnable() {
					@Override
					public void run() {
						setSupportProgressBarIndeterminateVisibility(false);
						Toast.makeText(getBaseContext(), 
								"File : " + file + " has been Loaded",
								Toast.LENGTH_SHORT).show();

					}
				};
				handler.post(r);
			} // run
		}; // thread
		thread.start(); 	
				
    }

    @Override
    public void setTitle(CharSequence title) {
        mTitle = title;
        getSupportActionBar().setTitle(mTitle);
    }

    /**
     * When using the ActionBarDrawerToggle, you must call it during
     * onPostCreate() and onConfigurationChanged()...
     */

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        // Sync the toggle state after onRestoreInstanceState has occurred.
        mDrawerToggle.syncState();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Pass any configuration change to the drawer toggls
        mDrawerToggle.onConfigurationChanged(newConfig);
    }

	@Override
	public boolean onNavigationItemSelected(int position, long id) {
		// handle selected view mode
		TiGLViewerNativeLib.changeCamera(position);
		
		return true;
	}
	
	public void addCPACSFilesToDrawer() {
	    fileList = new ArrayList<String>();   
	    
	    File file = new File(Environment.getExternalStorageDirectory(), "Tiglviewer");
	    if (!file.exists()) {
	    	Log.d("TiGL Viewer", "Application directory does not exist, creating directory Tiglviewer");
	        file.mkdirs();
	    }

	    File list[] = file.listFiles();
	    if(list != null) {		    
		    for ( int i=0; i< list.length; i++) {
		       fileList.add( list[i].getName() );
		    }
	    }

        mDrawerList.setAdapter(new ArrayAdapter<String>(this,
                R.layout.drawer_list_item, fileList));
	        
	}
	
	@Override
	protected void onRestart() {
		addCPACSFilesToDrawer();
		
		super.onRestart();
	}
	
	
	/**
	 * TiGLViewer about dialog
	 */
	public class AboutDialog extends Dialog {
		public AboutDialog(Context context) {
			super(context);
		}

		@Override
		public void onCreate(Bundle savedInstanceState) {
			setContentView(R.layout.about_dialog);
			TextView tv = (TextView) findViewById(R.id.about_text);
			tv.setText("The TiGLViewer allows you to view CPACS geometries.\n" +
					   "\n" + 
					   "TiGL uses Open CASCADE 6.7.0 and OpenSceneGraph 3.2.1\n" +
					   "\n" +
					   "Visit the TiGL project page at http://code.google.com/p/tigl/");
			Linkify.addLinks(tv, Linkify.WEB_URLS);
			tv = (TextView) findViewById(R.id.tigl_version);
			tv.setText("v2.1.0");
		}
	}


}

