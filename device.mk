#
# Copyright 2016 The CyanogenMod Project
# Copyright 2017-2018 The LineageOS Project
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

LOCAL_PATH := device/samsung/j7eltexx

$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

# Vendor security patch level
PRODUCT_PROPERTY_OVERRIDES += \
    ro.lineage.build.vendor_security_patch=2017-12-01

# Inherit board specific products
-include $(LOCAL_PATH)/configs/product/*.mk

# Inherit from Exynos7580-common
$(call inherit-product, device/samsung/exynos7580-common/device-common.mk)

# Get non-open-source specific aspects
$(call inherit-product-if-exists, vendor/samsung/j7eltexx/j7eltexx-vendor.mk)
