package net.jlekstrand.wheatley;

import net.jlekstrand.wheatley.config.Client;

class Compositor
{
    private static final String LOG_TAG = "wheatley:Compositor";

    private long _nativeHandle;

    private static native long createNative();
    private static native void destroyNative(long nativeHandle);
    private static native void launchClientNative(long nativeHandle, byte[] command);
    private static native void addToLooperNative(long nativeHandle);
    private static native void removeFromLooperNative(long nativeHandle);

    public Compositor()
    {
        _nativeHandle = createNative();
    }

    public Compositor(Client client)
    {
        this();
        launchClient(client);
    }

    long getNativeHandle()
    {
        return _nativeHandle;
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
