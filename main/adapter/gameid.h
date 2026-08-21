/*
 * Copyright (c) 2019-2022, Jacques Gagnon
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _GAMEID_H_
#define _GAMEID_H_

#include "adapter/adapter.h"

int32_t gid_update(struct raw_fb *fb_data);
int32_t gid_update_sys(struct raw_fb *fb_data);
char *gid_get(void);

int32_t sd_info_update(struct raw_fb *fb_data);
uint32_t sd_info_get_total(void);
uint32_t sd_info_get_free(void);

#endif /* _GAMEID_H_ */
