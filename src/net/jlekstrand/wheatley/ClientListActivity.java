package net.jlekstrand.wheatley;

import android.app.ActionBar;
import android.app.ListActivity;
import android.app.LoaderManager;
import android.content.AsyncTaskLoader;
import android.content.Context;
import android.content.Loader;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.LayoutInflater;
import android.widget.CursorAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class ClientListActivity extends ListActivity
        implements LoaderManager.LoaderCallbacks<Cursor>
{
    private static final String LOG_TAG = "wheatley:ClientListActivity";

    private class ClientCursorAdapter extends CursorAdapter
    {
        public ClientCursorAdapter(Context context, Cursor cursor)
        {
            super(context, cursor);
        }

        public void bindView(View view, Context context, Cursor cursor)
        {
            Client client = new Client(context, cursor);

            ((TextView)view.findViewById(R.id.title)).setText(
                    client.getTitle());
            ((TextView)view.findViewById(R.id.command)).setText(
                    client.getCommand());

            ImageView iconView = (ImageView)view.findViewById(R.id.icon);

            Bitmap icon = client.getIcon();
            if (icon != null)
                iconView.setImageBitmap(icon);
            else
                iconView.setImageResource(R.drawable.ic_launcher);
        }

        public View newView(Context context, Cursor cursor, ViewGroup parent)
        {
            LayoutInflater inflater = (LayoutInflater)context.getSystemService(
                    Context.LAYOUT_INFLATER_SERVICE);
            View view = inflater.inflate(R.layout.client_list_item, parent, false);
            bindView(view, context, cursor);
            return view;
        }
    }

    private ClientCursorAdapter _adapter;

    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.d(LOG_TAG, "onCreate");

        ClientDatabaseHelper helper =
                new ClientDatabaseHelper(ClientListActivity.this);
        SQLiteDatabase db = helper.getReadableDatabase();

        Cursor cursor = db.query(Client.DB.DATABASE_TABLE,
                Client.DB.DATABASE_PROJECTION,
                null, null, null, null, null, null);

        _adapter = new ClientCursorAdapter(this, cursor);
        setListAdapter(_adapter);

        // getLoaderManager().initLoader(0, null, this);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        getMenuInflater().inflate(R.menu.client_list_activity_actions, menu);

        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        switch (item.getItemId()) {
        case R.id.action_add_client:
            onActionAddClient();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    private void onActionAddClient()
    {
        Log.d(LOG_TAG, "onActionAddClient()");
    }

    public Loader<Cursor> onCreateLoader(int id, Bundle args)
    {
        Log.d(LOG_TAG, "onCreateLoader");
        return new AsyncTaskLoader<Cursor>(this) {
            public Cursor loadInBackground()
            {
                Log.d(LOG_TAG, "Loading configured clients.");

                ClientDatabaseHelper helper =
                        new ClientDatabaseHelper(ClientListActivity.this);
                SQLiteDatabase db = helper.getReadableDatabase();

                return db.query(Client.DB.DATABASE_TABLE,
                        Client.DB.DATABASE_PROJECTION,
                        null, null, null, null, null, null);
            }
        };
    }

    public void onLoadFinished(Loader<Cursor> loader, Cursor data)
    {
        _adapter.swapCursor(data);
    }

    public void onLoaderReset(Loader<Cursor> loader)
    {
        _adapter.swapCursor(null);
    }
}

// vim: ts=4 sw=4 sts=4 expandtab
