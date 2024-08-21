package com.example.JMPlayer;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class JMPlayer extends GLSurfaceView implements Runnable,SurfaceHolder.Callback, GLSurfaceView.Renderer, View.OnClickListener  {

    private static final String TAG = "jiaming JMPlayer";
    private static int duration = 0;

    public JMPlayer(Context context){
        super(context);
        setRenderer(this);
        setOnClickListener( this );
    }

    public JMPlayer(Context context, AttributeSet attrs) {
        super(context, attrs);
        setRenderer(this);
        setOnClickListener( this );
    }

    @Override
    public void run() {

        duration = getDuration();
        Log.d(TAG, "JMPlayer thread run duration " + duration);
        int width = getVideoWidth();
        int height = getVideoHeight();
        Log.d(TAG, "JMPlayer width: " + width + " height:"+ height);

    }

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
        InitView(holder.getSurface());
        new Thread( this ).start();
    }


    @Override
    public void surfaceChanged(SurfaceHolder var1, int var2, int var3, int var4)
    {

    }
    @Override
    public void surfaceDestroyed(SurfaceHolder var1)
    {

    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {

    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {

    }

    @Override
    public void onDrawFrame(GL10 gl) {

    }


    public void testFanShe(String msg ){
        Log.d(TAG, "JMPlayer testFanShe: "  + msg);
        return ;
    }

    public void postEventFromNative(int what, int arg1, int arg2) {

    }

    @Override
    public void onClick(View view) {
        Pause(true);
    }


    public  native void InitView(Object surface);

    public native boolean setDataSource(String url, Object handle);
    public native void start();

    public native void Pause(boolean isPause);
    public native void Seek(double Position);

    public native int getDuration();

    public native double getCurrentPosition();

    public native int getVideoWidth();
    public native int getVideoHeight();


    public native boolean testjni(String url, Object handle);

}
