package net.jlekstrand.wheatley;

import java.lang.reflect.Field;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Choreographer;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import net.jlekstrand.wheatley.config.Client;

public class WaylandActivity extends Activity
        implements SurfaceHolder.Callback2
{
    private static final String LOG_TAG = "wheatley:WaylandActivity";

    private static final int WL_POINTER_AXIS_VERTICAL_SCROLL = 0;
    private static final int WL_POINTER_AXIS_HORIZONTAL_SCROLL = 1;

    private static final int BTN_LEFT = 0x110;
    private static final int BTN_RIGHT = 0x111;
    private static final int BTN_MIDDLE = 0x112;
    private static final int BTN_FORWARD = 0x115;
    private static final int BTN_BACK = 0x116;

    private long _nativeHandle;

    private static native long createNative(long compositorHandle);
    private static native void destroyNative(long nativeHandle);
    private static native void surfaceCreatedNative(long nativeHandle,
            Surface surface);
    private static native void surfaceChangedNative(long nativeHandle,
            int format, int w, int h);
    private static native void surfaceDestroyedNative(long nativeHandle);
    private static native void repaintNative(long nativeHandle,
            boolean forceRepaint);
    private static native void repaintFinishedNative(long nativeHandle,
            int timestamp);

    private static native void pointerEnterNative(long nativeHandle,
            float x, float y);
    private static native void pointerMotionNative(long nativeHandle,
            int time, float x, float y);
    private static native void pointerButtonNative(long nativeHandle,
            int time, int button, boolean pressed);
    private static native void pointerAxisNative(long nativeHandle,
            int time, int axis, float value);
    private static native void pointerLeaveNative(long nativeHandle);

    private static native void touchDownNative(long nativeHandle,
            int time, int id, float x, float y);
    private static native void touchMoveNative(long nativeHandle,
            int id, float x, float y);
    private static native void touchFinishFrameNative(long nativeHandle,
            int time);
    private static native void touchUpNative(long nativeHandle,
            int time, int id);
    private static native void touchCancelNative(long nativeHandle);

    private Compositor _compositor;
    private Choreographer _choreographer;
    private SurfaceView _surfaceView;
    private Surface _surface;

    private int pointerButtonState;

    private FramerateEstimator _rateEstimator;

    private boolean _repaintScheduled;

    @Override
    public void
    onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();

        _surfaceView = new SurfaceView(this);
        _surfaceView.getHolder().addCallback(this);
        setContentView(_surfaceView);

        _choreographer = Choreographer.getInstance();

        // Estimate the framerate based on the past 10 frames
        _rateEstimator = new FramerateEstimator(10);

        Uri clientUri = intent.getData();
        if (clientUri == null) {
            finish();
            return;
        }

        Client client = Client.createForUri(this, null, clientUri);
        if (client == null) {
            finish();
            return;
        }

        _compositor = new Compositor(client);
        _compositor.addToLooper();

        _nativeHandle = createNative(_compositor.getNativeHandle());

        _repaintScheduled = false;

        pointerButtonState = 0;
    }

    private final Choreographer.FrameCallback _repaintCallback = 
        new Choreographer.FrameCallback() {
            @Override
            public void doFrame(long frameTimeNanos)
            {
                _repaintScheduled = false;

                // It is possible that the surface was removed after the
                // callback was posted.  In this case, we just return early.
                if (_surface == null)
                    return;

                repaintNative(_nativeHandle, false);

                _rateEstimator.addFrameTime(frameTimeNanos);
                long nextTime = frameTimeNanos + _rateEstimator.getEstimate();
                repaintFinishedNative(_nativeHandle, (int)(nextTime / 1000000));

                scheduleRepaint();
            }
        };

    private void
    scheduleRepaint()
    {
        if (_repaintScheduled || _surface == null)
            return; // Can't schedule at this time

        _choreographer.postFrameCallback(_repaintCallback);

        _repaintScheduled = true;
    }

    @Override
    public void
    onDestroy()
    {
        destroyNative(_nativeHandle);
        _nativeHandle = 0;

        super.onDestroy();
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
        _surface = holder.getSurface();
        surfaceCreatedNative(_nativeHandle, _surface);
        scheduleRepaint();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        surfaceChangedNative(_nativeHandle, format, w, h);
        scheduleRepaint();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        surfaceDestroyedNative(_nativeHandle);
        _surface = null;
    }

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder)
    {
        Log.d(LOG_TAG, "surfaceRedrawNeeded()");
        scheduleRepaint();
    }

    public boolean onMouseEvent(MotionEvent event)
    {
        if ((event.getSource() & InputDevice.SOURCE_MOUSE) == 0)
            return false;

        switch (event.getActionMasked()) {
        case MotionEvent.ACTION_HOVER_ENTER:
            // Note that we never handle ACTION_HOVER_LEAVE. This is
            // because, for some insane reason, Android thinks it needs to
            // send one right before a button press.
            pointerEnterNative(_nativeHandle, event.getX(), event.getY());
            return true;
        case MotionEvent.ACTION_MOVE:
        case MotionEvent.ACTION_HOVER_MOVE:
            pointerMotionNative(_nativeHandle, (int)event.getEventTime(),
                    event.getX(), event.getY());
            return true;
        case MotionEvent.ACTION_DOWN: {
            int buttons = event.getButtonState() & (~pointerButtonState);

            if ((buttons & MotionEvent.BUTTON_PRIMARY) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_LEFT, true);

            if ((buttons & MotionEvent.BUTTON_SECONDARY) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_RIGHT, true);

            if ((buttons & MotionEvent.BUTTON_TERTIARY) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_MIDDLE, true);

            if ((buttons & MotionEvent.BUTTON_BACK) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_BACK, true);

            if ((buttons & MotionEvent.BUTTON_FORWARD) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_FORWARD, true);

            pointerButtonState = event.getButtonState();
        } return true;
        case MotionEvent.ACTION_UP: {
            int buttons = pointerButtonState & (~event.getButtonState());

            if ((buttons & MotionEvent.BUTTON_PRIMARY) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_LEFT, false);

            if ((buttons & MotionEvent.BUTTON_SECONDARY) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_RIGHT, false);

            if ((buttons & MotionEvent.BUTTON_TERTIARY) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_MIDDLE, false);

            if ((buttons & MotionEvent.BUTTON_BACK) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_BACK, false);

            if ((buttons & MotionEvent.BUTTON_FORWARD) != 0)
                pointerButtonNative(_nativeHandle, (int)event.getEventTime(),
                        BTN_FORWARD, false);

            pointerButtonState = event.getButtonState();
        } return true;
        case MotionEvent.ACTION_SCROLL: {
            int time = (int)event.getEventTime();

            float xval = event.getAxisValue(MotionEvent.AXIS_X);
            if (xval != 0)
                pointerAxisNative(_nativeHandle, time,
                        WL_POINTER_AXIS_HORIZONTAL_SCROLL, xval);

            float yval = event.getAxisValue(MotionEvent.AXIS_Y);
            if (yval != 0)
                pointerAxisNative(_nativeHandle, time,
                        WL_POINTER_AXIS_VERTICAL_SCROLL, xval);

        }   return true;
        }

        return false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        if ((event.getSource() & InputDevice.SOURCE_MOUSE) != 0)
            return onMouseEvent(event);

        switch (event.getActionMasked()) {
        case MotionEvent.ACTION_DOWN:
        case MotionEvent.ACTION_POINTER_DOWN: {
            final int idx = event.getActionIndex();
            touchDownNative(_nativeHandle, (int)event.getEventTime(),
                    event.getPointerId(idx), event.getX(idx), event.getY(idx));
        }   return true;
        case MotionEvent.ACTION_MOVE: {
            final int historySize = event.getHistorySize();
            final int pointerCount = event.getPointerCount();

            for (int h = 0; h < historySize; h++) {
                for (int p = 0; p < pointerCount; p++) {
                    touchMoveNative(_nativeHandle,
                            event.getPointerId(p),
                            event.getHistoricalX(p, h),
                            event.getHistoricalY(p, h));
                }
                touchFinishFrameNative(_nativeHandle,
                        (int)event.getHistoricalEventTime(h));
            }

            for (int p = 0; p < pointerCount; p++) {
                Log.d(LOG_TAG, "Touch Move: (" + event.getX(p) + ", " + event.getY(p) + ")");
                touchMoveNative(_nativeHandle, event.getPointerId(p),
                        event.getX(p), event.getY(p));
            }
            touchFinishFrameNative(_nativeHandle, (int)event.getEventTime());
        }   return true;
        case MotionEvent.ACTION_UP:
        case MotionEvent.ACTION_POINTER_UP:
            touchUpNative(_nativeHandle, (int)event.getEventTime(),
                    event.getPointerId(event.getActionIndex()));
            return true;
        case MotionEvent.ACTION_CANCEL:
            touchCancelNative(_nativeHandle);
            return true;
        }

        return false;
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event)
    {
        if ((event.getSource() & InputDevice.SOURCE_MOUSE) != 0)
            return onMouseEvent(event);

        return false;
    }

    static {
        System.loadLibrary("wheatley");
    }
}

// vim: ts=4 sw=4 sts=4 expandtab
