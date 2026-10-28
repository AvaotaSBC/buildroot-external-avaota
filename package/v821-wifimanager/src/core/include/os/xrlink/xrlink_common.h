/*
 * Copyright (C) 2022 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef __WLAN_CMD_H__
#define __WLAN_CMD_H__
#include "xr_proto.h"
#include <wifimg.h>

typedef void (*xr_wifi_msg_cb_t)(char *data, uint32_t len);
typedef void (*xr_wifi_expand_msg_cb_t)(char *data, uint32_t len);

os_net_status_t xr_wifi_init(xr_wifi_msg_cb_t wifi_msg_cb, wifi_mode_t mode);

os_net_status_t xr_wifi_deinit(void);

os_net_status_t xrlink_send_cmd(uint32_t len,cfg_host_op_t type,void *param);

os_net_status_t xr_wifi_on(wifi_mode_t mode);

os_net_status_t xr_wifi_off(wifi_mode_t mode);

/* input parameter NULL means unregister*/
os_net_status_t xr_wifi_register_or_unregister_expand_msg_cb(xr_wifi_expand_msg_cb_t wifi_expand_msg);

void* xr_get_dev_event_status(uint16_t type);

uint8_t char2uint8(char* trs);

void wmg_system(const char *cmd, char *output, int size);

#define CMD_HEAD_LEN (sizeof(xr_cfg_payload_t))

#define WAIT_DEV_NORMOL_TO_MS  (5000)
#define WAIT_DEV_CONNECT_TO_MS (15000)
#define WAIT_DEV_CLEAN_TO_MS  (10)

#ifdef SUPPORT_STA_AP_MODE
#define AP_NETWORK_CARD  "wlan1"
#else
#define AP_NETWORK_CARD  "wlan0"
#endif

#define MAX_NUM_STA_DEFAULT 4

#endif
