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

#ifndef ZERO_HARDWARE_POWER_V1_0_PROFILES_H
#define ZERO_HARDWARE_POWER_V1_0_PROFILES_H

#define PROFILES_PATH_USER    "/data/system/power_profiles.xml"
#define PROFILES_PATH_VENDOR  "/vendor/etc/power_profiles.xml"
#define PROFILES_PATH_SYSTEM  "/system/etc/power_profiles.xml"

#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "Power.h"

namespace android {
namespace hardware {
namespace power {
namespace V1_0 {
namespace implementation {

using ::std::string;
using ::std::map;

#define PROFILE_DEFINE(_type, _name) \
    _type _name; \
    bool __##_name##_set

#define PROFILE_DEFINE2(_type, _name, _size) \
    _type _name[_size]; \
    bool __##_name##_set

#define PROFILE_INT(_name) \
    PROFILE_DEFINE(int, _name)

#define PROFILE_UINT(_name) \
    PROFILE_DEFINE(unsigned int, _name)

#define PROFILE_BOOL(_name) \
    PROFILE_DEFINE(bool, _name)

#define PROFILE_STRING(_name) \
    PROFILE_DEFINE(string, _name)

#define PROFILE_WRITE(_path, _parent, _name) \
    if (data->_parent.__##_name##_set) \
        Utils::write(_path, data->_parent._name);

struct SecPowerProfileCpuCluster {

    PROFILE_BOOL(enabled);

    PROFILE_STRING(governor);

    PROFILE_UINT(freq_min);
    PROFILE_UINT(freq_max);
    PROFILE_UINT(freq_hispeed);
    PROFILE_UINT(freq_boost);

    PROFILE_DEFINE(struct {

        PROFILE_BOOL(enabled);

        PROFILE_BOOL(core1);
        PROFILE_BOOL(core2);
        PROFILE_BOOL(core3);
        PROFILE_BOOL(core4);

    }, cores);

    PROFILE_DEFINE2(struct {

        char name[64];

        char data[64];

    }, governor_data, 64);
};

struct SecPowerProfile {

    PROFILE_BOOL(enabled);

    PROFILE_DEFINE(struct {

        PROFILE_BOOL(enabled);

        PROFILE_DEFINE(SecPowerProfileCpuCluster, cluster1);

        PROFILE_DEFINE(SecPowerProfileCpuCluster, cluster2);

    }, cpu);

    PROFILE_DEFINE(struct {

        PROFILE_BOOL(enabled);

        PROFILE_STRING(defaults);
        PROFILE_STRING(foreground);
        PROFILE_STRING(background);
        PROFILE_STRING(system_background);
        PROFILE_STRING(top_app);

    }, cpusets);

    PROFILE_DEFINE(struct {

        PROFILE_BOOL(enabled);

        PROFILE_DEFINE(struct {

            PROFILE_BOOL(enabled);

            PROFILE_UINT(freq_min);
            PROFILE_UINT(freq_max);

        }, dvfs);

        PROFILE_DEFINE(struct {

            PROFILE_BOOL(enabled);

            PROFILE_UINT(freq);
            PROFILE_UINT(load);

        }, highspeed);

    }, gpu);

    PROFILE_DEFINE(struct {

        PROFILE_BOOL(enabled);

        PROFILE_BOOL(pewq);

    }, kernel);
    

    PROFILE_DEFINE(struct {

        PROFILE_BOOL(enabled);

        PROFILE_STRING(tail);

        PROFILE_STRING(head);

    }, input_booster);

};

struct Profiles {
    static void loadProfiles();
    static const SecPowerProfile* getProfileData(SecPowerProfiles profile);
    static const SecPowerProfile* getProfileData(string profileName);

private:
    static void loadProfilesImpl(const char *path);
    static void loadProfileImpl(SecPowerProfile *profile, xmlXPathContext *ctx, const char *path);

    static map<string, SecPowerProfile *> kProfileNameToData;
    
    static SecPowerProfile kPowerProfileScreenOff;
    static SecPowerProfile kPowerProfilePowerSave;
    static SecPowerProfile kPowerProfileBiasPowerSave;
    static SecPowerProfile kPowerProfileBalanced;
    static SecPowerProfile kPowerProfileBiasPerformance;
    static SecPowerProfile kPowerProfileHighPerformance;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace power
}  // namespace hardware
}  // namespace android

#endif // ZERO_HARDWARE_POWER_V1_0_PROFILES_H
