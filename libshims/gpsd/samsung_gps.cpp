/*
 * Copyright (C) 2016 The CyanogenMod Project
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

#define LOG_TAG "libshim_gpsd"
#define LOG_NDEBUG 0

#include <stdlib.h>
#include <stdint.h>

#include <sensor/SensorManager.h>
#include <utils/Log.h>

namespace android {

extern "C" {
    SensorManager *_ZN7android9SingletonINS_13SensorManagerEE9sInstanceE = NULL;
    Mutex _ZN7android9SingletonINS_13SensorManagerEE5sLockE(Mutex::PRIVATE);


    void *CRYPTO_malloc(uint32_t size, const char *file __unused, uint32_t line __unused)
    {
        return calloc(size, 1);
    }

    long SSL_CTX_ctrl(void *ctx __unused, int cmd, long larg __unused, void *parg __unused)
    {
        ALOGE("SSL_CTX_ctrl cmd: %d", cmd);

        return 0;
    }

    void _ZN7android13SensorManagerC1ERKNS_8String16E(void* obj, const String16& opPackageName);
    void _ZN7android13SensorManagerC1Ev(void* obj)
    {
        _ZN7android13SensorManagerC1ERKNS_8String16E(obj, String16());
    }

    void *_ZN7android13SensorManager16createEventQueueENS_7String8Ei(void* obj, String8 packageName, int mode);
    void *_ZN7android13SensorManager16createEventQueueEv(void* obj)
    {
        return _ZN7android13SensorManager16createEventQueueENS_7String8Ei(obj, String8(""), 0);
    }

    ssize_t _ZN7android13SensorManager13getSensorListEPPKPKNS_6SensorE(void *obj, Sensor const* const** list);
    ssize_t _ZNK7android13SensorManager13getSensorListEPPKPKNS_6SensorE(void *obj, Sensor const* const** list)
    {
        return _ZN7android13SensorManager13getSensorListEPPKPKNS_6SensorE(obj, list);
    }
} /* extern */

} /* namespace */
