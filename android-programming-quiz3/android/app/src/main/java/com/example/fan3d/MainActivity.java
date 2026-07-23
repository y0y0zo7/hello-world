/**
 * @file MainActivity.java
 * @brief Hosts the Android GL surface and forwards touch input to native code.
 * @author Zoey <t.zoey@digipen.edu>
 */

package com.example.fan3d;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {
    private static final String TAG = "Fan3D";

    static {
        System.loadLibrary("fan3d");
    }

    private GLSurfaceView glSurfaceView;
    private volatile boolean rendererInitialized;

    private native boolean nativeInit(AssetManager assetManager);
    private native void nativeResize(int width, int height);
    private native void nativeRender();
    private native void nativeTouchDown(float x, float y);
    private native void nativeTouchMove(float x, float y);
    private native void nativeTouchRelease(float x, float y);

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);

        glSurfaceView = findViewById(R.id.gl_surface_view);
        glSurfaceView.setEGLContextClientVersion(3);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                rendererInitialized = nativeInit(getAssets());
                Log.i(TAG, rendererInitialized ? "Renderer ready" : "Renderer initialization failed");
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                if (rendererInitialized) nativeResize(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                if (rendererInitialized) nativeRender();
            }
        });
        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        glSurfaceView.setOnTouchListener((view, event) -> {
            if (!rendererInitialized) return true;
            final float x = event.getX();
            final float y = event.getY();
            switch (event.getActionMasked()) {
                case MotionEvent.ACTION_DOWN:
                    glSurfaceView.queueEvent(() -> nativeTouchDown(x, y));
                    break;
                case MotionEvent.ACTION_MOVE:
                    glSurfaceView.queueEvent(() -> nativeTouchMove(x, y));
                    break;
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_CANCEL:
                    glSurfaceView.queueEvent(() -> nativeTouchRelease(x, y));
                    break;
                default:
                    break;
            }
            return true;
        });
    }

    @Override
    protected void onPause() {
        glSurfaceView.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        glSurfaceView.onResume();
    }
}
