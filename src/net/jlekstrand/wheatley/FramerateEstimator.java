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
