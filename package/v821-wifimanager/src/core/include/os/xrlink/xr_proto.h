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


#ifndef __XR_PROTO_H__
#define __XR_PROTO_H__

#define XR_SSID_LEN 	32
#define XR_MAC_LEN		6
#define XR_BSSID_LEN	6
#define STORE_LIST_NUM	3

//Op: XR_WIFI_HOST_ON
typedef enum {
	XR_WIFI_STATION = 0,
	XR_WIFI_AP,
	XR_WIFI_MONITOR,
	XR_WIFI_AP_STATION,
	XR_WIFI_AP_MONITOR,
	XR_WIFI_MODE_UNKNOWN,
} xr_wifi_mode_t;

typedef struct {
	uint8_t mode;
} xr_wifi_on_t;

enum XR_WIFI_SEC {
	XR_WIFI_SEC_NONE = 0b0,
	XR_WIFI_SEC_WEP = 0b1,
	XR_WIFI_SEC_WPA_PSK = 0b10,
	XR_WIFI_SEC_WPA2_PSK = 0b100,
	XR_WIFI_SEC_WPA2_PSK_SHA256 = 0b1000,
	XR_WIFI_SEC_WPA3_PSK = 0b10000,
	XR_WIFI_SEC_EAP = 0b100000,
};

//Op: XR_WIFI_HOST_STA_CONNECT
typedef struct {
	char ssid[XR_SSID_LEN];
	uint8_t ssid_len;
	char pwd[64];
	uint8_t pwd_len;
	uint8_t sec;
	uint8_t fast_connect;
	/*only encryption method wep is valid*/
	int auth_alg;
	uint8_t bssid[XR_BSSID_LEN];
} xr_wifi_sta_cn_t;

typedef enum {
	XR_WIFI_FALSE,
	XR_WIFI_TRUE,
} xr_bool_t;

typedef struct {
	uint8_t enable;
} xr_sta_auto_reconnect_t;

//Op: XR_WIFI_HOST_STA_GET_INFO
//NULL

//Op: XR_WIFI_HOST_AP_ENABLE
typedef struct {
	char ssid[XR_SSID_LEN + 1];
	char pwd[64];
	uint8_t sec;
	uint8_t channel;
	uint8_t ip_addr[4];
	uint8_t netmask[4];
	uint8_t gw[4];
	uint8_t max_num_sta;
} xr_wifi_ap_config_t;

//Op: XR_WIFI_HOST_AP_GET_INF
//NULL


//Op: XR_WIFI_HOST_MONITOR_ENABLE
typedef struct {
	uint8_t channel;
} xr_wifi_monitor_enable_t;

//Op: XR_WIFI_HOST_GET_SCAN_RES
//NULL


//Op: XR_WIFI_HOST_SET/GET_MAC
typedef struct {
	uint8_t ifname[5];
	uint8_t mac[6];
} xr_wifi_mac_info_t;

//Op: XR_WIFI_HOST_SEND_RAW,
typedef struct {
	uint32_t len;
	uint8_t *data;
} xr_wifi_send_raw_data_t;

//Op: XR_WIFI_DEV_DEV_STATUS,
enum XR_WIFI_DEV_STATUS {
	XR_WLAN_STATUS_DOWN = 0,
	XR_WLAN_STATUS_UP,
};

typedef struct {
	uint8_t status;
} xr_wifi_dev_status_t;

//Op: XR_WIFI_DEV_STA_CN_EV,
enum XR_STA_CONNECT_EVET {
	XR_WIFI_DISCONNECTED = 0,
	XR_WIFI_SCAN_STARTED,
	XR_WIFI_SCAN_FAILED,
	XR_WIFI_SCAN_RESULTS,
	XR_WIFI_NETWORK_NOT_FOUND,
	XR_WIFI_NETWORK_DOWN,
	XR_WIFI_PASSWORD_INCORRECT,
	XR_WIFI_AUTHENTIACATION,
	XR_WIFI_AUTH_REJECT,
	XR_WIFI_ASSOCIATING,
	XR_WIFI_ASSOC_REJECT,
	XR_WIFI_ASSOCIATED,
	XR_WIFI_4WAY_HANDSHAKE,
	XR_WIFI_GROUNP_HANDSHAKE,
	XR_WIFI_GROUNP_HANDSHAKE_DONE,
	XR_WIFI_CONNECTED,
	XR_WIFI_CONNECT_TIMEOUT,
	XR_WIFI_DEAUTH,
	XR_WIFI_DHCP_START,
	XR_WIFI_DHCP_TIMEOUT,
	XR_WIFI_DHCP_SUCCESS,
	XR_WIFI_TERMINATING,
	XR_WIFI_UNKNOWN,
};

typedef struct {
	uint8_t event;
} xr_wifi_sta_cn_event_t;

//Op: XR_WIFI_DEV_STA_INFO
typedef struct {
	int id;
	int freq;
	int rssi;
	uint8_t bssid[6];
	char ssid[XR_SSID_LEN + 1];
	uint8_t mac_addr[6];
	uint8_t ip_addr[4];
	uint8_t netmask[4];
	uint8_t gw[4];
	uint8_t sec;
} xr_wifi_sta_info_t;


//Op: XR_WIFI_DEV_AP_STATE,
typedef enum {
	XR_WIFI_AP_ON = 0,
	XR_WIFI_AP_OFF,
	XR_WIFI_AP_STA_DISCONNECTED,
	XR_WIFI_AP_STA_CONNECTED,
	XR_WIFI_AP_UNKNOWN,
} xr_wifi_ap_state_t;

typedef struct {
	xr_wifi_ap_state_t state;
	uint8_t ip_addr[4];
	uint8_t netmask[4];
	uint8_t gw[4];
} xr_wifi_ap_network_card_info_t;

//Op: XR_WIFI_DEV_SCAN_RES
typedef struct {
	uint8_t bssid[6];
	uint8_t ssid[XR_SSID_LEN];
	uint8_t ssid_len;
	uint16_t freq;
	int rssi;
	uint8_t key_mgmt;
} xr_wifi_scan_info_t;

typedef struct {
	uint16_t num;
	uint8_t ap_info[0];
} xr_wifi_scan_result_t;

typedef struct {
	uint8_t addr[XR_MAC_LEN];
} xr_wifi_ap_sta_info_t;

typedef struct {
	uint8_t enable;
	uint8_t ssid[XR_SSID_LEN];
	uint8_t ssid_len;
} xr_wifi_scan_param_t;

//Op: XR_WIFI_DEV_AP_INF
typedef struct {
	char ssid[XR_SSID_LEN + 1];
	char pwd[64];
	uint8_t sec;
	uint8_t channel;
	uint8_t sta_num;
	uint8_t dev_list[0];
} xr_wifi_ap_info_t;

//Op: XR_WIFI_DEV_MONITOR_STATE,
enum XR_MON_STATE {
	XR_WIFI_MONITOR_ON,
	XR_WIFI_MONITOR_OFF,
};

typedef struct {
	uint8_t state;
} xr_wifi_monitor_state_t;

//Op: XR_WIFI_DEV_MONITOR_DATA,
typedef struct {
	uint32_t len;
	uint8_t *data;
} xr_wifi_monitor_data_t;

typedef struct {
    uint16_t len;
    char ext_cmd_info[0];
} xr_wifi_ext_cmd_info_t;

typedef struct {
    uint32_t type;
    uint32_t len;
    uint8_t data[0];
} xr_wifi_expand_cmd_data_t;

/* event */
typedef enum {
    XR_WIFI_ID_EXPAND_CMD_STATUS,//cmd status
    XR_WIFI_ID_EXPAND_CMD_RAW_DATA_RCV,//raw data
    XR_WIFI_ID_EXPAND_CMD_PRINTF,// printf rx data
    XR_WIFI_ID_EXPAND_CMD_MAX,
} protocol_expand_cmd_event_t;

//Op: XR_WIFI_DEV_IND_IP_INFO,
typedef struct {
	uint8_t ip_addr[4];
	uint8_t netmask[4];
	uint8_t gw[4];
	uint8_t dns[4];
} xr_wifi_sta_ip_info_t;

typedef struct {
	int id;
	char ssid[XR_SSID_LEN];
	char bssid[XR_BSSID_LEN];
	char flags[16];
} xr_wifi_sta_list_nod_t;

typedef struct {
	int list_num;
	int sys_list_num;
	uint8_t list_nod[0];
} xr_wifi_sta_list_t;

typedef enum {
	WIFI_COUNTRY_CODE_AU = 0,
	WIFI_COUNTRY_CODE_CA,
	WIFI_COUNTRY_CODE_CN,
	WIFI_COUNTRY_CODE_DE,
	WIFI_COUNTRY_CODE_EU,
	WIFI_COUNTRY_CODE_FR,
	WIFI_COUNTRY_CODE_JP,
	WIFI_COUNTRY_CODE_RU,
	WIFI_COUNTRY_CODE_SA,
	WIFI_COUNTRY_CODE_US,
	WIFI_COUNTRY_CODE_NONE
}xr_wifi_country_code_t;

typedef struct {
	xr_wifi_country_code_t code;
}xr_wifi_country_code_info_t;

// XR_WIFI_HOST_SEND_RAW
typedef struct {
	uint32_t len;
	uint8_t data[0];
} xr_wifi_raw_data_t;

#define PRO_HOST_LOW_ID_NUM 50
/* opcode*/
typedef enum {
    XR_WIFI_HOST_ON = PRO_HOST_LOW_ID_NUM,
    XR_WIFI_HOST_OFF,
    XR_WIFI_HOST_STA_CONNECT,
    XR_WIFI_HOST_STA_DISCONNECT,
    XR_WIFI_HOST_STA_GET_INFO,
    XR_WIFI_HOST_AP_ENABLE,
    XR_WIFI_HOST_AP_DISABLE,
    XR_WIFI_HOST_AP_GET_INF,
    XR_WIFI_HOST_MONITOR_ENABLE,
    XR_WIFI_HOST_MONITOR_DISABLE,
    XR_WIFI_HOST_GET_SCAN_RES,
    XR_WIFI_HOST_SET_MAC,
    XR_WIFI_HOST_GET_MAC,
    XR_WIFI_HOST_SEND_RAW,
    XR_WIFI_HOST_STA_AUTO_RECONNECT,
    XR_WIFI_HOST_STA_LIST_NETWORKS,
    XR_WIFI_HOST_STA_REMOVE_NETWORKS,
    XR_WIFI_HOST_SET_COUNTRY_CODE,
    XR_WIFI_HOST_GET_COUNTRY_CODE,
    XR_WIFI_HOST_EXPEND_CMD,
    XR_WIFI_HOST_SET_STA_IP,
    XR_WIFI_HOST_ID_MAX,
} cfg_host_op_t;

typedef enum {
    XR_WIFI_DEV_DEV_STATUS = PRO_HOST_LOW_ID_NUM,
    XR_WIFI_DEV_STA_CN_EV,
    XR_WIFI_DEV_STA_INFO,
    XR_WIFI_DEV_AP_STATE,
    XR_WIFI_DEV_AP_INF,
    XR_WIFI_DEV_SCAN_RES,
    XR_WIFI_DEV_MAC,
    XR_WIFI_DEV_MONITOR_STATE,
    XR_WIFI_DEV_MONITOR_DATA,
    XR_WIFI_DEV_NETWORKS,
    XR_WIFI_DEV_COUNTRY_CODE,
	XR_WIFI_DEV_IND_IP_INFO,
    XR_WIFI_DEV_IND_RAW_DATA,
    XR_WIFI_DEV_EXPEND_CMD,
	XR_WIFI_DEV_ID_MAX,
} cfg_dev_op_t;

/* config payload*/
typedef struct {
	uint16_t type;
	uint16_t len;
	uint8_t param[0];
} xr_cfg_payload_t;

#endif
