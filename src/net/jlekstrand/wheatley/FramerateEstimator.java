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
 *
 * vim: ts=4 sw=4 sts=4 expandtab
 */
package net.jlekstrand.wheatley;

class FramerateEstimator
{
    private long _times[];
    private int _firstTimePos;
    private int _numTimes;

    public FramerateEstimator(int pastFrameCount)
    {
        _times = new long[pastFrameCount + 1];
        _firstTimePos = 0;
        _numTimes = 0;
    }

    public void addFrameTime(long frameTime)
    {
        if (_numTimes < _times.length) {
            /*
             * The array is not full.  In this case, _firstTimePos == 0 and
             * we can simply place the new element at _numTimes.
             */
            _times[_numTimes] = frameTime;
            _numTimes++;
        } else {
            /*
             * The array is full.  In this case, we simply replace the
             * first element in the array and increment the counter
             */
            _times[_firstTimePos] = frameTime;
            _firstTimePos++;
            if (_firstTimePos == _times.length)
                _firstTimePos = 0;
        }
    }

    public long getEstimate()
    {
        final long first = _times[_firstTimePos];
        final long last;

        if (_numTimes < 2)
            return 0; // Calculation doesn't make sense

        if (_numTimes < _times.length) {
            /*
             * The array is not full.  In this case, _firstTimePos == 0 and
             * the last element is at _numTimes - 1
             */
            last = _times[_numTimes - 1];
        } else {
            /*
             * The array is full.  In this case, _firstTimePos could be
             * anything and the last element is the one directly in front
             * of it.
             */
            if (_firstTimePos > 0)
                last = _times[_firstTimePos - 1];
            else
                last = _times[_times.length - 1];
        }

        return (last - first) / (_numTimes - 1);
    }
}
