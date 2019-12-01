package com.flyscale.chapter_4_3;

import android.view.Surface;

public class PngView {
    public native String getStringFromJNI(String text);

    public native void init(String pngPath);

    public native void setSurface(Surface surface);

    public native void resetSize(int width, int height);

    public native void stop();
}
