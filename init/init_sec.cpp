/*
   Copyright (c) 2016, The CyanogenMod Project. All rights reserved.

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

   File Name : init_sec.c
   Create Date : 2016.04.13
 */

#include <stdlib.h>
#include <stdio.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

void make_me_dual()
{
	property_set("rild.libpath2", "/system/lib/libsec-ril-dsds.so");
	property_set("persist.radio.multisim.config", "dsds");
	property_set("ro.multisim.simslotcount", "2");
}

void vendor_load_properties()
{

    std::string bootloader = property_get("ro.bootloader");

    if (bootloader.find("J700F") == 0) {
        property_set("ro.build.fingerprint", "samsung/j7eltexx/j7elte:5.1.1/LMY48B/J700FXXU2APC4:user/release-keys");
        property_set("ro.build.description", "j7eltexx-user 5.1.1 LMY48B J700FXXU2APC4 release-keys");
        property_set("ro.product.model", "SM-J700F");
        property_set("ro.product.device", "j7elte");
	//make_me_dual();
    } else if (bootloader.find("J700M") == 0) {
        property_set("ro.build.fingerprint", "samsung/j7eltexx/j7elte:5.1.1/LMY47X/J700MUBU1APA1:user/release-keys");
        property_set("ro.build.description", "j7eltexx-user 5.1.1 LMY47X J700MUBU1APA1 release-keys");
        property_set("ro.product.model", "SM-J700M");
        property_set("ro.product.device", "j7elte");
    } else {
        property_set("ro.build.fingerprint", "samsung/j7e3gxx/j7e3g:5.1.1/LMY48B/J700HXXU2APC5:user/release-keys");
        property_set("ro.build.description", "j7e3gxx-user 5.1.1 LMY48B J700HXXU2APC5 release-keys");
        property_set("ro.product.model", "SM-J700H");
        property_set("ro.product.device", "j7e3g");
	//make_me_dual();
    }

    std::string device = property_get("ro.product.device");
    std::string devicename = property_get("ro.product.model");
    ERROR("Found bootloader id %s setting build properties for %s device\n", bootloader.c_str(), devicename.c_str());
}
