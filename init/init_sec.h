
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

enum device_variant {
    UNKNOWN = -1,
    J700F,
    J700M,
    J700H
};

device_variant match(std::string bl)
{
    if (bl == "J700F") {
        return J700F;
    } else if (bl == "J700M") {
        return J700M;
    } else if (bl == "J700H") {
        return J700H;
    } else {
        return UNKNOWN;
    }
}
