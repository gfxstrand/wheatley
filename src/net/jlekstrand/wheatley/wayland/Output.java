package net.jlekstrand.wheatley.wayland;

import android.util.DisplayMetrics;
import android.view.Display;

public class Output
{
    private static final String LOG_TAG = "wheatley:wayland.Output";

    private long _nativeHandle;

    private static native long createNative(long seatHandle,
            int physicalWidth, int physicalHeight);
    private static native void destroyNative(long nativeHandle);
    private static native void setModeNative(long nativeHandle,
            int width, int height, int refresh);

    private int _refresh;

    Output(Compositor compositor, Display display)
    {
        DisplayMetrics metrics = new DisplayMetrics();
        display.getRealMetrics(metrics);

        float width = ((metrics.widthPixels / metrics.xdpi) * 25.4f);
        float height = ((metrics.heightPixels / metrics.ydpi) * 25.4f);

        _nativeHandle = createNative(compositor.getNativeHandle(),
                Math.round(width), Math.round(height));

        setMode(metrics.widthPixels, metrics.heightPixels,
                Math.round(display.getRefreshRate() * 1000));
    }

    public long getNativeHandle()
    {
        return _nativeHandle;
    }

    public void setMode(int width, int height)
    {
        setModeNative(_nativeHandle, width, height, _refresh);
    }

    public void setMode(int width, int height, int refresh)
    {
        _refresh = refresh;
        setModeNative(_nativeHandle, width, height, refresh);
    }

    @Override
    public void finalize() throws Throwable
    {
        if (_nativeHandle != 0)
            destroyNative(_nativeHandle);

        super.finalize();
    }
}

// vim: ts=4 sw=4 sts=4 expandtab
