/*
 * Copyright © 2014 Jason Ekstrand
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
    private static native void prepareFrameNative(long nativeHandle);
    private static native void frameCompleteNative(long nativeHandle,
            int timestamp);

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

    public void prepareFrame()
    {
        prepareFrameNative(_nativeHandle);
    }

    public void frameComplete(int timestamp)
    {
        frameCompleteNative(_nativeHandle, timestamp);
    }

    public void destroy()
    {
        if (_nativeHandle == 0)
            return;

        destroyNative(_nativeHandle);
        _nativeHandle = 0;
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:
