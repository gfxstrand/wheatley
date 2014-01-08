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
package net.jlekstrand.wheatley.config;

import java.io.FileNotFoundException;
import java.util.List;

import android.content.Context;
import android.content.ContentValues;
import android.content.SharedPreferences;
import android.database.sqlite.SQLiteDatabase;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.util.Log;
import android.widget.Toast;

public class Client
{
    private static final String LOG_TAG = "wheatley:Client";

    public static final Uri BASE_CONTENT_URI =
            Uri.parse("content://net.jlekstrand.wheatley/clients/");

    public static class DB
    {
        public static final String DATABASE_TABLE = "clients";

        public static final String _ID = "_id";
        public static final String TITLE = "title";
        public static final String ICON = "icon";
        public static final String COMMAND = "command";

        public static final String[] DATABASE_PROJECTION = {
            _ID,
            TITLE,
            ICON,
            COMMAND,
        };
    };

    private Context _context;

    private long _id;
    private String _title;
    private Bitmap _iconBitmap;
    private boolean _iconBitmapDirty;
    private String _iconFilename;
    private String _command;

    public Client(Context context)
    {
        _context = context;
        _id = -1;
        _iconBitmapDirty = false;
    }

    private Client(Context context, Cursor cursor)
    {
        _context = context;
        _id = cursor.getLong(cursor.getColumnIndex(DB._ID));
        _title = cursor.getString(cursor.getColumnIndex(DB.TITLE));
        _iconFilename = cursor.getString(cursor.getColumnIndex(DB.ICON));
        _command = cursor.getString(cursor.getColumnIndex(DB.COMMAND));
    }

    public static Client createForCursor(Context context, Cursor cursor)
    {
        return new Client(context, cursor);
    }

    public static Client createForId(Context context, SQLiteDatabase db,
            long clientId)
    {
        if (db == null) {
            DatabaseHelper helper = new DatabaseHelper(context);
            db = helper.getReadableDatabase();
        }

        Cursor cursor = db.query(Client.DB.DATABASE_TABLE,
                Client.DB.DATABASE_PROJECTION,
                Client.DB._ID + "=?", new String[] { String.valueOf(clientId) },
                null, null, null, null);

        if (cursor.getCount() > 0) {
            cursor.moveToFirst();
            return createForCursor(context, cursor);
        }

        return null;
    }

    public static Client createForUri(Context context, SQLiteDatabase db,
            Uri uri)
    {
        if (! "content".equals(uri.getScheme())) {
            Log.d(LOG_TAG, "Not a content URI: " + uri);
            return null;
        }

        if (! "net.jlekstrand.wheatley".equals(uri.getAuthority())) {
            Log.d(LOG_TAG, "Wrong URI authority: " + uri);
            return null;
        }

        List<String> path = uri.getPathSegments();
        if (path.size() != 2 || ! "clients".equals(path.get(0))) {
            Log.e(LOG_TAG, "Invalid URI: " + uri);
            return null;
        }

        long id = -1;
        try {
            id = Long.parseLong(path.get(1));
        } catch (NumberFormatException e) {
            Log.e(LOG_TAG, "Invalid URI: " + uri);
            return null;
        }

        return createForId(context, db, id);
    }

    public void
    saveToDB(SQLiteDatabase db)
    {
        ContentValues cv = new ContentValues();

        String oldFilename = null;

        saveIconBitmap();

        cv.put(DB.TITLE, _title);
        cv.put(DB.ICON, _iconFilename);
        cv.put(DB.COMMAND, _command);

        if (_id == -1) {
            _id = db.insert(DB.DATABASE_TABLE, null, cv);
        } else {
            db.update(DB.DATABASE_TABLE, cv, DB._ID + " = " + _id, null);
        }
    }

    private void saveIconBitmap()
    {
        if (!_iconBitmapDirty)
            return;

        // This should be unique
        String name = DB.DATABASE_TABLE + "." + DB.ICON + "." +
                System.currentTimeMillis() + ".png";

        try {
            boolean saved = _iconBitmap.compress(Bitmap.CompressFormat.PNG, 50,
                    _context.openFileOutput(name, Context.MODE_PRIVATE));

            if (saved) {
                _iconFilename = name;
                _iconBitmapDirty = false;
            }
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        }
    }

    public String getTitle()
    {
        return _title;
    }

    public void setTitle(String title)
    {
        _title = title;
    }

    public Bitmap getIcon()
    {
        if (_iconBitmap != null)
            return _iconBitmap;

        if (_iconFilename == null)
            return null;

        try {
            _iconBitmap = BitmapFactory.decodeStream(
                    _context.openFileInput(_iconFilename));
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
            // return null;
        }

        return _iconBitmap;
    }

    public void setIcon(Bitmap iconBitmap)
    {
        _iconBitmap = iconBitmap;
        _iconBitmapDirty = true;
    }

    public String getCommand()
    {
        return _command;
    }

    public void setCommand(String command)
    {
        _command = command;
    }

    static void createDBTables(SQLiteDatabase db)
    {
        db.execSQL("CREATE TABLE " + DB.DATABASE_TABLE + "("
                + DB._ID + " INTEGER PRIMARY KEY AUTOINCREMENT, "
                + DB.TITLE + " TEXT, "
                + DB.ICON + " TEXT, "
                + DB.COMMAND + " TEXT"
                + ");");
    }
}

// vim: set ts=4 sw=4 sts=4 expandtab:
