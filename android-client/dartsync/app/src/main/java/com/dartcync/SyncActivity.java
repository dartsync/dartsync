package com.dartcync;

import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.FileObserver;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v7.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.io.File;
import java.util.ArrayList;

public class SyncActivity extends BaseActivity {

    /**
     * The {@link android.support.v4.view.PagerAdapter} that will provide
     * fragments for each of the sections. We use a
     * {@link FragmentPagerAdapter} derivative, which will keep every
     * loaded fragment in memory. If this becomes too memory intensive, it
     * may be best to switch to a
     * {@link android.support.v4.app.FragmentStatePagerAdapter}.
     */
    private FileObserver fileObserver = null;
    private TrackerInfo trackerInfo = null;
    private boolean isRunning = false;
    private File defaultDir = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/" + Constants.DEFAULT_DIR);
    private Client client = null;
    private ListView list = null;
    private ArrayList<File> fileList = new ArrayList<>();
    private FileViewAdapter adapter = null;
    public TrackerInfo getTrackerInfo(){
        return trackerInfo;
    }

    public File getRootDir(){
        return defaultDir;
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sync);
        list = (ListView)findViewById(R.id.list);
        adapter = new FileViewAdapter();
        list.setAdapter(adapter);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        // Create the adapter that will return a fragment for each of the three
        // primary sections of the activity.

        // Set up the ViewPager with the sections adapter.
        System.out.println("Default dir = " + defaultDir.getAbsolutePath());
        updateFileList();

    }

    private void updateFileList(){
        fileList.clear();
        File[] filearr = defaultDir.listFiles();
        if(filearr!=null) {
            for (int i = 0; i < filearr.length; i++) {
                fileList.add(filearr[i]);
            }
        }
        adapter.notifyDataSetChanged();
    }

    private void setFileObserver(){
        if(!defaultDir.exists()){
            defaultDir.mkdir();
        }
        fileObserver = new FileObserver(defaultDir.getAbsolutePath()){
            @Override
            public void onEvent(int event, String file) {
                System.out.println("onEvent File :- ");
                if(client != null) {
                    switch (event){
                        case FileObserver.CREATE: {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    updateFileList();
                                }
                            });
                            client.onFileCreate(new File(file));
                        }
                        break;
                        case FileObserver.DELETE: {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    updateFileList();
                                }
                            });
                            client.onFileDelete(new File(file));
                        }
                        break;
                        case FileObserver.MODIFY: {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    updateFileList();
                                }
                            });
                            client.onFileUpdate(new File(file));
                        }
                        break;
                    }
                }
            }
        };
        fileObserver.startWatching();
    }

    private void removeFileObserver(){
        if(fileObserver!=null){
            fileObserver.stopWatching();
            fileObserver  = null;
        }
    }

    private void startClient(TrackerInfo result){
        setFileObserver();
        trackerInfo = result;
        client = new Client(trackerInfo,defaultDir);
        client.startClient();
    }

    private void stopClient(){
        removeFileObserver();
        if(client != null){
            client.stopClient();
            client = null;
        }
    }
    @Override
    public void onDestroy(){
        super.onDestroy();
        stopClient();
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_sync, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            if(client == null){
                item.setTitle(getString(R.string.action_stop));
                new ConnectionTask().execute("tahoe.cs.dartmouth.edu");
            }else{
                item.setTitle(getString(R.string.action_start));
                client.stopClient();
                client = null;
            }
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private class FileViewAdapter extends BaseAdapter{



        @Override
        public int getCount() {
            return fileList.size();
        }

        @Override
        public File getItem(int position) {
            return fileList.get(position);
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if(convertView == null){
                convertView = LayoutInflater.from(getBaseContext()).inflate(R.layout.row_list,null);
            }
            File file = getItem(position);
           TextView txtName = (TextView) convertView.findViewById(R.id.txtFileName);
            TextView txtTime = (TextView) convertView.findViewById(R.id.txtFileTime);
            TextView txtSize = (TextView) convertView.findViewById(R.id.txtFileSize);
            txtName.setText(file.getName());
            txtTime.setText(""+file.lastModified());
            txtSize.setText(""+file.length() + " bytes");
            return convertView;
        }
    }

    private class ConnectionTask extends AsyncTask<String,Void,TrackerInfo>{

        @Override
        protected void onPreExecute() {
            showProgressDialog("Connecting to Client",false);
            super.onPreExecute();
        }

        @Override
        protected void onPostExecute(TrackerInfo result) {
            dismissProgressDialog();
            if(result != null){
                startClient(result);
            }
            super.onPostExecute(trackerInfo);
        }

        @Override
        protected void onCancelled() {
            super.onCancelled();
        }

        @Override
        protected TrackerInfo doInBackground(String... params) {
            String trackerAddress = params[0];
            return Client.connectToTracker(trackerAddress);
        }
    }
}
