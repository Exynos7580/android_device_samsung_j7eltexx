/*
 * Copyright (C) 2012-2016, The CyanogenMod Project
 * Copyright (C) 2018, Danny Wood
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

/**
* @file CameraWrapper.cpp
*
* This file wraps a vendor camera module.
*/

#define LOG_TAG "CameraWrapper"
#define LOG_NDEBUG 0
//#define LOG_PARAMETERS

#include <camera/CameraParameters.h>
#include <camera/Camera.h>
#include <cutils/log.h>
#include <hardware/camera.h>
#include <hardware/hardware.h>
#include <utils/String8.h>
#include <utils/threads.h>

static android::Mutex gCameraWrapperLock;
static camera_module_t *gVendorModule = 0;
static preview_stream_ops *gPreviewWindow = 0;
static bool gPreviewStartDeferred = false;

static int camera_device_open(const hw_module_t *module, const char *name,
        hw_device_t **device);

static int camera_mod_get_number_of_cameras(void);
static int camera_mod_get_camera_info(int camera_id, struct camera_info *info);
static int camera_mod_set_callbacks(const camera_module_callbacks_t *callbacks);
static void camera_mod_get_vendor_tag_ops(vendor_tag_ops_t* ops);
static int camera_mod_set_torch_mode(const char* camera_id, bool enabled);
static int camera_mod_open_legacy(const struct hw_module_t* module, const char* id, uint32_t halVersion, struct hw_device_t** device);

static int check_vendor_module();
static const char * msg2chr(int32_t msg_type);

typedef struct wrapper_camera_device {
    camera_device_t base;
    int id;
    camera_device_t *vendor;
} wrapper_camera_device_t;

#define VENDOR_CALL(device, func, ...) ({ \
    wrapper_camera_device_t *__wrapper_dev = (wrapper_camera_device_t*) device; \
    __wrapper_dev->vendor->ops->func(__wrapper_dev->vendor, ##__VA_ARGS__); \
})

#define CAMERA_ID(device) (((wrapper_camera_device_t *)device)->id)

static struct hw_module_methods_t camera_module_methods = {
    .open = camera_device_open,
};

camera_module_t HAL_MODULE_INFO_SYM = {
    .common = {
         .tag = HARDWARE_MODULE_TAG,
         .module_api_version = CAMERA_MODULE_API_VERSION_1_0,
         .hal_api_version = HARDWARE_HAL_API_VERSION,
         .id = CAMERA_HARDWARE_MODULE_ID,
         .name = "Exynos Camera Wrapper",
         .author = "The CyanogenMod Project",
         .methods = &camera_module_methods,
         .dso = NULL,
         .reserved = {0},
    },
    .get_number_of_cameras = camera_mod_get_number_of_cameras,
    .get_camera_info = camera_mod_get_camera_info,
    .set_callbacks = camera_mod_set_callbacks,
    .get_vendor_tag_ops = camera_mod_get_vendor_tag_ops,
    .open_legacy = NULL,
    .set_torch_mode = camera_mod_set_torch_mode,
    .init = NULL,
    .reserved = {0},
};

static const char * msg2chr(int32_t msg_type)
{
    switch (msg_type) {
    case CAMERA_MSG_ERROR:
        return "CAMERA_MSG_ERROR";
    case CAMERA_MSG_SHUTTER:
        return "CAMERA_MSG_SHUTTER";
    case CAMERA_MSG_FOCUS:
        return "CAMERA_MSG_FOCUS";
    case CAMERA_MSG_ZOOM:
        return "CAMERA_MSG_ZOOM";
    case CAMERA_MSG_PREVIEW_FRAME:
        return "CAMERA_MSG_PREVIEW_FRAME";
    case CAMERA_MSG_VIDEO_FRAME:
        return "CAMERA_MSG_VIDEO_FRAME";
    case CAMERA_MSG_POSTVIEW_FRAME:
        return "CAMERA_MSG_POSTVIEW_FRAME";
    case CAMERA_MSG_RAW_IMAGE:
        return "CAMERA_MSG_RAW_IMAGE";
    case CAMERA_MSG_COMPRESSED_IMAGE:
        return "CAMERA_MSG_COMPRESSED_IMAGE";
    case CAMERA_MSG_RAW_IMAGE_NOTIFY:
        return "CAMERA_MSG_RAW_IMAGE_NOTIFY";
    case CAMERA_MSG_PREVIEW_METADATA:
        return "CAMERA_MSG_PREVIEW_METADATA";
    case CAMERA_MSG_FOCUS_MOVE:
        return "CAMERA_MSG_FOCUS_MOVE";
    case CAMERA_MSG_VENDOR_START:
        return "CAMERA_MSG_VENDOR_START|CAMERA_MSG_STATS_DATA";
    case CAMERA_MSG_META_DATA:
        return "CAMERA_MSG_META_DATA";
    case CAMERA_MSG_VENDOR_END:
        return "CAMERA_MSG_VENDOR_END";
    case CAMERA_MSG_ALL_MSGS:
        return "CAMERA_MSG_ALL_MSGS";
    default:
        return "UNKNOWN_MSG";
    }
}

static int camera_mod_set_torch_mode(const char* camera_id, bool enabled)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->set_torch_mode(camera_id, enabled);
}

static int camera_mod_set_callbacks(const camera_module_callbacks_t *callbacks)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->set_callbacks(callbacks);
}

static void camera_mod_get_vendor_tag_ops(vendor_tag_ops_t* ops)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return;
    return gVendorModule->get_vendor_tag_ops(ops);
}

static int camera_mod_open_legacy(const struct hw_module_t* module, const char* id, uint32_t halVersion __unused, struct hw_device_t** device)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return 0;
    return camera_device_open(module, id, device);
}

static int check_vendor_module()
{
    int rv;
    ALOGV("%s", __FUNCTION__);

    if (gVendorModule)
        return 0;

    rv = hw_get_module_by_class("camera", "vendor", (const hw_module_t**)&gVendorModule);
    if (rv)
        ALOGE("Failed to open vendor camera module %d", rv);

    return rv;
}

static char *camera_fixup_getparams(int id __unused,
    const char *settings)
{
    android::CameraParameters params;
    params.unflatten(android::String8(settings));

#ifdef LOG_PARAMETERS
    ALOGV("%s: Original parameters:", __FUNCTION__);
    params.dump();
#endif

    /* Enforce scene-mode-values to auto */
    params.set(android::CameraParameters::KEY_SUPPORTED_SCENE_MODES, "auto");
    ALOGV("%s: Fixed: scene-mode-values: auto", __FUNCTION__);

    /* If the vendor has HFR values but doesn't also expose that
     * this can be turned off, fixup the params to tell the Camera
     * that it really is okay to turn it off.
     */
    const char *hfrModeValues = params.get("video-hfr-values");
    if (hfrModeValues && !strstr(hfrModeValues, "off")) {
        char hfrModes[strlen(hfrModeValues) + 4 + 1];
        sprintf(hfrModes, "%s,off", hfrModeValues);
        params.set("video-hfr-values", hfrModes);

        ALOGV("%s: Fixed: video-hfr-values: %s", __FUNCTION__, hfrModes);
    }

    /* Enforce video-snapshot-supported to true */
    params.set(android::CameraParameters::KEY_VIDEO_SNAPSHOT_SUPPORTED, "true");
    ALOGV("%s: Fixed: video-snapshot-supported: true", __FUNCTION__);

#ifdef LOG_PARAMETERS
    ALOGV("%s: Fixed parameters:", __FUNCTION__);
    params.dump();
#endif

    android::String8 strParams = params.flatten();
    char *ret = strdup(strParams.string());

    return ret;
}

static char *camera_fixup_setparams(int id __unused, const char *settings)
{
    android::CameraParameters params;
    params.unflatten(android::String8(settings));

#ifdef LOG_PARAMETERS
    ALOGV("%s: Original parameters:", __FUNCTION__);
    params.dump();
#endif

#ifdef LOG_PARAMETERS
    ALOGV("%s: Fixed parameters:", __FUNCTION__);
    params.dump();
#endif

    android::String8 strParams = params.flatten();
    char *ret = strdup(strParams.string());

return ret;
}

/*******************************************************************
 * Implementation of camera_device_ops functions
 *******************************************************************/
static char *camera_get_parameters(struct camera_device *device);
static int camera_set_parameters(struct camera_device *device, const char *params);

static int camera_set_preview_window(struct camera_device *device,
        struct preview_stream_ops *window)
{
    int rc = 0;
    if (!device)
        return -EINVAL;

    gPreviewWindow = window;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (gPreviewWindow != 0) {
        rc = VENDOR_CALL(device, set_preview_window, window);

        if (gPreviewStartDeferred) {
            ALOGV("%s call deferred start_preview", __FUNCTION__);
            gPreviewStartDeferred = false;
            VENDOR_CALL(device, start_preview);
        }
    }

   return rc;
}

static void camera_set_callbacks(struct camera_device *device,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void *user)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, set_callbacks, notify_cb, data_cb, data_cb_timestamp, get_memory, user);
}

static void camera_enable_msg_type(struct camera_device *device,
        int32_t msg_type)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X  msg_type=%s", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor),  msg2chr(msg_type));

    VENDOR_CALL(device, enable_msg_type, msg_type);
}

static void camera_disable_msg_type(struct camera_device *device,
        int32_t msg_type)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X  msg_type=%s", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor),  msg2chr(msg_type));

    VENDOR_CALL(device, disable_msg_type, msg_type);
}

static int camera_msg_type_enabled(struct camera_device *device,
        int32_t msg_type)
{
    if (!device)
        return 0;

    ALOGV("%s->%08X->%08X  msg_type=%s", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor),  msg2chr(msg_type));


    return VENDOR_CALL(device, msg_type_enabled, msg_type);
}

static int camera_start_preview(struct camera_device *device)
{
    int rc = 0;

    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (gPreviewWindow != 0) {
        rc = VENDOR_CALL(device, start_preview);
    } else {
        ALOGV("%s invalid preview window, defer start_preview", __FUNCTION__);
        gPreviewStartDeferred = true;
    }

   return rc;
}

static void camera_stop_preview(struct camera_device *device)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, stop_preview);
}

static int camera_preview_enabled(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (gPreviewStartDeferred) {
        ALOGV("%s deferred start_preview, return 1", __FUNCTION__);
        return 1;
    } else {
        return VENDOR_CALL(device, preview_enabled);
    }
}

static int camera_store_meta_data_in_buffers(struct camera_device *device, int enable)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, store_meta_data_in_buffers, enable);
}

static int camera_start_recording(struct camera_device *device)
{
    if (!device)
        return EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, start_recording);
}

static void camera_stop_recording(struct camera_device *device)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, stop_recording);
}

static int camera_recording_enabled(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, recording_enabled);
}

static void camera_release_recording_frame(struct camera_device *device, const void *opaque)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, release_recording_frame, opaque);
}

static int camera_auto_focus(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, auto_focus);
}

static int camera_cancel_auto_focus(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    /* If there is no preview window yet */
    if (gPreviewWindow == 0) {
        /* The cancel_auto_focus has no ill effects */
        return VENDOR_CALL(device, cancel_auto_focus);
    } else {
        /* Otherwise block it as it crashes the camera */
        ALOGV("%s->BLOCKED as it crashes the camera app!", __FUNCTION__);
    }

    return 0;
}

static int camera_take_picture(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, take_picture);
}

static int camera_cancel_picture(struct camera_device *device)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, cancel_picture);
}

static int camera_set_parameters(struct camera_device *device, const char *params)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    char *tmp = camera_fixup_setparams(CAMERA_ID(device), params);

    return VENDOR_CALL(device, set_parameters, tmp);
}

static char *camera_get_parameters(struct camera_device *device)
{
    if (!device)
        return NULL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    char *params = VENDOR_CALL(device, get_parameters);

    char *tmp = camera_fixup_getparams(CAMERA_ID(device), params);
    VENDOR_CALL(device, put_parameters, params);
    params = tmp;

    return params;
}

static void camera_put_parameters(__unused struct camera_device *device, char *params)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (params)
        free(params);
}

static int camera_send_command(struct camera_device *device,
        int32_t cmd, int32_t arg1, int32_t arg2)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, send_command, cmd, arg1, arg2);
}

static void camera_release(struct camera_device *device)
{
    if (!device)
        return;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    VENDOR_CALL(device, release);
}

static int camera_dump(struct camera_device *device, int fd)
{
    if (!device)
        return -EINVAL;

    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device, (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    return VENDOR_CALL(device, dump, fd);
}

static int camera_device_close(hw_device_t *device)
{
    int ret = 0;
    wrapper_camera_device_t *wrapper_dev = NULL;

    ALOGV("%s", __FUNCTION__);

    android::Mutex::Autolock lock(gCameraWrapperLock);

    if (!device) {
        ret = -EINVAL;
    } else {
        wrapper_dev = (wrapper_camera_device_t*) device;

        wrapper_dev->vendor->common.close((hw_device_t*)wrapper_dev->vendor);
        if (wrapper_dev->base.ops)
            free(wrapper_dev->base.ops);
        free(wrapper_dev);
    }

    gPreviewWindow = 0;
    gPreviewStartDeferred = false;
    return ret;
}

/*******************************************************************
 * Implementation of camera_module functions
 *******************************************************************/

/*
 * Open device handle to one of the cameras
 *
 * Assume camera service will keep singleton of each camera
 * so this function will always only be called once per camera instance
 */

static int camera_device_open(const hw_module_t *module, const char *name,
        hw_device_t **device)
{
    int rv = 0;
    int num_cameras = 0;
    int camera_id;
    wrapper_camera_device_t *camera_device = NULL;
    camera_device_ops_t *camera_ops = NULL;

    android::Mutex::Autolock lock(gCameraWrapperLock);

    ALOGV("%s", __FUNCTION__);

    if (name != NULL) {
        if (check_vendor_module())
            return -EINVAL;

        camera_id = atoi(name);
        num_cameras = gVendorModule->get_number_of_cameras();

        if (camera_id > num_cameras) {
            ALOGE("Camera service provided camera_id out of bounds, "
                    "camera_id = %d, num supported = %d",
                    camera_id, num_cameras);
            rv = -EINVAL;
            goto fail;
        }

        camera_device = (wrapper_camera_device_t*) malloc(sizeof(*camera_device));
        if (!camera_device) {
            ALOGE("camera_device allocation fail");
            rv = -ENOMEM;
            goto fail;
        }
        memset(camera_device, 0, sizeof(*camera_device));
        camera_device->id = camera_id;

        rv = gVendorModule->common.methods->open((const hw_module_t*)gVendorModule, name,(hw_device_t**)&(camera_device->vendor));
        if (rv) {
            ALOGE("Vendor camera open fail");
            goto fail;
        }
        ALOGV("%s: Got vendor camera device 0x%08X", __FUNCTION__, (uintptr_t)(camera_device->vendor));

        camera_ops = (camera_device_ops_t*)malloc(sizeof(*camera_ops));
        if (!camera_ops) {
            ALOGE("camera_ops allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        memset(camera_ops, 0, sizeof(*camera_ops));

        camera_device->base.common.tag = HARDWARE_DEVICE_TAG;
        camera_device->base.common.version = CAMERA_DEVICE_API_VERSION_1_0;
        camera_device->base.common.module = (hw_module_t *)(module);
        camera_device->base.common.close = camera_device_close;
        camera_device->base.ops = camera_ops;

        camera_ops->set_preview_window = camera_set_preview_window;
        camera_ops->set_callbacks = camera_set_callbacks;
        camera_ops->enable_msg_type = camera_enable_msg_type;
        camera_ops->disable_msg_type = camera_disable_msg_type;
        camera_ops->msg_type_enabled = camera_msg_type_enabled;
        camera_ops->start_preview = camera_start_preview;
        camera_ops->stop_preview = camera_stop_preview;
        camera_ops->preview_enabled = camera_preview_enabled;
        camera_ops->store_meta_data_in_buffers = camera_store_meta_data_in_buffers;
        camera_ops->start_recording = camera_start_recording;
        camera_ops->stop_recording = camera_stop_recording;
        camera_ops->recording_enabled = camera_recording_enabled;
        camera_ops->release_recording_frame = camera_release_recording_frame;
        camera_ops->auto_focus = camera_auto_focus;
        camera_ops->cancel_auto_focus = camera_cancel_auto_focus;
        camera_ops->take_picture = camera_take_picture;
        camera_ops->cancel_picture = camera_cancel_picture;
        camera_ops->set_parameters = camera_set_parameters;
        camera_ops->get_parameters = camera_get_parameters;
        camera_ops->put_parameters = camera_put_parameters;
        camera_ops->send_command = camera_send_command;
        camera_ops->release = camera_release;
        camera_ops->dump = camera_dump;

        *device = &camera_device->base.common;
    }

    return rv;

fail:
    if (camera_device) {
        free(camera_device);
        camera_device = NULL;
    }
    if (camera_ops) {
        free(camera_ops);
        camera_ops = NULL;
    }
    *device = NULL;
    return rv;
}

static int camera_mod_get_number_of_cameras(void)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->get_number_of_cameras();
}

static int camera_mod_get_camera_info(int camera_id, struct camera_info *info)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->get_camera_info(camera_id, info);
}

