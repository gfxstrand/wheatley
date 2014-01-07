package net.jlekstrand.wheatley.wayland;

public class Seat
{
    private static final String LOG_TAG = "wheatley:wayland.Seat";

    private long _nativeHandle;

    private static native long createNative(long compositorHandle);
    private static native void destroyNative(long nativeHandle);

    Seat(Compositor compositor)
    {
        _nativeHandle = createNative(compositor.getNativeHandle());
    }

    long getNativeHandle()
    {
        return _nativeHandle;
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
