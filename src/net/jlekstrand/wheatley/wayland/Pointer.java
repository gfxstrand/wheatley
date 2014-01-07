package net.jlekstrand.wheatley.wayland;

import android.view.InputDevice;
import android.view.MotionEvent;

public class Pointer
{
    private static final String LOG_TAG = "wheatley:wayland.Pointer";

    private static final int BTN_LEFT = 0x110;
    private static final int BTN_RIGHT = 0x111;
    private static final int BTN_MIDDLE = 0x112;
    private static final int BTN_FORWARD = 0x115;
    private static final int BTN_BACK = 0x116;

    private long _nativeHandle;

    private static native long createNative(long seatHandle);
    private static native void destroyNative(long nativeHandle);

    private static native void enterOutputNative(long nativeHandle,
            long outputHandle, float x, float y);
    private static native void moveOnOutputNative(long nativeHandle, int time,
            long outputHandle, float x, float y);
    private static native void buttonNative(long nativeHandle, int time,
            int button, boolean pressed);
    private static native void axisNative(long nativeHandle, int time,
            float xval, float yval);

    private int _buttonState;

    Pointer(Seat seat)
    {
        _nativeHandle = createNative(seat.getNativeHandle());
        _buttonState = 0;
    }

    private void handleEnter(MotionEvent event, Output output)
    {
        enterOutputNative(_nativeHandle, output.getNativeHandle(),
                event.getX(), event.getY());
    }

    private void handleMove(MotionEvent event, Output output)
    {
        final int historySize = event.getHistorySize();

        for (int h = 0; h < historySize; h++) {
            moveOnOutputNative(_nativeHandle,
                    (int)event.getHistoricalEventTime(h),
                    output.getNativeHandle(),
                    event.getHistoricalX(h), event.getHistoricalY(h));
        }
        moveOnOutputNative(_nativeHandle, (int)event.getEventTime(),
                output.getNativeHandle(), event.getX(), event.getY());
    }

    private void handleButton(MotionEvent event, boolean pressed)
    {
        final int buttons;

        if (pressed)
            buttons = event.getButtonState() & (~_buttonState);
        else
            buttons = _buttonState & (~event.getButtonState());

        final int time = (int)event.getEventTime();

        if ((buttons & MotionEvent.BUTTON_PRIMARY) != 0)
            buttonNative(_nativeHandle, time, BTN_LEFT, pressed);

        if ((buttons & MotionEvent.BUTTON_SECONDARY) != 0)
            buttonNative(_nativeHandle, time, BTN_RIGHT, pressed);

        if ((buttons & MotionEvent.BUTTON_TERTIARY) != 0)
            buttonNative(_nativeHandle, time, BTN_MIDDLE, pressed);

        if ((buttons & MotionEvent.BUTTON_BACK) != 0)
            buttonNative(_nativeHandle, time, BTN_BACK, pressed);

        if ((buttons & MotionEvent.BUTTON_FORWARD) != 0)
            buttonNative(_nativeHandle, time, BTN_FORWARD, pressed);

        _buttonState = event.getButtonState();
    }

    private void handleAxis(MotionEvent event)
    {
        final int historySize = event.getHistorySize();

        for (int h = 0; h < historySize; h++)
            axisNative(_nativeHandle, (int)event.getHistoricalEventTime(h),
                    event.getHistoricalAxisValue(MotionEvent.AXIS_X, h),
                    event.getHistoricalAxisValue(MotionEvent.AXIS_Y, h));

        axisNative(_nativeHandle, (int)event.getEventTime(),
                event.getAxisValue(MotionEvent.AXIS_X),
                event.getAxisValue(MotionEvent.AXIS_Y));
    }

    private void handleLeave(MotionEvent event)
    {
        // We do nothing here.  This is because, for some insane reason,
        // Android thinks it needs to send a leave event right before a
        // button press.  This very badly messes up clients, so we just
        // ignore leave events for now.
        return;
    }

    public boolean onMotionEvent(MotionEvent event, Output output)
    {
        if ((event.getSource() & InputDevice.SOURCE_MOUSE) == 0)
            return false;

        switch (event.getActionMasked()) {
        case MotionEvent.ACTION_HOVER_ENTER:
            handleEnter(event, output);
            return true;
        case MotionEvent.ACTION_MOVE:
        case MotionEvent.ACTION_HOVER_MOVE:
            handleMove(event, output);
            return true;
        case MotionEvent.ACTION_DOWN:
            handleButton(event, true);
            return true;
        case MotionEvent.ACTION_UP:
            handleButton(event, false);
            return true;
        case MotionEvent.ACTION_SCROLL:
            handleAxis(event);
            return true;
        case MotionEvent.ACTION_HOVER_EXIT:
            handleLeave(event);
            return true;
        default:
            return false;
        }
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
