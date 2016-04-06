/*
 * Copyright (C) 2013 The CyanogenMod Project
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

#ifndef _ROUTING_H_
#define _ROUTING_H_

enum {
    OUT_DEVICE_SPEAKER,
    OUT_DEVICE_EARPIECE,
    OUT_DEVICE_HEADSET,
    OUT_DEVICE_HEADPHONES,
    OUT_DEVICE_BT_SCO,
    OUT_DEVICE_BT_SCO_HEADSET_OUT,
    OUT_DEVICE_BT_SCO_CARKIT,
    OUT_DEVICE_SPEAKER_AND_HEADSET,
    OUT_DEVICE_SPEAKER_AND_EARPIECE,
    OUT_DEVICE_TAB_SIZE,           /* number of rows in route_configs[][] */
    OUT_DEVICE_NONE,
    OUT_DEVICE_CNT
};

enum {
    IN_SOURCE_MIC,
    IN_SOURCE_CAMCORDER,
    IN_SOURCE_VOICE_RECOGNITION,
    IN_SOURCE_VOICE_COMMUNICATION,
    IN_SOURCE_VOICE_CALL,
    IN_SOURCE_VOICE_CALL_WB,
    IN_SOURCE_TAB_SIZE,            /* number of lines in route_configs[][] */
    IN_SOURCE_NONE,
    IN_SOURCE_CNT
};

struct route_config {
    const char * const output_route;
    const char * const input_route;
};

const struct route_config voice_speaker = {
    "incall_default-speaker",
    "incall_nb-speaker-mic"
};

const struct route_config voice_speaker_wb = {
    "incall_wb-speaker",
    "incall_wb-speaker-mic"
};

const struct route_config voice_earpiece = {
    "incall_default-handset",
    "incall_nb-handset-mic"
};

const struct route_config voice_earpiece_wb = {
    "incall_wb-handset",
    "incall_wb-handset-mic"
};

const struct route_config voice_headphones = {
    "incall_default-headphone",
    "incall_default-headphone-mic"
};

const struct route_config voice_headphones_wb = {
    "incall_wb-headphone",
    "incall_wb-headphone-mic"
};

const struct route_config voice_headset = {
    "incall_default-headset",
    "incall_default-headset-mic"
};

const struct route_config voice_headset_wb = {
    "incall_wb-headset",
    "incall_wb-headset-mic"
};

const struct route_config voice_bt_sco = {
    "incall_default-bt-sco-headset",
    "incall_default-bt-sco-headset-in",
};

const struct route_config voice_bt_sco_wb = {
    "incall_wb-bt-sco-headset",
    "incall_wb-bt-sco-headset-in",
};

const struct route_config voice_bt_sco_headset_out = {
    "incall_default-bt-sco-headset",
    "incall_default-bt-sco-headset-in",
};

const struct route_config voice_bt_sco_headset_out_wb = {
    "incall_wb-bt-sco-headset",
    "incall_wb-bt-sco-headset-in",
};

const struct route_config media_speaker = {
    "media-speaker",
    "media-mic"
};

const struct route_config media_earpiece = {
    "media-handset",
    "media-mic"
};

const struct route_config media_headphones = {
    "media-headset",
    "media-headphone-mic"
};

const struct route_config media_headset = {
    "media-headset",
    "media-headset-mic"
};

const struct route_config media_bt_sco = {
    "media-bt-sco-headset",
    "media-bt-sco-headset-in",
};

const struct route_config media_bt_sco_headset_out = {
    "media-bt-sco-headset",
    "media-bt-sco-headset-in",
};

const struct route_config camcorder_speaker = {
    "media-speaker",
    "camcorder-mic"
};

const struct route_config camcorder_headphones = {
    "media-headset",
    "camcorder-headset-mic"
};

const struct route_config camcorder_headset = {
    "media-headset",
    "camcorder-headset-mic"
};

const struct route_config voice_rec_speaker = {
    "media-speaker",
    "recording-mic"
};

const struct route_config voice_rec_headphones = {
    "media-headset",
    "recording-headphone-mic"
};

const struct route_config voice_rec_headset = {
    "media-headset",
    "recording-headset-mic"
};

const struct route_config communication_speaker = {
    "communication-speaker",
    "communication-speaker-mic"
};

const struct route_config communication_earpiece = {
    "communication-handset",
    "communication-handset-mic"
};

const struct route_config communication_headphones = {
    "communication-headphone",
    "communication-headphone-mic"
};

const struct route_config communication_headset = {
    "communication-headset",
    "communication-headset-mic"
};

const struct route_config speaker_and_headphones = {
    "media-speaker-headset",
    "media-mic"
};

const struct route_config bt_sco_carkit = {
    "media-bt-sco-headset",
    "media-bt-sco-headset-in",
};

const struct route_config none = {
    "none",
    "none"
};

const struct route_config * const route_configs[IN_SOURCE_TAB_SIZE]
                                               [OUT_DEVICE_TAB_SIZE] = {
    {   /* IN_SOURCE_MIC */
        &media_speaker,             /* OUT_DEVICE_SPEAKER */
        &media_earpiece,            /* OUT_DEVICE_EARPIECE */
        &media_headset,             /* OUT_DEVICE_HEADSET */
        &media_headphones,          /* OUT_DEVICE_HEADPHONES */
        &media_bt_sco,              /* OUT_DEVICE_BT_SCO */
        &media_bt_sco_headset_out,  /* OUT_DEVICE_BT_SCO_HEADSET_OUT */
        &bt_sco_carkit,             /* OUT_DEVICE_BT_SCO_CARKIT */
        &speaker_and_headphones,    /* OUT_DEVICE_SPEAKER_AND_HEADSET */
        &media_speaker              /* OUT_DEVICE_SPEAKER_AND_EARPIECE */
    },
    {   /* IN_SOURCE_CAMCORDER */
        &camcorder_speaker,         /* OUT_DEVICE_SPEAKER */
        &none,                      /* OUT_DEVICE_EARPIECE */
        &camcorder_headset,         /* OUT_DEVICE_HEADSET */
        &camcorder_headphones,      /* OUT_DEVICE_HEADPHONES */
        &media_bt_sco,              /* OUT_DEVICE_BT_SCO */
        &media_bt_sco_headset_out,  /* OUT_DEVICE_BT_SCO_HEADSET_OUT */
        &bt_sco_carkit,             /* OUT_DEVICE_BT_SCO_CARKIT */
        &speaker_and_headphones,    /* OUT_DEVICE_SPEAKER_AND_HEADSET */
        &camcorder_speaker          /* OUT_DEVICE_SPEAKER_AND_EARPIECE */
    },
    {   /* IN_SOURCE_VOICE_RECOGNITION */
        &voice_rec_speaker,         /* OUT_DEVICE_SPEAKER */
        &none,                      /* OUT_DEVICE_EARPIECE */
        &voice_rec_headset,         /* OUT_DEVICE_HEADSET */
        &voice_rec_headphones,      /* OUT_DEVICE_HEADPHONES */
        &media_bt_sco,              /* OUT_DEVICE_BT_SCO */
        &media_bt_sco_headset_out,  /* OUT_DEVICE_BT_SCO_HEADSET_OUT */
        &bt_sco_carkit,             /* OUT_DEVICE_BT_SCO_CARKIT */
        &speaker_and_headphones,    /* OUT_DEVICE_SPEAKER_AND_HEADSET */
        &voice_rec_speaker          /* OUT_DEVICE_SPEAKER_AND_EARPIECE */
    },
    {   /* IN_SOURCE_VOICE_COMMUNICATION */
        &communication_speaker,     /* OUT_DEVICE_SPEAKER */
        &communication_earpiece,    /* OUT_DEVICE_EARPIECE */
        &communication_headset,     /* OUT_DEVICE_HEADSET */
        &communication_headphones,  /* OUT_DEVICE_HEADPHONES */
        &media_bt_sco,              /* OUT_DEVICE_BT_SCO */
        &media_bt_sco_headset_out,  /* OUT_DEVICE_BT_SCO_HEADSET_OUT */
        &bt_sco_carkit,             /* OUT_DEVICE_BT_SCO_CARKIT */
        &speaker_and_headphones,    /* OUT_DEVICE_SPEAKER_AND_HEADSET */
        &communication_earpiece     /* OUT_DEVICE_SPEAKER_AND_EARPIECE */
    },
    {   /* IN_SOURCE_VOICE_CALL */
        &voice_speaker,             /* OUT_DEVICE_SPEAKER */
        &voice_earpiece,            /* OUT_DEVICE_EARPIECE */
        &voice_headset,             /* OUT_DEVICE_HEADSET */
        &voice_headphones,          /* OUT_DEVICE_HEADPHONES */
        &voice_bt_sco,              /* OUT_DEVICE_BT_SCO */
        &voice_bt_sco_headset_out,  /* OUT_DEVICE_BT_SCO_HEADSET_OUT */
        &bt_sco_carkit,             /* OUT_DEVICE_BT_SCO_CARKIT */
        &voice_headphones,          /* OUT_DEVICE_SPEAKER_AND_HEADSET */
        &voice_earpiece             /* OUT_DEVICE_SPEAKER_AND_EARPIECE */
    },
    {   /* IN_SOURCE_VOICE_CALL_WB */
        &voice_speaker_wb,          /* OUT_DEVICE_SPEAKER */
        &voice_earpiece_wb,         /* OUT_DEVICE_EARPIECE */
        &voice_headset_wb,          /* OUT_DEVICE_HEADSET */
        &voice_headphones_wb,       /* OUT_DEVICE_HEADPHONES */
        &voice_bt_sco_wb,           /* OUT_DEVICE_BT_SCO */
        &voice_bt_sco_headset_out_wb, /* OUT_DEVICE_BT_SCO_HEADSET_OUT */
        &bt_sco_carkit,             /* OUT_DEVICE_BT_SCO_CARKIT */
        &voice_headphones_wb,       /* OUT_DEVICE_SPEAKER_AND_HEADSET */
        &voice_earpiece_wb          /* OUT_DEVICE_SPEAKER_AND_EARPIECE */
    },
};

#endif
