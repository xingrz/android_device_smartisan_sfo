/*
 * Copyright (C) 2019 The MoKee Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.mokee.settings.device;

import android.content.Context;
import android.hardware.input.InputManager;
import android.os.Handler;
import android.os.Looper;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemClock;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.ViewConfiguration;
import android.view.WindowManager;
import android.view.WindowManagerGlobal;

import com.android.internal.os.DeviceKeyHandler;

import org.mokee.internal.util.FileUtils;

public class KeyHandler implements DeviceKeyHandler {

    private static final String TAG = "KeyHandler";

    private static final int KEY_LEFT_UP = 0;
    private static final int KEY_LEFT_DOWN = 1;
    private static final int KEY_RIGHT_UP = 2;
    private static final int KEY_RIGHT_DOWN = 3;

    private final int singleTapTimeout = 150;
    private final int longPressTimeout = ViewConfiguration.getLongPressTimeout();
    private final int keyRepeatDelay = ViewConfiguration.getKeyRepeatDelay();
    private final int keyRepeatTimeout = ViewConfiguration.getKeyRepeatTimeout();

    private final KeyInfo[] keys = new KeyInfo[] {
        new KeyInfo("left_up", "gpio-keys"),
        new KeyInfo("left_down", "gpio-keys"),
        new KeyInfo("right_up", "gpio-keys"),
        new KeyInfo("right_down", "gpio-keys"),
    };

    private Context context;
    private PowerManager pm;

    private boolean ongoingPowerLongPress = false;
    private boolean pendingLeftSingleTap = false;
    private boolean pendingRightSingleTap = false;

    private int leftIndexPressed = -1;
    private int rightIndexPressed = -1;

    private Handler handler = new Handler(Looper.getMainLooper());

    private final Runnable triggerPartialScreenshot = new Runnable() {
        @Override
        public void run() {
            if (leftIndexPressed != -1 && rightIndexPressed != -1) {
                injectKey(keys[leftIndexPressed].keyCode, KeyEvent.ACTION_UP, KeyEvent.FLAG_CANCELED);
                injectKey(keys[rightIndexPressed].keyCode, KeyEvent.ACTION_UP, KeyEvent.FLAG_CANCELED);
                leftIndexPressed = -1;
                rightIndexPressed = -1;
                takeScreenshot(true);
            }
        }
    };

    public KeyHandler(Context context) {
        this.context = context;
        this.pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
    }

    public KeyEvent handleKeyEvent(KeyEvent event) {
        boolean handled = false;
        handled = handleLeftKeyEvent(event) || handled;
        handled = handleRightKeyEvent(event) || handled;
        return handled ? null : event;
    }

    private boolean handleLeftKeyEvent(KeyEvent event) {
        KeyInfo matchedKey;
        int matchedKeyIndex;

        if (keys[KEY_LEFT_UP].match(event)) {
            matchedKey = keys[KEY_LEFT_UP];
            matchedKeyIndex = KEY_LEFT_UP;
        } else if (keys[KEY_LEFT_DOWN].match(event)) {
            matchedKey = keys[KEY_LEFT_DOWN];
            matchedKeyIndex = KEY_LEFT_DOWN;
        } else {
            return false;
        }

        if (leftIndexPressed != -1 && leftIndexPressed != matchedKeyIndex) {
            injectKey(keys[leftIndexPressed].keyCode, KeyEvent.ACTION_UP, KeyEvent.FLAG_CANCELED);
            handler.removeCallbacksAndMessages("left_repeat");
            leftIndexPressed = -1;
        }

        pendingRightSingleTap = false;
        handler.removeCallbacksAndMessages("right_tap");
        handler.removeCallbacksAndMessages("right_repeat");

        switch (event.getAction()) {
            case KeyEvent.ACTION_DOWN:
                if (rightIndexPressed != -1) {
                    handler.postDelayed(triggerPartialScreenshot, "partial_screenshot", longPressTimeout);
                } else {
                    pendingLeftSingleTap = true;
                    handler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            pendingLeftSingleTap = false;
                            injectKey(matchedKey.keyCode, KeyEvent.ACTION_DOWN, 0);
                        }
                    }, "left_tap", singleTapTimeout);
                    scheduleKeyRepeat(matchedKey.keyCode, "left_repeat");
                }
                if (pm.isInteractive()) {
                    leftIndexPressed = matchedKeyIndex;
                }
                break;
            case KeyEvent.ACTION_UP:
                if (rightIndexPressed != -1) {
                    takeScreenshot(false);
                    rightIndexPressed = -1;
                } else {
                    if (pendingLeftSingleTap) {
                        pendingLeftSingleTap = false;
                        handler.removeCallbacksAndMessages("left_tap");
                        injectKey(matchedKey.keyCode, KeyEvent.ACTION_DOWN, 0);
                    }
                    injectKey(matchedKey.keyCode, KeyEvent.ACTION_UP, 0);
                }
                handler.removeCallbacksAndMessages("left_repeat");
                handler.removeCallbacksAndMessages("partial_screenshot");
                leftIndexPressed = -1;
                break;
        }

        return true;
    }

    private boolean handleRightKeyEvent(KeyEvent event) {
        KeyInfo matchedKey;
        int matchedKeyIndex;

        if (keys[KEY_RIGHT_UP].match(event)) {
            matchedKey = keys[KEY_RIGHT_UP];
            matchedKeyIndex = KEY_RIGHT_UP;
        } else if (keys[KEY_RIGHT_DOWN].match(event)) {
            matchedKey = keys[KEY_RIGHT_DOWN];
            matchedKeyIndex = KEY_RIGHT_DOWN;
        } else {
            return false;
        }

        if (rightIndexPressed != -1 && rightIndexPressed != matchedKeyIndex) {
            injectKey(keys[rightIndexPressed].keyCode, KeyEvent.ACTION_UP, KeyEvent.FLAG_CANCELED);
            handler.removeCallbacksAndMessages("right_repeat");
            rightIndexPressed = -1;
        }

        pendingLeftSingleTap = false;
        handler.removeCallbacksAndMessages("left_tap");
        handler.removeCallbacksAndMessages("left_repeat");

        switch (event.getAction()) {
            case KeyEvent.ACTION_DOWN:
                if (leftIndexPressed != -1) {
                    handler.postDelayed(triggerPartialScreenshot, "partial_screenshot", longPressTimeout);
                } else {
                    pendingRightSingleTap = true;
                    handler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            pendingRightSingleTap = false;
                            injectKey(matchedKey.keyCode, KeyEvent.ACTION_DOWN, 0);
                        }
                    }, "right_tap", singleTapTimeout);
                    scheduleKeyRepeat(matchedKey.keyCode, "right_repeat");
                }
                if (pm.isInteractive()) {
                    rightIndexPressed = matchedKeyIndex;
                }
                break;
            case KeyEvent.ACTION_UP:
                if (leftIndexPressed != -1) {
                    takeScreenshot(false);
                    leftIndexPressed = -1;
                } else {
                    if (pendingRightSingleTap) {
                        pendingRightSingleTap = false;
                        handler.removeCallbacksAndMessages("right_tap");
                        injectKey(matchedKey.keyCode, KeyEvent.ACTION_DOWN, 0);
                    }
                    injectKey(matchedKey.keyCode, KeyEvent.ACTION_UP, 0);
                }
                handler.removeCallbacksAndMessages("right_repeat");
                handler.removeCallbacksAndMessages("partial_screenshot");
                rightIndexPressed = -1;
                break;
        }

        return true;
    }

    private String getDeviceName(KeyEvent event) {
        final int deviceId = event.getDeviceId();
        final InputDevice device = InputDevice.getDevice(deviceId);
        return device == null ? null : device.getName();
    }

    private void scheduleKeyRepeat(int code, String token) {
        final Runnable repeatRunnable = new Runnable() {
            @Override
            public void run() {
                injectKey(code);
                handler.postDelayed(this, token, keyRepeatDelay);
            }
        };

        final Runnable timeoutRunnable = new Runnable() {
            @Override
            public void run() {
                injectKey(code, KeyEvent.ACTION_UP, 0);
                handler.postDelayed(repeatRunnable, token, keyRepeatDelay);
            }
        };

        handler.postDelayed(timeoutRunnable, token, keyRepeatTimeout);
    }

    private void injectKey(int code) {
        injectKey(code, KeyEvent.ACTION_DOWN, 0);
        injectKey(code, KeyEvent.ACTION_UP, 0);
    }

    private void injectKey(int code, int action, int flags) {
        final long now = SystemClock.uptimeMillis();
        InputManager.getInstance().injectInputEvent(new KeyEvent(
                        now, now, action, code, 0, 0,
                        KeyCharacterMap.VIRTUAL_KEYBOARD,
                        0, flags,
                        InputDevice.SOURCE_KEYBOARD),
                InputManager.INJECT_INPUT_EVENT_MODE_ASYNC);
    }

    private void takeScreenshot(final boolean partial) {
        final int type = partial
                ? WindowManager.TAKE_SCREENSHOT_SELECTED_REGION
                : WindowManager.TAKE_SCREENSHOT_FULLSCREEN;

        try {
            WindowManagerGlobal.getWindowManagerService().mokeeTakeScreenshot(type);
        } catch (RemoteException e) {
            Log.e(TAG, "Error while trying to takeScreenshot.", e);
        }
    }

    private class KeyInfo {

        final String file;
        final String deviceName;
        final int scanCode;
        int deviceId;
        int keyCode;

        KeyInfo(String file, String deviceName) {
            int scanCode;
            this.file = "/proc/keypad/" + file;
            this.deviceName = deviceName;
            try {
                scanCode = Integer.parseInt(FileUtils.readOneLine(this.file));
            } catch (NumberFormatException ignored) {
                scanCode = 0;
            }
            this.scanCode = scanCode;
        }

        boolean match(KeyEvent event) {
            if (deviceId == 0) {
                final String deviceName = getDeviceName(event);
                if (this.deviceName.equals(deviceName)) {
                    deviceId = event.getDeviceId();
                } else {
                    return false;
                }
            } else {
                if (deviceId != event.getDeviceId()) {
                    return false;
                }
            }

            if (event.getScanCode() == scanCode) {
                keyCode = event.getKeyCode();
            } else {
                return false;
            }

            return true;
        }

    }

}
