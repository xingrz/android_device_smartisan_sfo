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

#include <linux/input.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>

#include "KeyDisabler.h"

namespace vendor {
namespace mokee {
namespace touch {
namespace V1_0 {
namespace implementation {

constexpr const char kNavLeftControlPath[] = "/proc/keypad/nav_left";
constexpr const char kNavMiddleControlPath[] = "/proc/keypad/nav_middle";
constexpr const char kNavRightControlPath[] = "/proc/keypad/nav_right";

KeyDisabler::KeyDisabler() {
    mHasKeyDisabler = !access(kNavLeftControlPath, F_OK) &&
                      !access(kNavMiddleControlPath, F_OK) &&
                      !access(kNavRightControlPath, F_OK);
}

// Methods from ::vendor::mokee::touch::V1_0::IKeyDisabler follow.
Return<bool> KeyDisabler::isEnabled() {
    std::string buf;

    if (!mHasKeyDisabler) return false;

    if (!android::base::ReadFileToString(kNavMiddleControlPath, &buf, true)) {
        LOG(ERROR) << "Failed to read " << kNavMiddleControlPath;
        return false;
    }

    return std::stoi(android::base::Trim(buf)) == KEY_HOMEPAGE;
}

Return<bool> KeyDisabler::setEnabled(bool enabled) {
    std::string leftValue = std::to_string(enabled ? 0 : KEY_BACK);
    std::string middleValue = std::to_string(enabled ? 0 : KEY_HOMEPAGE);
    std::string rightValue = std::to_string(enabled ? 0 : KEY_APPSELECT);

    if (!mHasKeyDisabler) return false;

    if (!android::base::WriteStringToFile(leftValue, kNavLeftControlPath, true)) {
        LOG(ERROR) << "Failed to write " << kNavLeftControlPath;
        return false;
    }

    if (!android::base::WriteStringToFile(middleValue, kNavMiddleControlPath, true)) {
        LOG(ERROR) << "Failed to write " << kNavMiddleControlPath;
        return false;
    }

    if (!android::base::WriteStringToFile(rightValue, kNavRightControlPath, true)) {
        LOG(ERROR) << "Failed to write " << kNavRightControlPath;
        return false;
    }

    return true;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace mokee
}  // namespace vendor
