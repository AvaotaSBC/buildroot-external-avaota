/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef _LINUX_COMMON_H
#define _LINUX_COMMON_H

#if __cplusplus
extern "C" {
#endif

#define PROCESS_MAX 32
#define IFACE_MAX 32
#define CONFIG_PATH_MAX 128

#define DEFAULT_WPA_STA_IFACE         "wlan0"
#define DEFAULT_WPA_STA_CONFIG_FILE   "/wpa_supplicant/wpa_supplicant.conf"
#define DEFAULT_WPA_STA_IFACE_PATH    "/wpa_supplicant/sockets"

#define DEFAULT_WPA_P2P_PROCESS       "p2p_supplicant"
#define DEFAULT_WPA_P2P_IFACE         "p2p-dev-wlan0"
#define DEFAULT_WPA_P2P_CONFIG_FILE   "/wpa_supplicant/wpa_supplicant_p2p.conf"
#define DEFAULT_WPA_P2P_IFACE_PATH    "/wpa_supplicant/sockets"

#define DEFAULT_HAPD_AP_IFACE         "wlan0"
#define DEFAULT_HAPD_CONFIG_FILE      "/hostapd/hostapd.conf"
#define DEFAULT_HAPD_AP_IFACE_PATH    "/hostapd/sockets"

#define DEFAULT_WPA_IFACE_PATH        "/wpa_supplicant/sockets"
#define DEFAULT_HAPD_IFACE_PATH       "/hostapd/sockets"

typedef int (* dispatch_event_t)(const char *event_str, int nread);

typedef struct {
	wifi_mode_t wifi_mode;
	dispatch_event_t dispatch_event;
	void *linux_mode_private_data;
} init_wpad_para_t;

uint8_t char2uint8(char* trs);
void wmg_system(const char *cmd, char *output, int size);
wmg_status_t stop_linux_process(char* process);
wmg_status_t start_linux_process(char* process, char* process_para);

wmg_status_t init_wpad(init_wpad_para_t wpad_para);
wmg_status_t deinit_wpad(wifi_mode_t wifi_mode);
/* p2p_gp_inf only for p2p module, other module input NULL */
wmg_status_t command_to_wpad(char const *cmd, char *reply, size_t reply_len, wifi_mode_t wifi_mode, char *p2p_gp_inf);

const char* get_hapd_ap_iface(void);

#if __cplusplus
};  // extern "C"
#endif

#endif  // _LINUX_COMMON_H
