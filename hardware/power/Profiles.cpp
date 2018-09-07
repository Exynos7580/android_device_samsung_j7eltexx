/*
 * Copyright (C) 2018 TeamNexus
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

#define LOG_TAG "universal7580-power-profiles"
#define LOG_NDEBUG 0

#include <log/log.h>

#include "Power.h"
#include "Profiles.h"
#include "../Utils.h"

#if defined(LOG_NDEBUG) && !LOG_NDEBUG
const bool LOG_DEBUG = true;
#else
const bool LOG_DEBUG = false;
#endif

namespace android {
namespace hardware {
namespace power {
namespace V1_0 {
namespace implementation {

using ::vendor::universal7580::Utils;

SecPowerProfile Profiles::kPowerProfileScreenOff;
SecPowerProfile Profiles::kPowerProfilePowerSave;
SecPowerProfile Profiles::kPowerProfileBiasPowerSave;
SecPowerProfile Profiles::kPowerProfileBalanced;
SecPowerProfile Profiles::kPowerProfileBiasPerformance;
SecPowerProfile Profiles::kPowerProfileHighPerformance;

map<string, SecPowerProfile *> Profiles::kProfileNameToData = {
    { "screen_off", &kPowerProfileScreenOff },
    { "power_save", &kPowerProfilePowerSave },
    { "bias_power_save", &kPowerProfileBiasPowerSave },
    { "balanced", &kPowerProfileBalanced },
    { "bias_performance", &kPowerProfileBiasPerformance },
    { "performance", &kPowerProfileHighPerformance },
};

void Profiles::loadProfiles() {
    if (Utils::isFile(PROFILES_PATH_USER)) {
        loadProfilesImpl(PROFILES_PATH_USER);
    } else if (Utils::isFile(PROFILES_PATH_VENDOR)) {
        loadProfilesImpl(PROFILES_PATH_VENDOR);
    } else if (Utils::isFile(PROFILES_PATH_SYSTEM)) {
        loadProfilesImpl(PROFILES_PATH_SYSTEM);
    } else {
        LOG_FATAL("Could not find valid power profiles XML file");
    }
}

void Profiles::loadProfilesImpl(const char *path) {
    xmlInitParser();

    xmlDoc *doc = xmlParseFile(path);
    xmlXPathContext *ctx = xmlXPathNewContext(doc);

    loadProfileImpl(
        &kPowerProfileScreenOff,
        ctx,
        "/profiles/screen_off");

    loadProfileImpl(
        &kPowerProfilePowerSave,
        ctx,
        "/profiles/power_save");

    loadProfileImpl(
        &kPowerProfileBiasPowerSave,
        ctx,
        "/profiles/bias_power_save");

    loadProfileImpl(
        &kPowerProfileBalanced,
        ctx,
        "/profiles/balanced");

    loadProfileImpl(
        &kPowerProfileBiasPerformance,
        ctx,
        "/profiles/bias_performance");

    loadProfileImpl(
        &kPowerProfileHighPerformance,
        ctx,
        "/profiles/performance");
}

static inline bool to_bool(const char *c) {
    return !strcmp(c, "true") || !strcmp(c, "TRUE") || !strcmp(c, "1");
}

static inline int to_int(const char *c) {
    return std::stoi(c);
}

static inline unsigned int to_uint(const char *c) {
    return (unsigned int)std::stoi(c);
}

#define XML_GET(_path)                                                \
    ({                                                                \
        char objPath[255];                                            \
        const char *result = NULL;                                    \
        strcpy(objPath, path);                                        \
        strcat(objPath, "/" _path);                                   \
        xmlXPathObject *obj =                                         \
            xmlXPathEvalExpression ((xmlChar *)objPath, ctx);         \
        if (!obj->nodesetval->nodeTab ||                              \
            !obj->nodesetval->nodeTab[0]) {        \
            ALOGE("loadProfileImpl: missing XML node '%s'", objPath); \
            result = NULL;                                            \
        } else {                                                      \
            xmlNode *node = obj->nodesetval->nodeTab[0];              \
            result = (const char *)node->children->content;           \
            if (LOG_DEBUG) ALOGV("Loaded XML node '%s': \"%s\"",      \
                objPath, result);                                     \
        }                                                             \
        result;                                                       \
    })

#define XML_GET_ATTR(_path, _attr)                                    \
    ({                                                                \
        char objPath[255];                                            \
        strcpy(objPath, path);                                        \
        if (_path) {                                                  \
            strcat(objPath, "/");                                     \
            strcat(objPath, _path);                                   \
        }                                                             \
        xmlXPathObject *obj =                                         \
            xmlXPathEvalExpression ((xmlChar *)objPath, ctx);         \
        const char *result = NULL;                                    \
        if (!obj ||                                                   \
            !obj->nodesetval ||                                       \
            !obj->nodesetval->nodeTab ||                              \
            !obj->nodesetval->nodeTab[0])                             \
        {                                                             \
            ALOGE("loadProfileImpl: missing XML node '%s'", objPath); \
            result = NULL;                                            \
        } else {                                                      \
            xmlNode *node = obj->nodesetval->nodeTab[0];              \
            xmlAttr *attr = node->properties;                         \
            while (attr) {                                            \
                if (!strcmp((const char *)attr->name, _attr)) {       \
                    result = (const char *)xmlNodeListGetString(node->doc, attr->children, 1); \
                    break;                                            \
                }                                                     \
                attr = attr->next;                                    \
            }                                                         \
        }                                                             \
        result;                                                       \
    })

#define __PROFILE_SET(_parent, _name, _path, _default, _type) \
    {                                                         \
        const char *c = XML_GET(_path);                       \
        if (c) {                                              \
            profile->_parent._name = to_##_type(c);           \
            profile->_parent.__##_name##_set = true;          \
        } else if (!profile->_parent.__##_name##_set) {       \
            profile->_parent._name = to_##_type(_default);    \
        }                                                     \
    }

#define __PROFILE_SET2(_name, _path, _default, _type)  \
    {                                                  \
        const char *c = XML_GET(_path);                \
        if (c) {                                       \
            profile->_name = to_##_type(c);            \
            profile->__##_name##_set = true;           \
        } else if (!profile->__##_name##_set) {        \
            profile->_name = to_##_type(_default);     \
        }                                              \
    }

#define PROFILE_SET(_parent, _name, _path, _default)           \
    {                                                          \
        const char *c = XML_GET(_path);                        \
        if (c) {                                               \
            profile->_parent._name = c;                        \
            profile->_parent.__##_name##_set = true;           \
        } else if (!profile->_parent.__##_name##_set) {        \
            profile->_parent._name = _default;                 \
        }                                                      \
    }

#define PROFILE_SET_BOOL(_parent, _name, _path, _default) \
    __PROFILE_SET(_parent, _name, _path, _default, bool); \
    ALOGV("loadProfileImpl: [%s] "  #_parent "." #_name " := '%s'", path, profile->_parent._name ? "true" : "false");

#define PROFILE_SET_INT(_parent, _name, _path, _default) \
    __PROFILE_SET(_parent, _name, _path, _default, int); \
    ALOGV("loadProfileImpl: [%s] " #_parent "." #_name " := '%d'", path, profile->_parent._name);

#define PROFILE_SET_UINT(_parent, _name, _path, _default) \
    __PROFILE_SET(_parent, _name, _path, _default, uint); \
    ALOGV("loadProfileImpl: [%s] " #_parent "." #_name " := '%u'", path, profile->_parent._name);

#define PROFILE_SET_BOOL2(_name, _path, _default) \
    __PROFILE_SET2(_name, _path, _default, bool); \
    ALOGV("loadProfileImpl: [%s] " #_name " := '%s'", path, profile->_name ? "true" : "false");

#define PROFILE_SET_INT2(_name, _path, _default) \
    __PROFILE_SET2(_name, _path, _default, int); \
    ALOGV("loadProfileImpl: [%s] " #_name " := '%d'", path, profile->_name);

#define PROFILE_SET_UINT2(_name, _path, _default) \
    __PROFILE_SET2(_name, _path, _default, uint); \
    ALOGV("loadProfileImpl: [%s] " #_name " := '%u'", path, profile->_name);

#define XML_GET_CPUCLUSTER(_cl, _cln)                                                                          \
    PROFILE_SET     (cpu._cl, governor,     "cpu/" #_cl "/governor",     "nexus");                             \
    PROFILE_SET_UINT(cpu._cl, freq_min,     "cpu/" #_cl "/freq_min",     "200000");                            \
    PROFILE_SET_UINT(cpu._cl, freq_max,     "cpu/" #_cl "/freq_max",     (_cln == 0 ? "1500000" : "2100000")); \
    PROFILE_SET_UINT(cpu._cl, freq_hispeed, "cpu/" #_cl "/freq_hispeed", (_cln == 0 ? "1500000" : "2100000")); \
    PROFILE_SET_UINT(cpu._cl, freq_boost,   "cpu/" #_cl "/freq_boost",   "0");                                 \
    if (profile->cpu._cl.cores.enabled) {                                                                      \
        PROFILE_SET_BOOL(cpu._cl.cores, core1, "cpu/" #_cl "/cores/core1", "true");                            \
        PROFILE_SET_BOOL(cpu._cl.cores, core2, "cpu/" #_cl "/cores/core2", "true");                            \
        PROFILE_SET_BOOL(cpu._cl.cores, core3, "cpu/" #_cl "/cores/core3", "true");                            \
        PROFILE_SET_BOOL(cpu._cl.cores, core4, "cpu/" #_cl "/cores/core4", "true");                            \
    }

#define XML_GET_CPUGOV(_gov)                                            \
    {                                                                   \
        char objPath[255];                                              \
        strcpy(objPath, path);                                          \
        strcat(objPath, "/cpu/" #_gov "/governor_data");                \
        xmlXPathObject *obj =                                           \
            xmlXPathEvalExpression ((xmlChar *)objPath, ctx);           \
        if (!obj->nodesetval->nodeTab ||                                \
            !obj->nodesetval->nodeTab[0] ||                             \
            !obj->nodesetval->nodeTab[0]->children) {                   \
            ALOGE("loadProfileImpl: missing XML node '%s'", objPath);   \
        } else {                                                        \
            xmlNode *node = obj->nodesetval->nodeTab[0]->children;      \
            int index = 0;                                              \
            do {                                                        \
                if (node->children) {                                   \
                    strcpy(profile->cpu._gov.governor_data[index].name, \
                        (const char *)node->name);                      \
                    strcpy(profile->cpu._gov.governor_data[index].data, \
                        (const char *)node->children->content);         \
                    index++;                                            \
                }                                                       \
                node = node->next;                                      \
            } while (node);                                             \
            profile->cpu._gov.governor_data[index].name[0] = 0;         \
            profile->cpu._gov.governor_data[index].data[0] = 0;         \
        }                                                               \
    }

#define XML_HAS_NODE(_path)                                           \
    ({                                                                \
        char objPath[255];                                            \
        strcpy(objPath, path);                                        \
        strcat(objPath, "/" _path);                                   \
        xmlXPathObject *obj =                                         \
            xmlXPathEvalExpression ((xmlChar *)objPath, ctx);         \
        bool result = true;                                           \
        if (!obj->nodesetval->nodeTab ||                              \
            !obj->nodesetval->nodeTab[0] ||                           \
            !obj->nodesetval->nodeTab[0]->children)                   \
        {                                                             \
            ALOGE("loadProfileImpl: missing XML node '%s'", objPath); \
            result = false;                                           \
        }                                                             \
        (result);                                                     \
    })

#define PROFILE_INIT(_name, _path)                                       \
    {                                                                    \
        if (XML_HAS_NODE(_path)) {                                       \
            PROFILE_SET_BOOL(_name, enabled, _path "/enabled", "false"); \
            profile->__##_name##_set = true;                             \
        } else {                                                         \
            profile->__##_name##_set = false;                            \
        }                                                                \
    }

#define PROFILE_INIT2(_parent, _name, _path)                                     \
    {                                                                            \
        if (XML_HAS_NODE(_path)) {                                               \
            PROFILE_SET_BOOL(_parent._name, enabled, _path "/enabled", "false"); \
            profile->_parent.__##_name##_set = true;                             \
        } else {                                                                 \
            profile->_parent.__##_name##_set = false;                            \
        }                                                                        \
    }


void Profiles::loadProfileImpl(SecPowerProfile *profile, xmlXPathContext *ctx, const char *path) {
    PROFILE_SET_BOOL2(enabled, "enabled", "false");
    
    PROFILE_INIT (cpu,                         "cpu");
    PROFILE_INIT2(cpu,           cluster1,     "cpu/cluster1");
    PROFILE_INIT2(cpu.cluster1,  cores,        "cpu/cluster1/cores");
    PROFILE_INIT2(cpu,           cluster2,     "cpu/cluster2");
    PROFILE_INIT2(cpu.cluster2,  cores,        "cpu/cluster2/cores");
    PROFILE_INIT (cpusets,                     "cpusets");
    PROFILE_INIT (gpu,                         "gpu");
    PROFILE_INIT2(gpu,           dvfs,         "gpu/dvfs");
    PROFILE_INIT2(gpu,           highspeed,    "gpu/highspeed");
    PROFILE_INIT (kernel,                      "kernel");
    PROFILE_INIT (input_booster,               "input_booster");

    if (!profile->enabled) {
        return;
    }

    {
        const char *parent = XML_GET_ATTR(nullptr, "extends");
        if (parent) {
            ALOGI("Profile '%s' inherits from profile '%s'", path, parent);

            const SecPowerProfile *parentProfile = getProfileData(parent);
            memcpy(profile, parentProfile, sizeof(SecPowerProfile));
        } else {
            ALOGI("Profile '%s' is not extending any profile", path);
        }
    }

    if (profile->cpu.enabled) {
        if (profile->cpu.cluster1.enabled) {
            XML_GET_CPUCLUSTER(cluster1, 0);
            XML_GET_CPUGOV(cluster1);
        }

        if (profile->cpu.cluster2.enabled) {
            XML_GET_CPUCLUSTER(cluster2, 1);
            XML_GET_CPUGOV(cluster2);
        }
    }

    if (profile->cpusets.enabled) {
        PROFILE_SET(cpusets, defaults,          "cpusets/default",           "0-3");
        PROFILE_SET(cpusets, foreground,        "cpusets/foreground",        "0-3");
        PROFILE_SET(cpusets, background,        "cpusets/background",        "0-3");
        PROFILE_SET(cpusets, system_background, "cpusets/system_background", "0-3");
        PROFILE_SET(cpusets, top_app,           "cpusets/top_app",           "0-3");
    }

    if (profile->gpu.enabled) {
        if (profile->gpu.dvfs.enabled) {
            PROFILE_SET_UINT(gpu.dvfs, freq_min, "gpu/dvfs/freq_min", "160");
            PROFILE_SET_UINT(gpu.dvfs, freq_max, "gpu/dvfs/freq_max", "800");
        }
        if (profile->gpu.highspeed.enabled) {
            PROFILE_SET_UINT(gpu.highspeed, freq, "gpu/highspeed/freq", "800");
            PROFILE_SET_UINT(gpu.highspeed, load, "gpu/highspeed/load", "99");
        }
    }

    if (profile->kernel.enabled) {
        PROFILE_SET_BOOL(kernel, pewq, "kernel/pewq", "false");
    }

    if (profile->input_booster.enabled) {
        PROFILE_SET(input_booster, tail, "input_booster/tail", "0 0 0 0 0 0");
        PROFILE_SET(input_booster, head, "input_booster/head", "0 0 0 0 0 0");
    }
}

const SecPowerProfile* Profiles::getProfileData(SecPowerProfiles profile) {
    switch_uint32_t (profile)
    {
        case_uint32_t (SecPowerProfiles::SCREEN_OFF):
        {
            ALOGV("%s: returning kPowerProfileScreenOff", __func__);
            return &kPowerProfileScreenOff;
        }
        case_uint32_t (SecPowerProfiles::POWER_SAVE):
        {
            ALOGV("%s: returning kPowerProfilePowerSave", __func__);
            return &kPowerProfilePowerSave;
        }
        case_uint32_t (SecPowerProfiles::BIAS_POWER_SAVE):
        {
            ALOGV("%s: returning kPowerProfileBiasPowerSave", __func__);
            return &kPowerProfileBiasPowerSave;
        }
        case_uint32_t (SecPowerProfiles::BALANCED):
        {
            ALOGV("%s: returning kPowerProfileBalanced", __func__);
            return &kPowerProfileBalanced;
        }
        case_uint32_t (SecPowerProfiles::BIAS_PERFORMANCE):
        {
            ALOGV("%s: returning kPowerProfileBiasPerformance", __func__);
            return &kPowerProfileBiasPerformance;
        }
        case_uint32_t (SecPowerProfiles::HIGH_PERFORMANCE):
        {
            ALOGV("%s: returning kPowerProfileHighPerformance", __func__);
            return &kPowerProfileHighPerformance;
        }
    }
    return nullptr;
}

const SecPowerProfile* Profiles::getProfileData(string profileName) {
    return kProfileNameToData[profileName];
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace power
}  // namespace hardware
}  // namespace android
