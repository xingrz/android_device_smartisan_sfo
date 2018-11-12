/*
 * Copyright 2016 The Android Open Source Project
 * Copyright 2018 The MoKee Open Source Project
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

#define LOG_TAG "android.hardware.light@2.0-service.sfo"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>

#include "Light.h"

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::light::V2_0::ILight;
using android::hardware::light::V2_0::implementation::Light;

const static std::string kLcdBacklightPath = "/sys/class/leds/lcd-backlight/brightness";
const static std::string kLcdMaxBacklightPath = "/sys/class/leds/lcd-backlight/max_brightness";
const static std::string kRedLedPath = "/sys/class/leds/red/brightness";
const static std::string kRedDutyPctsPath = "/sys/class/leds/red/duty_pcts";
const static std::string kRedStartIdxPath = "/sys/class/leds/red/start_idx";
const static std::string kRedPauseLoPath = "/sys/class/leds/red/pause_lo";
const static std::string kRedPauseHiPath = "/sys/class/leds/red/pause_hi";
const static std::string kRedRampStepMsPath = "/sys/class/leds/red/ramp_step_ms";
const static std::string kRedBlinkPath = "/sys/class/leds/red/blink";

int main() {
    uint32_t lcdMaxBrightness = 255;

    std::ofstream lcdBacklight(kLcdBacklightPath);
    if (!lcdBacklight) {
        LOG(ERROR) << "Failed to open " << kLcdBacklightPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    std::ifstream lcdMaxBacklight(kLcdMaxBacklightPath);
    if (!lcdMaxBacklight) {
        LOG(ERROR) << "Failed to open " << kLcdMaxBacklightPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    } else {
        lcdMaxBacklight >> lcdMaxBrightness;
    }

    std::ofstream redLed(kRedLedPath);
    if (!redLed) {
        LOG(ERROR) << "Failed to open " << kRedLedPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream redDutyPcts(kRedDutyPctsPath);
    if (!redDutyPcts) {
        LOG(ERROR) << "Failed to open " << kRedDutyPctsPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream redStartIdx(kRedStartIdxPath);
    if (!redStartIdx) {
        LOG(ERROR) << "Failed to open " << kRedStartIdxPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream redPauseLo(kRedPauseLoPath);
    if (!redPauseLo) {
        LOG(ERROR) << "Failed to open " << kRedPauseLoPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream redPauseHi(kRedPauseHiPath);
    if (!redPauseHi) {
        LOG(ERROR) << "Failed to open " << kRedPauseHiPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream redRampStepMs(kRedRampStepMsPath);
    if (!redRampStepMs) {
        LOG(ERROR) << "Failed to open " << kRedRampStepMsPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    std::ofstream redBlink(kRedBlinkPath);
    if (!redBlink) {
        LOG(ERROR) << "Failed to open " << kRedBlinkPath << ", error=" << errno
                   << " (" << strerror(errno) << ")";
        return -errno;
    }

    android::sp<ILight> service = new Light(
            {std::move(lcdBacklight), lcdMaxBrightness},
            std::move(redLed),
            std::move(redDutyPcts),
            std::move(redStartIdx),
            std::move(redPauseLo),
            std::move(redPauseHi),
            std::move(redRampStepMs),
            std::move(redBlink));

    configureRpcThreadpool(1, true);

    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        LOG(ERROR) << "Cannot register Light HAL service";
        return 1;
    }

    LOG(INFO) << "Light HAL Ready.";
    joinRpcThreadpool();
    // Under normal cases, execution will not reach this line.
    LOG(ERROR) << "Light HAL failed to join thread pool.";
    return 1;
}
