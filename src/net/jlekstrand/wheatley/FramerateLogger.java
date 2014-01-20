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

import android.util.Log;

class FramerateLogger
{
    private static final String LOG_TAG = "wheatley:FramerateLogger";

    private final long _logFrequency;
    private final long _unitsPerSecond;
    private long _lastLoggedTime;
    private int _framesSinceLastLog;

    public FramerateLogger(long logFrequency, long unitsPerSecond)
    {
        _logFrequency = logFrequency;
        _unitsPerSecond = unitsPerSecond;
        _lastLoggedTime = -1;
        _framesSinceLastLog = 0;
    }

    public void frame(long frameTime)
    {
        if (_lastLoggedTime < 0) {
            _lastLoggedTime = frameTime;
            _framesSinceLastLog = 0;
        } else {
            _framesSinceLastLog++;
        }

        if (frameTime > _lastLoggedTime + _logFrequency) {
            double duration = frameTime - _lastLoggedTime;
            duration = duration / _unitsPerSecond;

            Log.d(LOG_TAG, String.format("%d frames in %.1f seconds = %.3f FPS",
                    _framesSinceLastLog, duration, _framesSinceLastLog / duration));

            // Reset
            _framesSinceLastLog = 0;
            _lastLoggedTime = frameTime;
        }
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:

