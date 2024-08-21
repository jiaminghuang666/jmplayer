package com.example.JMPlayer;

import android.content.pm.ActivityInfo;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;
import android.widget.SeekBar;

import androidx.fragment.app.FragmentActivity;

/**
 * Loads {@link PlaybackVideoFragment}.
 */
public class PlaybackActivity extends FragmentActivity implements SeekBar.OnSeekBarChangeListener{
    private static final String TAG = "jiaming PlaybackActivity";

    private static JMPlayer myJMplayer;

    private SeekBar seek_position;
    private static int duration = 0;
    private  Thread playbackThread = null;
    static {
        System.loadLibrary("JMPlayer");
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //去掉标题栏
        //supportRequestWindowFeature( Window.FEATURE_NO_TITLE);
        //全屏，隐藏状态
        getWindow().setFlags( WindowManager.LayoutParams.FLAG_FULLSCREEN ,
                WindowManager.LayoutParams.FLAG_FULLSCREEN
        );
        //屏幕为横屏
        setRequestedOrientation( ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE );

        setContentView(R.layout.activity_playback);


        myJMplayer = new JMPlayer(this);

        myJMplayer.testjni("ceshi fanshe",this);

        /*if (savedInstanceState == null) {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.PlaybackVideo_fragment, new PlaybackVideoFragment())
                    .commit();
        }*/
        seek_position = findViewById(R.id.seekBar);
        seek_position.setMax(100);
        seek_position.setOnSeekBarChangeListener((SeekBar.OnSeekBarChangeListener) this);

        if (playbackThread == null) {
            playbackThread = new Thread(runnable);
            playbackThread.start();
        }
    }

    private  Runnable runnable = new Runnable(){
        @Override
        public void run() {
            Log.d(TAG, "jiaming java thread open " );
            Open("/data/1080.mp4", this);

            duration = myJMplayer.getDuration();
            Log.d(TAG, "jiaming video duration: " + duration);


            //Open("rtmp://live.hkstv.hk.lxdns.com/live/hks",this);
            //Open("https://commondatastorage.googleapis.com/android-tv/Sample%20videos/Zeitgeist/Zeitgeist%202010_%20Year%20in%20Review.mp4",this);
            for(;;){
                seek_position.setProgress((int)(myJMplayer.getCurrentPosition() * 1000 ));
                try {
                    Thread.sleep(40);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }

        }
    };

    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        myJMplayer.Seek((double)seekBar.getProgress() / (double)seekBar.getMax());
    }

    public void testFanShe(String msg ){
        Log.d(TAG, "JMPlayer testFanShe: "  + msg);
        return ;
    }

    public native boolean Open(String url, Object handle);




}
