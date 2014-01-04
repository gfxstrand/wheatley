package net.jlekstrand.wheatley;

import java.lang.reflect.Field;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;

import net.jlekstrand.wheatley.config.Client;

public class WaylandActivity extends Activity implements SurfaceHolder.Callback2
{
    private static final String LOG_TAG = "wheatley:WaylandActivity";

    private long _nativeHandle;

/*
    private static native long createNative();
    private static native void destroyNative(long nativeHandle);
*/

    private Compositor _compositor;

    @Override
    public void
    onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Intent intent = getIntent();

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
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder)
    {
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h)
    {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder)
    {
    }

    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder)
    {
    }
}
