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

#####################
# ADB Security
#####################

#ifeq ($(TARGET_BUILD_VARIANT),eng)

PRODUCT_PROPERTY_OVERRIDES += \
       ro.secure=0 \
       ro.adb.secure=0 \
       ro.hardware=universal7580 \
       persist.service.adb.enable=1 \
       persist.service.debuggable=1 \
       persist.sys.usb.config=adb \
       ro.securestorage.support=false

# endif
