package com.flyscale.chapter_4_3;

import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

public class PngViewActivity extends AppCompatActivity {

    private static final String TAG = "PngViewActivity";
    private LinearLayout mPngContainer;
    private SurfaceView surfaceView;
    private PngView pngView;
    private String pngPath = "/mnt/sdcard/1.png";

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.pngview_activity);
        pngView = new PngView();
        Log.d(TAG, "Receive:" + pngView.getStringFromJNI("Hello,I am Java!"));
        mPngContainer = findViewById(R.id.png_container);
        surfaceView = new SurfaceView(this);
        SurfaceHolder holder = surfaceView.getHolder();
        holder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                Log.d(TAG, "surfaceCreated");
                pngView.init(pngPath);
               /* try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }*/
                pngView.setSurface(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.d(TAG, "surfaceChanged,width=" + width + ",height=" + height);
                pngView.resetSize(width, height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                Log.d(TAG, "surfaceDestroyed");
                pngView.stop();
            }
        });
        mPngContainer.addView(surfaceView);
    }

}
