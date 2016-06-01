/**
 * Created by Vishal Gaurav
 */
package com.dartcync;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.support.v7.app.AppCompatActivity;
import android.widget.Toast;

/**
 * Common parent for most of the activities of this app.
 * should not be used as an activity to display layouts.
 */
public class BaseActivity extends AppCompatActivity {


    private ProgressDialog mProgressDialog = null;
    private AlertDialog mAlertDialog = null;

    /**
     * @param progressMessage
     * @param isCancelable
     */
    protected void showProgressDialog(String progressMessage, boolean isCancelable) {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(BaseActivity.this);
        }
        mProgressDialog.setIndeterminate(true);
        mProgressDialog.setCancelable(isCancelable);
        mProgressDialog.setMessage(progressMessage);
        mProgressDialog.show();
    }

    /**
     * @param title
     * @param alertMessage
     * @param isCancelable
     */
    protected void showAlertDialog(String title, String alertMessage, boolean isCancelable) {
        if (mAlertDialog == null) {
            mAlertDialog = getNewAlertDialog();
        }
        mAlertDialog.setCancelable(isCancelable);
        mAlertDialog.setTitle(title);
        mAlertDialog.setMessage(alertMessage);
        mAlertDialog.show();

    }

    /**
     *
     * @return
     */
    private AlertDialog getNewAlertDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(BaseActivity.this);
        builder.setPositiveButton("Ok", null);
        return builder.create();
    }

    @Override
    protected void onDestroy() {
        dismissProgressDialog();
        dismissAlertDialog();
        super.onDestroy();
    }

    protected void dismissAlertDialog() {
        if (mAlertDialog != null && mAlertDialog.isShowing()) {
            mAlertDialog.dismiss();
        }
        mAlertDialog = null;
    }

    protected void dismissProgressDialog() {
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
        mProgressDialog = null;
    }

    public void showToast(String msg){
        Toast.makeText(BaseActivity.this, msg, Toast.LENGTH_SHORT).show();
    }
}
