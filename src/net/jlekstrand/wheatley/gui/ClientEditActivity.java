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
package net.jlekstrand.wheatley.gui;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;

import net.jlekstrand.wheatley.R;
import net.jlekstrand.wheatley.config.Client;
import net.jlekstrand.wheatley.config.DatabaseHelper;

public class ClientEditActivity extends Activity
{
    private static final String LOG_TAG = "wheatley:ClientEditActivity";

    public static final String EXTRA_CLIENT_ID = "ClientEditActivity.clientId";

    private static final int REQUEST_CHOOSE_IMAGE = 1;

    Client _client;
    SQLiteDatabase _database;

    EditText _titleEdit;
    ImageView _iconView;
    EditText _commandEdit;

    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Default is to discard changes
        setResult(RESULT_CANCELED);

        Intent intent = getIntent();

        setContentView(R.layout.client_edit_activity);

        _titleEdit = (EditText)findViewById(R.id.title);
        _iconView = (ImageView)findViewById(R.id.icon);
        _commandEdit = (EditText)findViewById(R.id.command);

        DatabaseHelper helper = new DatabaseHelper(this);
        _database = helper.getReadableDatabase();

        Uri clientUri = intent.getData();
        if (clientUri != null)
            _client = Client.createForUri(this, _database, clientUri);

        if (_client == null) {
            _client = new Client(this);
        } else {
            _titleEdit.setText(_client.getTitle());
            _commandEdit.setText(_client.getCommand());

            Bitmap icon = _client.getIcon();
            if (icon == null) {
                _iconView.setImageResource(R.drawable.ic_launcher);
            } else {
                _iconView.setImageBitmap(icon);
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        if (resultCode == Activity.RESULT_CANCELED) {
            Log.d(LOG_TAG, "Action canceled: requestCode=" + requestCode);
            return;
        }

        switch (requestCode) {
        case REQUEST_CHOOSE_IMAGE:
            onIconImageSelected(data);
            break;
        default:
        }
    }

    public void onIconClicked(View view)
    {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT, null);

        intent.setType("image/*");
        intent.putExtra("crop", "true");
        intent.putExtra("outputX", 128);
        intent.putExtra("outputY", 128);
        intent.putExtra("aspectX", 1);
        intent.putExtra("aspectY", 1);
        intent.putExtra("scale", true);
        intent.putExtra("return-data", true);
        intent.putExtra("noFaceDetection", true);

        startActivityForResult(Intent.createChooser(intent,
                "Select an image using"), REQUEST_CHOOSE_IMAGE);
    }

    private void onIconImageSelected(Intent data)
    {
        Log.d(LOG_TAG, "Image selected");

        Bundle extras = data.getExtras();
        Bitmap icon = (Bitmap)extras.getParcelable("data");
        _client.setIcon(icon);
        _iconView.setImageBitmap(icon);
    }

    public void onCancelClicked(View view)
    {
        Log.d(LOG_TAG, "onCancelClicked()");
        finish();
    }

    public void onSaveClicked(View view)
    {
        Log.d(LOG_TAG, "onSaveClicked()");

        _client.setTitle(_titleEdit.getText().toString());
        _client.setCommand(_commandEdit.getText().toString());
        _client.saveToDB(_database);

        setResult(RESULT_OK);

        finish();
    }
}
