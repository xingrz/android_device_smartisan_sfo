/*
Copyright (c) 2013, The Linux Foundation. All rights reserved.
Copyright (c) 2019, The MoKee Open Source Project

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android-base/file.h>
#include <android-base/strings.h>
#include <android-base/logging.h>
#include <android-base/properties.h>

#include "property_service.h"
#include "log.h"

using android::base::ReadFileToString;

namespace android {
namespace init {

const std::string cpu_id_file = "/sys/pversion_info/cpu_id";

void load_device(const char*name, const char *model, const char *description, const char *fingerprint)
{
    property_set("ro.bootimage.build.fingerprint", fingerprint);
    property_set("ro.build.product", name);
    property_set("ro.build.description", description);
    property_set("ro.build.fingerprint", fingerprint);
    property_set("ro.product.name", name);
    property_set("ro.product.model", model);
    property_set("ro.product.device", name);
    property_set("ro.vendor.product.model", model);
    property_set("ro.vendor.build.fingerprint", fingerprint);
}

void vendor_load_properties()
{
    std::string cpu_id;

    if (ReadFileToString(cpu_id_file, &cpu_id)) {
        if (cpu_id.find("cpu_id=213") != -1) {
            load_device(
                "msm8974sfo",
                "SM701",
                "msm8974sfo-user 4.4.2 SANFRANCISCO dev-keys",
                "smartisan/msm8974sfo/msm8974sfo:4.4.2/SANFRANCISCO:user/dev-keys"
            );
        }
        else if (cpu_id.find("cpu_id=194") != -1) {
            load_device(
                "msm8974sfo_lte",
                "SM705",
                "msm8974sfo_lte-user 4.4.2 SANFRANCISCO dev-keys",
                "smartisan/msm8974sfo_lte/msm8974sfo_lte:4.4.2/SANFRANCISCO:user/dev-keys"
            );
        }
        else {
            LOG(ERROR) << "Unknown cpu_id: " << cpu_id;
        }
    }
    else {
        LOG(ERROR) << "Unable to read cpu_id from " << cpu_id_file;
    }
}

}  // namespace init
}  // namespace android
