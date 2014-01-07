package net.jlekstrand.wheatley.wayland;

import android.content.Context;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.WindowManager;

import net.jlekstrand.wheatley.config.Client;

public class Compositor
{
    private static final String LOG_TAG = "wheatley:wayland.Compositor";

    private long _nativeHandle;

    private static native long createNative();
    private static native void destroyNative(long nativeHandle);
    private static native void launchClientNative(long nativeHandle, byte[] command);
    private static native void addToLooperNative(long nativeHandle);
    private static native void removeFromLooperNative(long nativeHandle);

    private static native void hasKeyboardNative(long nativeHandle,
            boolean hasKeyboard);
    private static native void hasTouchNative(long nativeHandle,
            boolean hasTouch);
    private static native void hasPointerNative(long nativeHandle,
            boolean hasPointer);

    Output _output;
    Seat _seat;
    Touch _touch;
    Pointer _pointer;

    public Compositor(Context context)
    {
        WindowManager wm =
                (WindowManager)context.getSystemService(Context.WINDOW_SERVICE);

        _nativeHandle = createNative();

        _output = new Output(this, wm.getDefaultDisplay());
        _seat = new Seat(this);
        _touch = new Touch(_seat);
        _pointer = new Pointer(_seat);
    }

    public Compositor(Context context, Client client)
    {
        this(context);
        launchClient(client);
    }

    public long getNativeHandle()
    {
        return _nativeHandle;
    }

    public Output getPrimaryOutput()
    {
        return _output;
    }

    public void launchClient(Client client)
    {
        launchClientNative(_nativeHandle, client.getCommand().getBytes());
    }

    public void addToLooper()
    {
        addToLooperNative(_nativeHandle);
    }

    public void removeFromLooper()
    {
        removeFromLooperNative(_nativeHandle);
    }

    private boolean onMotionEvent(MotionEvent event, Output output)
    {
        if ((event.getSource() & InputDevice.SOURCE_MOUSE) != 0)
            return _pointer.onMotionEvent(event, output);
        else if ((event.getSource() & InputDevice.SOURCE_TOUCHSCREEN) != 0)
            return _touch.onTouchEvent(event, output);

        return false;
    }

    public boolean onMotionEvent(MotionEvent event)
    {
        return onMotionEvent(event, _output);
    }

    public boolean onKeyEvent(KeyEvent event)
    {
        return false;
    }

    @Override
    public void finalize() throws Throwable
    {
        if (_nativeHandle != 0)
            destroyNative(_nativeHandle);

        super.finalize();
    }

    static {
        System.loadLibrary("wheatley");
    }
}

// vim: ts=4 sw=4 sts=4 expandtab
