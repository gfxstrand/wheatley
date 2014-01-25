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

import java.util.HashMap;

import android.app.Service;
import android.content.Intent;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

import net.jlekstrand.wheatley.wayland.*;
import net.jlekstrand.wheatley.config.Client;
import net.jlekstrand.wheatley.config.DatabaseHelper;

public class CompositorService extends Service
{
    private static final String LOG_TAG = "wheatley:CompositorService";

    public static final int COMPOSITOR_READY = 1;
    public static final int COMPOSITOR_ERROR = 2;

    private static final int ATTACH_COMPOSITOR = 1001;
    private static final int DETATCH_COMPOSITOR = 1002;

    public class Binder extends android.os.Binder
    {
        public CompositorService getService()
        {
            return CompositorService.this;
        }
    }

    Binder _binder = new Binder();

    private class KeepAliveThread extends Thread
    {
        public Handler handler;
        
        @Override
        public void run()
        {
            Looper.prepare();

            synchronized (this) {
                handler = new Handler();
                notifyAll();
            }

            Looper.loop();
        }
    }

    private KeepAliveThread _keepAliveThread;

    private SQLiteDatabase _database;
    private HashMap<Uri, Compositor> _instanceMap;
    private int _lastStartId;

    @Override
    public void onCreate()
    {
        super.onCreate();

        DatabaseHelper helper = new DatabaseHelper(this);
        _database = helper.getReadableDatabase();

        _instanceMap = new HashMap<Uri, Compositor>();
    }

    @Override
    public IBinder onBind(Intent intent)
    {
        Log.d(LOG_TAG, "Service bound: " + intent);
        return _binder;
    }

    private void queueRunnable(Runnable r)
    {
        if (_keepAliveThread == null) {
            Log.d(LOG_TAG, "Creating keep-alive thread");

            _keepAliveThread = new KeepAliveThread();
            _keepAliveThread.start();

            synchronized (_keepAliveThread) {
                while(_keepAliveThread.handler == null) {
                    if (!_keepAliveThread.isAlive())
                        throw new RuntimeException("Keep-alive thread died unexpectedly");

                    try {
                        _keepAliveThread.wait();
                    } catch (InterruptedException e) {
                    }
                }
            }
        }

        if (!_keepAliveThread.handler.post(r))
            Log.wtf(LOG_TAG, "Failed to post runnable");
    }

    public synchronized boolean isRunning(Uri uri)
    {
        return _instanceMap.containsKey(uri);
    }

    // This method MUST be called from the keep-alive thread.
    private synchronized void fetchCompositor(Uri uri, Handler returnHandler)
    {
        if (!_instanceMap.containsKey(uri)) {
            returnHandler.sendMessage(
                    Message.obtain(returnHandler, COMPOSITOR_ERROR));
            return;
        }

        Compositor compositor = _instanceMap.get(uri);
        compositor.removeFromLooper();
        returnHandler.sendMessage(Message.obtain(returnHandler,
                COMPOSITOR_READY, compositor));
    }

    public synchronized void requestCompositor(final Uri uri,
            final Handler returnHandler)
    {
        if (_instanceMap.containsKey(uri)) {
            Log.d(LOG_TAG, "Client already started: " + uri);
            queueRunnable(new Runnable() {
                @Override
                public void run()
                {
                    fetchCompositor(uri, returnHandler);
                }
            });
        } else {
            Log.d(LOG_TAG, "Starting client: " + uri);
            Client client = Client.createForUri(this, _database, uri);
            Compositor compositor = new Compositor(this, client);
            _instanceMap.put(uri, compositor);
            returnHandler.sendMessage(Message.obtain(returnHandler,
                    COMPOSITOR_READY, compositor));
        }
    }

    public synchronized void returnCompositor(final Compositor compositor)
    {
        startService(new Intent(this, CompositorService.class));

        queueRunnable(new Runnable() {
            @Override
            public void run()
            {
                compositor.addToLooper();
            }
        });
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        super.onStartCommand(intent, flags, startId);

        _lastStartId = startId;

        return START_STICKY;
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:
