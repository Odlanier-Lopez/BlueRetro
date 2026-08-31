/*
 * Copyright (c) 2019-2022, Jacques Gagnon
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include "gameid.h"
#include "tools/ps1_gameid.h"

#define GAME_ID_BUF_LEN 23

static char gameid[GAME_ID_BUF_LEN] = {0};
static char tmp_gameid[GAME_ID_BUF_LEN] = {0};

static void set_pfx_gameid(uint8_t *data, uint32_t len) {
    if (len * 2 >= sizeof(gameid)) {
        len = (sizeof(gameid) - 1) / 2;
    }

    snprintf(tmp_gameid, GAME_ID_BUF_LEN, "%02X%02X%02X%02X%02X%02X%02X%02X",
        data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);
}

static void set_ps1_gameid(uint8_t *data, uint32_t len) {
    memcpy(tmp_gameid, data, len);
    ps1_gid_sanitize(tmp_gameid);
}

int32_t gid_update(struct raw_fb *fb_data) {
    memset(tmp_gameid, 0, sizeof(gameid));

    switch (wired_adapter.system_id) {
        case N64:
        case GC:
            set_pfx_gameid(fb_data->data, fb_data->header.data_len);
            break;
        case PSX:
        case PS2:
            set_ps1_gameid(fb_data->data, fb_data->header.data_len);
            break;
    }

    if (memcmp(gameid, tmp_gameid, sizeof(gameid)) != 0) {
        memcpy(gameid, tmp_gameid, sizeof(gameid));
        printf("# %s: %s\n", __FUNCTION__, gameid);
        return 1;
    }
    return 0;
}

int32_t gid_update_sys(struct raw_fb *fb_data) {
    memset(tmp_gameid, 0, sizeof(gameid));
    memcpy(tmp_gameid, fb_data->data, fb_data->header.data_len);

    if (memcmp(gameid, tmp_gameid, sizeof(gameid)) != 0) {
        memcpy(gameid, tmp_gameid, sizeof(gameid));
        printf("# %s: %s\n", __FUNCTION__, gameid);
        return 1;
    }
    return 0;
}

char *gid_get(void) {
    return gameid;
}
static uint32_t sd_total_gb = 0;
static uint32_t sd_free_gb = 0;

int32_t sd_info_update(struct raw_fb *fb_data) {
    uint32_t new_total = ((uint32_t)fb_data->data[0] << 24) | ((uint32_t)fb_data->data[1] << 16) |
                          ((uint32_t)fb_data->data[2] << 8)  | (uint32_t)fb_data->data[3];
    uint32_t new_free  = ((uint32_t)fb_data->data[4] << 24) | ((uint32_t)fb_data->data[5] << 16) |
                          ((uint32_t)fb_data->data[6] << 8)  | (uint32_t)fb_data->data[7];

    if (new_total != sd_total_gb || new_free != sd_free_gb) {
        sd_total_gb = new_total;
        sd_free_gb = new_free;
        printf("# %s: total=%luGB free=%luGB\n", __FUNCTION__, (unsigned long)sd_total_gb, (unsigned long)sd_free_gb);
        return 1;
    }
    return 0;
}

uint32_t sd_info_get_total(void) {
    return sd_total_gb;
}

uint32_t sd_info_get_free(void) {
    return sd_free_gb;
}
