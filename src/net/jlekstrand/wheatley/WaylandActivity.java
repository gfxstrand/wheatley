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

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;
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
import net.jlekstrand.wheatley.wayland.Renderer;
import net.jlekstrand.wheatley.config.Client;

public class WaylandActivity extends Activity
        implements SurfaceHolder.Callback2, ServiceConnection
{
    private static final String LOG_TAG = "wheatley:WaylandActivity";

/*
    private long _nativeHandle;

    private static native long createNative();
    private static native void destroyNative(long nativeHandle);
    private static native void setCompositorNative(long nativeHandle,
            long compositorHandle);
    private static native void surfaceCreatedNative(long nativeHandle,
            long outputHandle, Surface surface);
    private static native void surfaceDestroyedNative(long nativeHandle,
            long outputHandle);
    private static native void repaintNative(long nativeHandle,
            long outputHandle, boolean forceRepaint);
    private static native void repaintFinishedNative(long nativeHandle,
            long outputHandle, int timestamp);
*/

    private Compositor _compositor;
    private Renderer _renderer;
    private Output _output;
    private Surface _surface;
    private int _swidth, _sheight;

    private Choreographer _choreographer;
    private FramerateEstimator _rateEstimator;
    private FramerateLogger _rateLogger;

    private CompositorService.Binder _compositorServiceBinder;
    private CompositorService _compositorService;
    private Handler _compositorServiceHandler;

    private boolean _repaintScheduled;

    @Override
    public void
    onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        setContentView(new View(this));
        getWindow().takeSurface(this);

        _choreographer = Choreographer.getInstance();

        // Estimate the framerate based on the past 10 frames
        _rateEstimator = new FramerateEstimator(10);
        _rateLogger = new FramerateLogger(5000000000L, 1000000000L);

        _repaintScheduled = false;
    }

    @Override
    public void
    onStart()
    {
        super.onStart();

        Log.d(LOG_TAG, "Activity started: " + getIntent());

        Intent intent = new Intent(this, CompositorService.class);
        bindService(intent, this, BIND_AUTO_CREATE);
    }

    @Override
    public void
    onStop()
    {
        destroyRenderer();

        _compositor.removeFromLooper();
        _compositorService.returnCompositor(_compositor);
        _compositor = null;

        unbindService(this);

        super.onStop();
    }

    private final Choreographer.FrameCallback _repaintCallback = 
        new Choreographer.FrameCallback() {
            @Override
            public void doFrame(long frameTimeNanos)
            {
                _repaintScheduled = false;

                if (_renderer == null)
                    return;

                _output.prepareFrame();
                _renderer.repaintOutput(_output, false);

                _rateLogger.frame(frameTimeNanos);
                _rateEstimator.addFrameTime(frameTimeNanos);
                long nextTime = frameTimeNanos + _rateEstimator.getEstimate();

                _output.frameComplete((int)(nextTime / 1000000));

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

    private void handleCompositorServiceMessage(Message msg)
    {
        Log.d(LOG_TAG, "Received service message");

        switch(msg.what) {
        case CompositorService.COMPOSITOR_READY:
            _compositor = (Compositor)msg.obj;
            _compositor.addToLooper();
            createRenderer();
            break;
        case CompositorService.COMPOSITOR_ERROR:
            Toast.makeText(this, R.string.compositor_failed, Toast.LENGTH_LONG);
            finish();
            break;
        }
    }

    @Override
    public void onServiceConnected(ComponentName className, IBinder binder)
    {
        Log.d(LOG_TAG, "Service Connected");

        _compositorServiceBinder = (CompositorService.Binder)binder;
        _compositorService = _compositorServiceBinder.getService();
        _compositorServiceHandler = new Handler() {
            @Override
            public void handleMessage(Message msg)
            {
                handleCompositorServiceMessage(msg);
            }
        };

        _compositorService.requestCompositor(getIntent().getData(),
                _compositorServiceHandler);
    }

    @Override
    public void onServiceDisconnected(ComponentName className)
    {
        Log.d(LOG_TAG, "Service Disconnected");
        
        _compositorService = null;
        _compositorServiceBinder = null;
        _compositorServiceHandler = null;
    }

    private void createRenderer()
    {
        if (_surface == null || _compositor == null)
            return;

        _renderer = new Renderer(_compositor);

        _output = _compositor.getPrimaryOutput();
        _output.setMode(_swidth, _sheight);

        _renderer.addOutput(_output, _surface);

        scheduleRepaint();
    }

    private void destroyRenderer()
    {
        if (_renderer == null)
            return;

        _renderer.destroy();
        _renderer = null;
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
        _surface = holder.getSurface();
        _swidth = w;
        _sheight = h;

        createRenderer();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        destroyRenderer();
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
