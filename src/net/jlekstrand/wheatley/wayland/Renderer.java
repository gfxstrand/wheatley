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
package net.jlekstrand.wheatley.wayland;

import android.view.Surface;

public class Renderer
{
    private static final String LOG_TAG = "wheatley:wayland.Renderer";

    private long _nativeHandle;

    private static native long createNative(long compositorHandle);
    private static native void destroyNative(long nativeHandle);
    private static native void addOutputNative(long nativeHandle,
            long outputHandle, Surface surface);
    private static native void removeOutputNative(long nativeHandle,
            long outputHandle);
    private static native void repaintOutputNative(long nativeHandle,
            long outputHandle, boolean forceRepaint);

    public Renderer(Compositor compositor)
    {
        _nativeHandle = createNative(compositor.getNativeHandle());
    }

    public void addOutput(Output output, Surface surface)
    {
        addOutputNative(_nativeHandle, output.getNativeHandle(), surface);
    }

    public void removeOutput(Output output)
    {
        removeOutputNative(_nativeHandle, output.getNativeHandle());
    }

    public void repaintOutput(Output output, boolean forceRepaint)
    {
        repaintOutputNative(_nativeHandle, output.getNativeHandle(),
                forceRepaint);
    }

    public void destroy()
    {
        if (_nativeHandle != 0)
            destroyNative(_nativeHandle);
        _nativeHandle = 0;
    }

    @Override
    public void finalize() throws Throwable
    {
        destroy();

        super.finalize();
    }

    static {
        System.loadLibrary("wheatley");
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:

