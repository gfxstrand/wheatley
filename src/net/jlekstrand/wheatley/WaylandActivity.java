/*
 * Copyright © 2013-2014 Jason Ekstrand
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
package net.jlekstrand.wheatley;

import java.lang.reflect.Field;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Choreographer;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import net.jlekstrand.wheatley.wayland.Compositor;
import net.jlekstrand.wheatley.wayland.Output;
import net.jlekstrand.wheatley.config.Client;

public class WaylandActivity extends Activity
        implements SurfaceHolder.Callback2
{
    private static final String LOG_TAG = "wheatley:WaylandActivity";

    private long _nativeHandle;

    private static native long createNative(long compositorHandle);
    private static native void destroyNative(long nativeHandle);
    private static native void surfaceCreatedNative(long nativeHandle,
            long outputHandle, Surface surface);
    private static native void surfaceDestroyedNative(long nativeHandle,
            long outputHandle);
    private static native void repaintNative(long nativeHandle,
            long outputHandle, boolean forceRepaint);
    private static native void repaintFinishedNative(long nativeHandle,
            long outputHandle, int timestamp);

    private Compositor _compositor;
    private Output _output;
    private Choreographer _choreographer;
    private Surface _surface;

    private FramerateEstimator _rateEstimator;

    private boolean _repaintScheduled;

    @Override
    public void
    onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();

        setContentView(new View(this));
        getWindow().takeSurface(this);

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

        _compositor = new Compositor(this, client);
        _compositor.addToLooper();
        _output = _compositor.getPrimaryOutput();

        _nativeHandle = createNative(_compositor.getNativeHandle());

        _repaintScheduled = false;
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

                repaintNative(_nativeHandle, _output.getNativeHandle(), false);

                _rateEstimator.addFrameTime(frameTimeNanos);
                long nextTime = frameTimeNanos + _rateEstimator.getEstimate();
                repaintFinishedNative(_nativeHandle, _output.getNativeHandle(),
                        (int)(nextTime / 1000000));

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
        surfaceCreatedNative(_nativeHandle, _output.getNativeHandle(),
                _surface);
        scheduleRepaint();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        _output.setMode(w, h);
        scheduleRepaint();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        surfaceDestroyedNative(_nativeHandle, _output.getNativeHandle());
        _surface = null;
    }

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder)
    {
        Log.d(LOG_TAG, "surfaceRedrawNeeded()");
        scheduleRepaint();
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent event)
    {
        boolean consumed = false;
        if (_compositor != null)
            consumed = _compositor.onMotionEvent(event);

        if (consumed)
            return true;
        else
            return super.dispatchTouchEvent(event);
    }

    @Override
    public boolean dispatchGenericMotionEvent(MotionEvent event)
    {
        boolean consumed = false;
        if (_compositor != null)
            consumed = _compositor.onMotionEvent(event);

        if (consumed)
            return true;
        else
            return super.dispatchGenericMotionEvent(event);
    }

    @Override
    public boolean dispatchKeyEvent(KeyEvent event)
    {
        boolean consumed = false;
        if (_compositor != null)
            consumed = _compositor.onKeyEvent(event);

        if (consumed)
            return true;
        else
            return super.dispatchKeyEvent(event);
    }

    static {
        System.loadLibrary("wheatley");
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:
