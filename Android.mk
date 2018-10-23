#
# Copyright (C) 2018 The MoKee Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This contains the module build definitions for the hardware-specific
# components for this device.
#
# As much as possible, those components should be built unconditionally,
# with device-specific names to avoid collisions, to avoid device-specific
# bitrot and build breakages. Building a component unconditionally does
# *not* include it on all devices, so it is safe even with hardware-specific
# components.

LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),sfo)
include $(call all-makefiles-under,$(LOCAL_PATH))

include $(CLEAR_VARS)

AUDIO_BINS := mbhc.bin wcd9320_anc.bin wcd9320_mad_audio.bin
AUDIO_SYMLINKS := $(addprefix $(TARGET_OUT_ETC)/firmware/wcd9320/,$(notdir $(AUDIO_BINS)))
$(AUDIO_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Audio bin link: $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	$(hide) ln -sf /data/misc/audio/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(AUDIO_SYMLINKS)

WIFI_CFG_SYMLINK := $(TARGET_OUT_ETC)/firmware/wlan/prima/WCNSS_qcom_cfg.ini
$(WIFI_CFG_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@echo "Wi-Fi config ini link: $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	$(hide) ln -sf /data/misc/wifi/$(notdir $@) $@

WIFI_MAC_BINS := wifi_mac_nv.bin wifi_random_mac.bin
WIFI_MAC_SYMLINK := $(addprefix $(TARGET_OUT_ETC)/firmware/wlan/prima/,$(notdir $(WIFI_MAC_BINS)))
$(WIFI_MAC_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@echo "Wi-Fi MAC bin link: $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	$(hide) ln -sf /persist/.$(notdir $@) $@

WIFI_NV_SYMLINK := $(TARGET_OUT_ETC)/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin
$(WIFI_NV_SYMLINK): $(LOCAL_INSTALLED_MODULE)
	@echo "Wi-Fi NV bin link: $@"
	@mkdir -p $(dir $@)
	@rm -f $@
	$(hide) ln -sf /persist/$(notdir $@) $@

ALL_DEFAULT_INSTALLED_MODULES += $(WIFI_CFG_SYMLINK) $(WIFI_MAC_SYMLINK) $(WIFI_NV_SYMLINK)

endif
