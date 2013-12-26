package net.jlekstrand.wheatley;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

class ClientDatabaseHelper extends SQLiteOpenHelper
{
    private static final String LOG_TAG = "wheatley:ClientDatabaseHelper";

    public static final String DATABASE_NAME = "clients.db";
    public static final int DATABASE_VERSION = 1;

    Context _context;

    public ClientDatabaseHelper(Context context)
    {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);

        _context = context;
    }

    public void onCreate(SQLiteDatabase db)
    {
        Log.d(LOG_TAG, "Creating database...");

        Client.createDBTables(db);

        Client client = new Client(_context);
        client.setTitle("simple-shm");
        client.setCommand("/data/local/tmp/simple-shm");
        client.saveToDB(db);
    }

    public void
    onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
    {
        // Right now there is only one version
        return;
    }
}

// vim: ts=4 sw=4 sts=4 expandtab
