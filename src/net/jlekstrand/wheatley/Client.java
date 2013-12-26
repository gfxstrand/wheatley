package net.jlekstrand.wheatley;

import java.io.FileNotFoundException;

import android.content.Context;
import android.content.ContentValues;
import android.content.SharedPreferences;
import android.database.sqlite.SQLiteDatabase;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.widget.Toast;

class Client
{
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

    public Client(Context context, Cursor cursor)
    {
        _context = context;
        _id = cursor.getLong(cursor.getColumnIndex(DB._ID));
        _title = cursor.getString(cursor.getColumnIndex(DB.TITLE));
        _iconFilename = cursor.getString(cursor.getColumnIndex(DB.ICON));
        _command = cursor.getString(cursor.getColumnIndex(DB.COMMAND));
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

// vim: ts=4 sw=4 sts=4 expandtab
