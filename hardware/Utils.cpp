/*
 * Copyright (C) 2018 TeamNexus
 * Copyright (C) 2018 Danny Wood
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

#define LOG_TAG "universal7580-utils"
// #define LOG_NDEBUG 0

#include <log/log.h>

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "Utils.h"

namespace vendor {
namespace universal7580 {

bool Utils::isFile(const string path) {
    return __stat_compare_mode(path, S_IFREG);
}

bool Utils::isDirectory(const string path) {
    return __stat_compare_mode(path, S_IFDIR);
}

int Utils::statMode(const string path) {
    struct stat fstat;
    const char *cpath = path.c_str();

    if (stat(cpath, &fstat) == -1) {
        return 0;
    }

    return fstat.st_mode;
}

bool Utils::canRead(const string path) {
    return __access(path, R_OK);
}

bool Utils::canWrite(const string path) {
    return __access(path, W_OK);
}

bool Utils::canExecute(const string path) {
    return __access(path, X_OK);
}

bool Utils::write(const string path, const string data) {
    ofstream file;
    FILE_TRY_OPEN(file, path);

    ALOGV("%s: store \"%s\" to \"%s\"", __func__, data.c_str(), path.c_str());
    file << data;

    file.close();
    return true;
}

bool Utils::write(const string path, const bool data) {
    return write(path, (data ? 1 : 0));
}

bool Utils::write(const string path, const int data) {
    return write(path, to_string(data));
}

bool Utils::write(const string path, const unsigned int data) {
    return write(path, to_string(data));
}

bool Utils::writeLegacy(const string path, const string data) {
    int fd, len;
    const char* cdata = data.c_str();

    FILE_LEGACY_TRY_OPEN(path, O_WRONLY);

    len = ::write(fd, cdata, strlen(cdata));
    if (len < 0) {
        ALOGE("%s: failed to write to \"%s\": %s (%d)", __func__, path.c_str(), strerror(errno), errno);
        goto exit;
    }

    ALOGV("%s: store \"%s\" to \"%s\"", __func__, cdata, path.c_str());

exit:
    close(fd);
    return true;
}

bool Utils::writeLegacy(const string path, const bool data) {
    return writeLegacy(path, (data ? 1 : 0));
}

bool Utils::writeLegacy(const string path, const int data) {
    return writeLegacy(path, to_string(data));
}

bool Utils::writeLegacy(const string path, const unsigned int data) {
    return writeLegacy(path, to_string(data));
}

map<int, string> Utils::kCpuGovernors;

bool Utils::updateCpuGov(const int core) {
    string cpugov;
    ostringstream path;

    ASSERT_CPU_CORE();

    path << "/sys/devices/system/cpu/cpu" << core << "/cpufreq/scaling_governor";
    if (!read(path.str(), cpugov)) {
        return false;
    }

    kCpuGovernors[core] = cpugov;
    return true;
}

bool Utils::assertCpuGov(const int core, const string asserted_cpugov) {
    ASSERT_CPU_CORE();
    ALOGV("%s: comparing %s == %s", __func__, kCpuGovernors[core].c_str(), asserted_cpugov.c_str());
    return (kCpuGovernors[core] == asserted_cpugov);
}

bool Utils::writeCpuGov(const int core, const string file, const string data) {
    ostringstream path;

    ASSERT_CPU_CORE();

    path << "/sys/devices/system/cpu/cpu" << core << "/cpufreq/" << kCpuGovernors[core] << "/" << file;
    if (!isFile(path.str())) {
        return false;
    }

    return write(path.str(), data);
}

bool Utils::writeCpuGov(const int core, const string file, const bool data) {
    return writeCpuGov(core, file, (data ? 1 : 0));
}

bool Utils::writeCpuGov(const int core, const string file, const int data) {
    return writeCpuGov(core, file, to_string(data));
}

bool Utils::writeCpuGov(const int core, const string file, const unsigned int data) {
    return writeCpuGov(core, file, to_string(data));
}

/***********************************
 * Reading
 */
bool Utils::read(const string path, string &str) {
    ifstream file;

    FILE_TRY_OPEN(file, path);

    if (!getline(file, str)) {
        ALOGE("%s: failed to read from \"%s\"", __func__, path.c_str());
        return false;
    }

    ALOGV("%s: read \"%s\" from \"%s\"", __func__, str.c_str(), path.c_str());

    file.close();
    return true;
}

bool Utils::read(const string path, bool &f) {
    int out = 0;

    if (!read(path, out)) {
        return false;
    }

    f = (out ? true : false);
    return true;
}

bool Utils::read(const string path, int &v) {
    string line;

    if (!read(path, line)) {
        return false;
    }

    v = stoi(line);
    return true;
}

bool Utils::screenIsOn() {
    int brightness = 0;
    if (!Utils::read("/sys/devices/14800000.dsim/backlight/panel/brightness", brightness)) {
        return true;
    }
    return (brightness > 0);
}

/***********************************
 * Timing
 */
std::chrono::milliseconds Utils::getTime() {
    return duration_cast<std::chrono::milliseconds>(
        system_clock::now().time_since_epoch()
    );;
}

}  // namespace universal7580
}  // namespace vendor

