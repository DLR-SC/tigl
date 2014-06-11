/*
 * Copyright (C) 2007-2014 German Aerospace Center (DLR/SC)
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

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import org.apache.http.util.ByteArrayBuffer;

import android.annotation.SuppressLint;
import android.app.ActionBar.LayoutParams;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.opengl.GLSurfaceView;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.ActionBarDrawerToggle;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBar.OnNavigationListener;
import android.support.v7.app.ActionBarActivity;
import android.text.Html;
import android.text.util.Linkify;
import android.util.DisplayMetrics;
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
import de.dlr.sc.tiglviewer.android.NoticeFragment.noticeFragmentsListener;

@SuppressLint("ValidFragment")
public class TiglViewerActivity extends ActionBarActivity implements
        OnNavigationListener, noticeFragmentsListener {
    enum navType {
        MOVE, ROTATE
    }

    private DrawerLayout mDrawerLayout;
    private ListView mDrawerList;
    private ActionBarDrawerToggle mDrawerToggle;
    private EGLTouchListener mTouchListener;
    public boolean downloadingOrNot = false;

    private CharSequence mDrawerTitle;
    private CharSequence mTitle;
    navType navMode = navType.ROTATE;

    // list of possible files to open
    private GLSurfaceView openGlSurface;
    private Handler handler = new Handler();
    ArrayList<Item> drawerItems = new ArrayList<Item>();
    Context context;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        context = this;
        //TiGLViewerNativeLib.setAssetMgr(getAssets());        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        // supportRequestWindowFeature(Window.FEATURE_PROGRESS);
        supportRequestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.activity_main);
        setSupportProgressBarIndeterminateVisibility(false);

        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);

        mTitle = mDrawerTitle = getTitle();

        mDrawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        mDrawerList = (ListView) findViewById(R.id.left_drawer);

        // set a custom shadow that overlays the main content when the drawer
        // opens
        mDrawerLayout.setDrawerShadow(R.drawable.drawer_shadow, Gravity.RIGHT);

        drawerItems.add(new ParentItem("Open Files"));
        drawerItems.add(new ParentItem("Other Actions"));
        drawerItems.add(new ChildItem("Download Models", "Download Models"));
        drawerItems.add(new ChildItem("Export File", "Export File"));

        ModelAdapter drawerWrapper = new ModelAdapter(this, drawerItems);
        mDrawerList.setAdapter(drawerWrapper);

        mDrawerList.setOnItemClickListener(new DrawerItemClickListener());

        File list[] = checkDirectory("Tiglviewer");
        if (list == null) {
            showDownloadPrompt();
        } else {
            addCPACSFilesToDrawer();
        }

        // enable ActionBar app icon to behave as action to toggle nav drawer
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeButtonEnabled(true);
        getSupportActionBar().setLogo(R.drawable.ic_action_logo);

        getSupportActionBar().setDisplayShowTitleEnabled(false);
        getSupportActionBar().setNavigationMode(ActionBar.NAVIGATION_MODE_LIST);

        String[] navStrings = getResources().getStringArray(R.array.view_mode);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                getSupportActionBar().getThemedContext(),
                R.layout.spinner_layout, android.R.id.text1, navStrings);

        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        getSupportActionBar().setListNavigationCallbacks(adapter, this);

        // ActionBarDrawerToggle ties together the the proper interactions
        // between the sliding drawer and the action bar app icon
        mDrawerToggle = new ActionBarDrawerToggle(this, /* host Activity */
            mDrawerLayout, /* DrawerLayout object */
            R.drawable.ic_drawer, /* nav drawer image to replace 'Up' caret */
            R.string.drawer_open, /* "open drawer" description for accessibility */
            R.string.drawer_close /* "close drawer" description for accessibility */
        ) {
            public void onDrawerClosed(View view) {
                getSupportActionBar().setTitle(mTitle);
                supportInvalidateOptionsMenu(); // creates call to
                                                // onPrepareOptionsMenu()
            }

            public void onDrawerOpened(View drawerView) {
                getSupportActionBar().setTitle(mDrawerTitle);
                supportInvalidateOptionsMenu(); // creates call to
                                                // onPrepareOptionsMenu()
            }
        };
        mDrawerLayout.setDrawerListener(mDrawerToggle);

        openGlSurface = (GLSurfaceView) findViewById(R.id.surfaceGLES1);
        mTouchListener = new EGLTouchListener(metrics.densityDpi);
        openGlSurface.setOnTouchListener(mTouchListener);
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
        if (navMode == navType.ROTATE) {
            mTouchListener.setNavMode(navMode);
            menu.findItem(R.id.action_toggle_navigation).setIcon(
                    R.drawable.ic_move);
        } else {
            mTouchListener.setNavMode(navMode);
            menu.findItem(R.id.action_toggle_navigation).setIcon(
                    R.drawable.ic_action_rotate_left);
        }

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
        switch (item.getItemId()) {
        case R.id.action_delete:
            TiGLViewerNativeLib.removeObjects();
            return true;

        case R.id.action_about:
            AboutDialog about = new AboutDialog(this);
            about.setTitle("About TiGL Viewer");
            about.show();
            return true;
        case R.id.action_toggle_navigation:
            switch (navMode) {
            case MOVE:
                navMode = navType.ROTATE;
                mTouchListener.setNavMode(navMode);
                item.setIcon(R.drawable.ic_move);
                break;
            case ROTATE:
                navMode = navType.MOVE;
                mTouchListener.setNavMode(navMode);
                item.setIcon(R.drawable.ic_action_rotate_left);
                break;
            }
            return true;
        case R.id.action_zoom:
            TiGLViewerNativeLib.fitScreen();
            return true;
        case R.id.action_help:
            HelpDialog help = new HelpDialog(this);
            help.setTitle("Help");
            help.show();
            Window window = help.getWindow();
            window.setLayout(LayoutParams.WRAP_CONTENT,
                    LayoutParams.WRAP_CONTENT);
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    /* The click listner for ListView in the navigation drawer */
    private class DrawerItemClickListener implements
            ListView.OnItemClickListener {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position,
                long id) {

            if (drawerItems.get(position).getName() == "Download Models") {
                downloadAction();
            } else if (drawerItems.get(position).getName() != "Export File") {
                selectItem(position);
            }
            mDrawerLayout.closeDrawer(mDrawerList);
        }
    }

    // is called when item in drawer is selected
    private void selectItem(int position) {
        // update the main content by replacing fragments
        // update selected item and title, then close the drawer
        mDrawerList.setItemChecked(position, true);
        mDrawerLayout.closeDrawer(mDrawerList);

        if (drawerItems == null || drawerItems.size() <= position) {
            return;
        }
        final Item selected = drawerItems.get(position);
        final String filename = Environment.getExternalStorageDirectory()
                .getPath() + "/Tiglviewer/" + selected.getName();
        setSupportProgressBarIndeterminateVisibility(true);
        Thread thread = new Thread() {
            public void run() {
                TiGLViewerNativeLib.openFile(filename);

                // make success dialog appear and stop the progress bar
                Runnable r = new Runnable() {
                    @Override
                    public void run() {
                        setSupportProgressBarIndeterminateVisibility(false);
                        Toast.makeText(
                                getBaseContext(),
                                "File : " + selected.getName()
                                        + " has been Loaded",
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

    /**
     * Checks if the a give directory of the device is existing or not and if it
     * is existring, is it empty or not? and if not it returns a list with the
     * files in the folder
     */
    public File[] checkDirectory(String folderName) {
        File file = new File(Environment.getExternalStorageDirectory(),
                folderName);
        if (!file.exists()) {
            return null;
        }
        File list[] = file.listFiles();
        if (list.length == 0) {
            return null;
        }
        return list;
    }

    /**
     * Add the sample models to the list in the navigation drawer
     */
    public void addCPACSFilesToDrawer() {

        File list[] = checkDirectory("Tiglviewer");
        boolean itemExists = false;

        if (list != null) {
            int ivalid = 0;
            for (int i = 0; i < list.length; i++) {
                itemExists = false;
                for (int j = 0; j < drawerItems.size(); j++) {
                    if (drawerItems.get(j).isParentSection() == 1) {
                        continue;
                    } else {
                        if (list[i].getName().equals(
                                drawerItems.get(j).getName())) {
                            itemExists = true;
                            break;
                        }
                    }
                }
                if (!itemExists
                        && TiGLViewerNativeLib.isFiletypeSupported(list[i]
                                .getName())) {
                    drawerItems.add(ivalid + 1, new ChildItem(
                            list[i].getName(), "-"));
                    ivalid++;
                }
            }
        }

        mDrawerList.setAdapter(new ModelAdapter(this, drawerItems));
    }

    /**
     * Checks If the device is connected to the internet with the correct
     * parameters or not
     */
    public boolean isConnectedToInternet() {
        ConnectivityManager connectivity = (ConnectivityManager) getApplicationContext()
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        if (connectivity != null) {
            NetworkInfo[] info = connectivity.getAllNetworkInfo();
            if (info != null) {
                for (int i = 0; i < info.length; i++) {
                    if (info[i].getState() == NetworkInfo.State.CONNECTED) {
                        return true;
                    }
                }
            }
        }
        return false;
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
            // set version numbers
            String s = tv.getText().toString();
            s = s.replace("TIGL_VERSION", TiGLViewerNativeLib.tiglGetVersion());
            s = s.replace("OCCT_VERSION", TiGLViewerNativeLib.occtGetVersion());
            s = s.replace("OSG_VERSION", TiGLViewerNativeLib.osgGetVersion());
            tv.setText(s);
            Linkify.addLinks(tv, Linkify.WEB_URLS);
            try {
                tv = (TextView) findViewById(R.id.tigl_version);
                String versionName = getPackageManager().getPackageInfo(
                        getPackageName(), 0).versionName;
                tv.setText("v" + versionName);
            } catch (PackageManager.NameNotFoundException e) {
                // don't add version info
            }

        }
    }

    public class HelpDialog extends Dialog {
        public HelpDialog(Context context) {
            super(context);
        }

        @Override
        public void onCreate(Bundle savedInstanceState) {
            setContentView(R.layout.help_dialog);
            TextView tv = (TextView) findViewById(R.id.tigl_details);
            tv.setText(Html.fromHtml(getResources().getString(
                    R.string.help_text)));
        }
    }

    /**
     * BackGround thread to download the sample models on the create of the
     * viewer activity
     */
    public class DownloadFilesThread extends AsyncTask<URL, Integer, Long>

    {

        @Override
        protected void onPreExecute() {
            setSupportProgressBarIndeterminateVisibility(true);
        }

        @Override
        protected Long doInBackground(URL... urls) {
            try {

                File file = new File(Environment.getExternalStorageDirectory(),
                        "Tiglviewer");

                if (!file.exists()) {
                    Log.d("TiGL Viewer",
                            "Application directory does not exist, creating directory Tiglviewer");
                    file.mkdirs();
                }

                HttpURLConnection c = (HttpURLConnection) urls[0]
                        .openConnection();
                InputStream inStream = c.getInputStream();

                Log.d("Downloading Files",
                        "Connection is opened, downloading Started");

                BufferedInputStream biStream = new BufferedInputStream(inStream);
                ByteArrayBuffer bB = new ByteArrayBuffer(70);

                int rest = 0;
                while ((rest = biStream.read()) != -1) {
                    bB.append((byte) rest);
                }

                String localpath = Environment.getExternalStorageDirectory()
                        .getPath() + "/Tiglviewer/models.zip";
                FileOutputStream outStream = new FileOutputStream(new File(
                        localpath));
                outStream.write(bB.toByteArray());
                outStream.close();

                Log.d("Downloading Files",
                        "Stream is closed, downloading Ended");

                File zippedModeled = new File(localpath);
                FileInputStream fin = new FileInputStream(zippedModeled);
                ZipInputStream zin = new ZipInputStream(fin);
                ZipEntry ze = null;
                while ((ze = zin.getNextEntry()) != null) {
                    FileOutputStream fout = new FileOutputStream(new File(
                            Environment.getExternalStorageDirectory().getPath()
                                    + "/Tiglviewer/" + ze.getName()));
                    byte[] bf = new byte[8192];
                    int len;
                    while ((len = zin.read(bf)) != -1) {
                        fout.write(bf, 0, len);
                    }
                    fout.close();
                    zin.closeEntry();
                }

                zin.close();
                zippedModeled.delete();

            } catch (IOException e) {

                Log.d("Downloading files", "IO EXception" + e.getMessage());
            }
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            addCPACSFilesToDrawer();
            setSupportProgressBarIndeterminateVisibility(false);
            Toast.makeText(getBaseContext(),
                    "Sample Models have been downloaded", Toast.LENGTH_SHORT)
                    .show();
        }

    }

    /**
     * Function to call to show the download Fragment that asks the user if
     * he/she wants to download sample files or not
     */
    public void showDownloadPrompt() {
        NoticeFragment dialog = new NoticeFragment();
        dialog.show(getFragmentManager(), "downloadFragment");
    }

    /**
     * HandlerFunctions to run whenever the user chooses whether to download the
     * sample files or not.
     */
    @Override
    public void onYesClick(DialogFragment dialog) {
        downloadAction();
    }

    @Override
    public void onNoClick(DialogFragment dialog) {
        addCPACSFilesToDrawer();
    }

    public void downloadAction() {
        URL downloadingLink = null;
        try {
            downloadingLink = new URL(
                    "http://sourceforge.net/projects/tigl/files/DevTools/TiGL-SampleModels.zip");
        } catch (MalformedURLException e) {
            e.printStackTrace();
        }
        if (isConnectedToInternet()) {
            new DownloadFilesThread().execute(downloadingLink);
        } else {
            Toast.makeText(
                    getBaseContext(),
                    "No internet connection. Check internet settings and try again.",
                    Toast.LENGTH_SHORT).show();
        }
    }
}
