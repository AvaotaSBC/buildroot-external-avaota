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


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <log_core.h>
#include <os_net_sync_notify.h>
#include <xr_proto.h>
#include <wifimg.h>
#include <xrlink_common.h>
#include <os_net_thread.h>
#include <api_action.h>
#include <wifimg_glue.h>
#include <wmg_sta.h>
#include <wifi_log.h>
#include <xrlink_get_config.h>

typedef struct {
	snfy_handle_t *sta_event_snfy_handle;
	wmg_bool_t sta_event_snfy_await_valid;
	wifi_sta_event_t event;
	snfy_handle_t *dev_event_snfy_handle;
	wmg_bool_t dev_event_snfy_await_valid;
	wifi_vend_cb_t vend_cb;
} xrlink_sta_private_data_t;

static wmg_sta_inf_object_t sta_inf_object;
static xrlink_sta_private_data_t xrlink_sta_private_data;
static wifi_secure_t scan_get_secure(int scan_time, char **ssid, char *ssid_buf, uint8_t *bssid);

static unsigned char convert_decimal(char str)
{
	unsigned char str_nu = 0;
	if(str >= '0' && str <='9') {
		str_nu = str - '0';
	} else if (str >= 'a' && str <='f') {
		str_nu = str - 'a' + 10;
	} else if (str >= 'A' && str <='F') {
		str_nu = str - 'A' + 10;
	} else {
		str_nu = 0;
	}
	return str_nu;
}

/*
 * ret: returns the converted length
 */
static int convert_utf8(char *str, char *buf)
{
	int i = 0, j = 0;
	while(str[i] != '\0') {
		if(((str[i] == '\\') && (str[i + 1] == 'x')) || ((str[i] == '\\') && (str[i+1] == 'X'))) {
			buf[j] = (convert_decimal(str[i + 2]) << 4) + convert_decimal(str[i + 3]);
			i += 3;
		} else {
			buf[j] = str[i];
		}
		j++;
		i++;
	}
	buf[j] = '\0';
	return strlen(buf);
}

/* Use a very short time to clear the signal volume */
static void clean_dev_and_sta_event(void)
{
	snfy_await(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_handle,
		WAIT_DEV_CLEAN_TO_MS);
	snfy_await(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle,
	WAIT_DEV_CLEAN_TO_MS);
}

static os_net_status_t xrlink_sta_send_cmd(uint32_t len, cfg_host_op_t type, void *param, int dev_event_flag, int sta_event_flag)
{
	if(dev_event_flag) {
		((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_await_valid = true;
	}
	if(sta_event_flag) {
		((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_await_valid = true;
	}
	return xrlink_send_cmd(len, type, param);
}

static void event_notify_to_sta_dev(wifi_sta_event_t event)
{
	if (sta_inf_object.sta_event_cb) {
		sta_inf_object.sta_event_cb(event);
	}
}

static void sta_event_notify(wifi_sta_event_t event)
{
	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->event = event;
	if(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_await_valid) {
		WMG_DEBUG("** sta: sta event effective monitoring time period, generating snfy **\n");
		snfy_ready(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle,
			&(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->event));
	}
	event_notify_to_sta_dev(event);
}

static wifi_sta_event_t get_sta_event(void)
{
	wifi_sta_event_t *sta_event = NULL;
	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_await_valid = true;
	sta_event = snfy_await(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle,
			WAIT_DEV_CONNECT_TO_MS);
	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_await_valid = false;
	if(sta_event != NULL) {
		return *sta_event;
	} else {
		return WIFI_UNKNOWN;
	}
}

static void dev_event_notify(char *data)
{
	if(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_await_valid) {
		WMG_DEBUG("** sta: dev event effective monitoring time period, generating snfy **\n");
		snfy_ready(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_handle, data);
	}
}

static void* get_dev_event(uint16_t *type)
{
	char *data = NULL;
	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_await_valid = true;
	data = snfy_await(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_handle,
				WAIT_DEV_NORMOL_TO_MS);
	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_await_valid = false;
	if(data != NULL) {
		xr_cfg_payload_t *payload = (xr_cfg_payload_t*) data;
		*type = payload->type;
		return payload->param;
	}
	return data;
}

static void xr_wifi_sta_network_ip_clean(void)
{
	char cmd[32] = {0};

	sprintf(cmd, "ifconfig wlan0 0.0.0.0");
	WMG_DEBUG("ip clean:%s\n", cmd);
	system(cmd);
}

static void xr_wifi_sta_cb(char *data,uint32_t len)
{
	xr_cfg_payload_t *payload = (xr_cfg_payload_t*) data;
	xr_wifi_sta_cn_event_t *cn_event;
	xr_wifi_raw_data_t *raw_data;

	WMG_DEBUG("sta: payload type:%d\n",payload->type);
	if(payload->type == XR_WIFI_DEV_STA_CN_EV) {
		cn_event = (xr_wifi_sta_cn_event_t *) payload->param;
		WMG_DEBUG("wifi sta event:%d\n", cn_event->event);
		switch(cn_event->event) {
			case XR_WIFI_DISCONNECTED:
				WMG_WARNG("wifi sta disconnect\n");
				xr_wifi_sta_network_ip_clean();
				sta_event_notify(WIFI_DISCONNECTED);
				break;
			case XR_WIFI_SCAN_STARTED:
				sta_event_notify(WIFI_SCAN_STARTED);
				break;
			case XR_WIFI_SCAN_FAILED:
				event_notify_to_sta_dev(WIFI_SCAN_FAILED);
				break;
			case XR_WIFI_SCAN_RESULTS:
				event_notify_to_sta_dev(WIFI_SCAN_RESULTS);
				break;
			case XR_WIFI_NETWORK_NOT_FOUND:
				sta_event_notify(WIFI_NETWORK_NOT_FOUND);
				break;
			case XR_WIFI_NETWORK_DOWN:
				xr_wifi_sta_network_ip_clean();
				sta_event_notify(WIFI_NETWORK_DOWN);
				break;
			case XR_WIFI_PASSWORD_INCORRECT:
				sta_event_notify(WIFI_PASSWORD_INCORRECT);
				break;
			case XR_WIFI_AUTHENTIACATION:
				event_notify_to_sta_dev(WIFI_AUTHENTIACATION);
				break;
			case XR_WIFI_AUTH_REJECT:
				sta_event_notify(WIFI_AUTH_REJECT);
				break;
			case XR_WIFI_ASSOCIATING:
				event_notify_to_sta_dev(WIFI_ASSOCIATING);
				break;
			case XR_WIFI_ASSOC_REJECT:
				sta_event_notify(WIFI_ASSOC_REJECT);
				break;
			case XR_WIFI_ASSOCIATED:
				event_notify_to_sta_dev(WIFI_ASSOCIATED);
				break;
			case XR_WIFI_4WAY_HANDSHAKE:
				event_notify_to_sta_dev(WIFI_4WAY_HANDSHAKE);
				break;
			case XR_WIFI_GROUNP_HANDSHAKE:
				event_notify_to_sta_dev(WIFI_GROUNP_HANDSHAKE);
				break;
			case XR_WIFI_GROUNP_HANDSHAKE_DONE:
				event_notify_to_sta_dev(WIFI_GROUNP_HANDSHAKE_DONE);
				break;
			case XR_WIFI_CONNECTED:
				event_notify_to_sta_dev(WIFI_CONNECTED);
				break;
			case XR_WIFI_CONNECT_TIMEOUT:
				sta_event_notify(WIFI_CONNECT_TIMEOUT);
				break;
			case XR_WIFI_DEAUTH:
				sta_event_notify(WIFI_DEAUTH);
				break;
			case XR_WIFI_DHCP_START:
				event_notify_to_sta_dev(WIFI_DHCP_START);
				break;
			case XR_WIFI_DHCP_TIMEOUT:
				sta_event_notify(WIFI_DHCP_TIMEOUT);
				break;
			case XR_WIFI_DHCP_SUCCESS:
				sta_event_notify(WIFI_DHCP_SUCCESS);
				break;
			case XR_WIFI_TERMINATING:
				sta_event_notify(WIFI_TERMINATING);
				break;
			case XR_WIFI_UNKNOWN:
				event_notify_to_sta_dev(WIFI_UNKNOWN);
				break;
		}
	} else if(payload->type == XR_WIFI_DEV_IND_RAW_DATA) {
		raw_data = (xr_wifi_raw_data_t *) payload->param;
		if(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->vend_cb) {
			((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->vend_cb(raw_data->data, raw_data->len);
		}
	} else {
		dev_event_notify(data);
	}
}

static wmg_status_t glue_sta_inf_init(sta_event_cb_t sta_event_cb, void *data)
{
	wmg_status_t status = WMG_STATUS_SUCCESS;

	sta_inf_object.sta_event_cb = sta_event_cb;

	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle = snfy_new();
	if(!(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle)) {
		WMG_ERROR("sta event snfy_new failed\n");
		return WMG_STATUS_FAIL;
	}

	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_handle = snfy_new();
	if(!(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_handle)) {
		snfy_free(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle);
		WMG_ERROR("dev event snfy_new failed\n");
		return WMG_STATUS_FAIL;
	}

	xr_wifi_init(xr_wifi_sta_cb, WIFI_STATION);

	return status;
}

static wmg_status_t glue_sta_inf_deinit(void *data)
{
	wmg_status_t status = WMG_STATUS_SUCCESS;

	xr_wifi_deinit();

	if(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle) {
		snfy_free(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->sta_event_snfy_handle);
		((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->event = XR_WIFI_DEV_ID_MAX;
	}
	if(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_handle) {
		snfy_free(((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->dev_event_snfy_handle);
	}

	return WMG_STATUS_SUCCESS;
}

static wmg_status_t glue_sta_inf_enable(void *data)
{
	char sta_auto_reconn_buf[10] = {0};
	char char_sec[24] = {0};
	char char_auth_alg[8] = {0};
	xr_wifi_sta_cn_t *con = NULL;

	if(xr_wifi_on(WIFI_STATION) == OS_NET_STATUS_OK) {
		get_config("xrlink_auto_reconnect", sta_auto_reconn_buf, "disable");
		if(!strcmp("enable", sta_auto_reconn_buf)) {
			con = malloc(sizeof(xr_wifi_sta_cn_t));
			memset(con, 0, sizeof(xr_wifi_sta_cn_t));
			get_config("xrlink_last_connected_ssid", con->ssid, NULL);
			con->ssid_len = strlen(con->ssid);
			if(con->ssid_len == 0) {
				WMG_DEBUG("No valid connection information was obtained and no connection was attempted\n");
				free(con);
				return WMG_STATUS_SUCCESS;
			}
			get_config("xrlink_last_connected_pwd", con->pwd, NULL);
			con->pwd_len = strlen(con->pwd);
			get_config("xrlink_last_connected_sec", char_sec, NULL);
			if(strcmp(char_sec, "NONE") == 0) {
				con->sec = XR_WIFI_SEC_NONE;
			} else if(strcmp(char_sec, "WEP") == 0) {
				con->sec = XR_WIFI_SEC_WEP;
				con->auth_alg = 1;
				get_config("xrlink_last_connected_wep_auth_alg", char_auth_alg, NULL);
				if(strcmp(char_auth_alg, "2") == 0) {
					con->auth_alg = 2;
				}
			} else if(strcmp(char_sec, "WPA_PSK") == 0) {
				con->sec = XR_WIFI_SEC_WPA_PSK;
			} else if(strcmp(char_sec, "WPA2_PSK") == 0) {
				con->sec = XR_WIFI_SEC_WPA2_PSK;
			} else if(strcmp(char_sec, "WPA2_PSK_SHA256") == 0) {
				con->sec = XR_WIFI_SEC_WPA2_PSK_SHA256;
			} else if(strcmp(char_sec, "WPA3_PSK") == 0) {
				con->sec = XR_WIFI_SEC_WPA3_PSK;
			} else {
				WMG_DEBUG("No valid connection information was obtained and no connection was attempted\n");
				free(con);
				return WMG_STATUS_SUCCESS;
			}

			WMG_DEBUG("Get xrlink auto reconnect enable, now try to connect last ssid: %s with pwd %s(%s)\n", con->ssid , con->pwd, char_sec);

			/* This is just an attempt to connect, there is no guarantee that it must be connected */
			xrlink_sta_send_cmd(sizeof(xr_wifi_sta_cn_t), XR_WIFI_HOST_STA_CONNECT, con, 0, 0);
			free(con);
		}
		return WMG_STATUS_SUCCESS;
	}

	return WMG_STATUS_FAIL;
}

static wmg_status_t glue_sta_inf_disable(void *data)
{
	if (xr_wifi_off(WIFI_STATION) == OS_NET_STATUS_OK) {
		return WMG_STATUS_SUCCESS;
	}

	return WMG_STATUS_FAIL;
}

#ifndef XRLINK_CONNECT_EVENT_TRY_MAX
#define XRLINK_CONNECT_EVENT_TRY_MAX 5
#endif
static wmg_status_t glue_sta_connect(wifi_sta_cn_para_t *cn)
{
	char sta_auto_reconn_buf[10] = {0};
	xr_wifi_sta_cn_t *con = malloc(sizeof(xr_wifi_sta_cn_t));
	wifi_sta_event_t sta_event;
	wmg_status_t connect_result = WMG_STATUS_FAIL;
	int connect_retry = 0;
	char ssid_buf[SSID_MAX_LEN + 1] = {0};
	uint8_t bssid[6] = {0};

	if ((cn->ssid == NULL) && (!memcmp(cn->bssid, bssid, 6))) {
		WMG_ERROR("ssid and bssid is invalid\n");
		event_notify_to_sta_dev(WIFI_DISCONNECTED);
		free(con);
		return WMG_STATUS_UNSUPPORTED;
	}
	memset(con, 0, sizeof(xr_wifi_sta_cn_t));
	if(cn->sec == WIFI_SEC_UNKNOWN) {
		if((cn->sec = scan_get_secure(2, (char **)&(cn->ssid), ssid_buf, cn->bssid)) == WIFI_SEC_UNKNOWN) {
			WMG_ERROR("xrlink os can't scan get secure\n");
			free(con);
			return WMG_STATUS_UNSUPPORTED;
		} else {
			WMG_DEBUG("xrlink os get secure %d\n", cn->sec);
		}
	}
	con->sec = cn->sec;
	if(con->sec == XR_WIFI_SEC_WEP) {
		con->auth_alg = 1;
	}
	con->fast_connect = cn->fast_connect;

	if(cn->ssid == NULL) {
		memcpy(con->bssid, cn->bssid, sizeof(cn->bssid));
	} else {
		memcpy(con->ssid, cn->ssid, strlen(cn->ssid));
		con->ssid_len = strlen(cn->ssid);
	}

	if(cn->password != NULL) {
		memcpy(con->pwd, cn->password, strlen(cn->password));
		con->pwd_len = strlen(cn->password);
	} else {
		con->pwd_len = 0;
	}

	if(cn->ssid != NULL) {
		WMG_INFO("[wifi_sta_connect] cn->ssid len=%d, %s\n", strlen(cn->ssid), cn->ssid);
	} else {
		WMG_INFO("[wifi_sta_connect] cn->bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
				cn->bssid[0], cn->bssid[1], cn->bssid[2], cn->bssid[3], cn->bssid[4], cn->bssid[5]);
	}

	WMG_INFO("[wifi_sta_connect] cn->password len=%d, %s\n",
			cn->password != NULL ? strlen(cn->password) : 0, cn->password != NULL ? cn->password : "NULL");

wep_auth_try:
	clean_dev_and_sta_event();
	xrlink_sta_send_cmd(sizeof(xr_wifi_sta_cn_t), XR_WIFI_HOST_STA_CONNECT, con, 0, 1);
	while(connect_retry < XRLINK_CONNECT_EVENT_TRY_MAX) {
		sta_event = get_sta_event();
		WMG_INFO("get sta event %d\n", sta_event);
		if (sta_event == WIFI_DHCP_SUCCESS) {
			connect_result = WMG_STATUS_SUCCESS;
			break;
		} else if ((sta_event == WIFI_ASSOC_REJECT) || (sta_event == WIFI_AUTH_REJECT)) {
			WMG_WARNG("Receive xrlink event assoc or auto reject\n");
			break;
		} else if (sta_event == WIFI_DHCP_TIMEOUT) {
			WMG_WARNG("Receive xrlink event dhcp time out\n");
			break;
		} else if (sta_event == WIFI_PASSWORD_INCORRECT) {
			WMG_WARNG("Receive xrlink password incorrect\n");
			break;
		//} else if (sta_event == WIFI_NETWORK_NOT_FOUND) {
		//	WMG_WARNG("Receive xrlink event not found\n");
		} else {
			connect_retry++;
		}
	};

	if((connect_result != WMG_STATUS_SUCCESS) && (con->sec == XR_WIFI_SEC_WEP) && (con->auth_alg == 1)) {
		WMG_WARNG("wep connect use auth_alg open fail, now try auth_alg shared\n");
		con->auth_alg = 2;
		connect_retry = 3;
		goto wep_auth_try;
	}

	if(connect_result == WMG_STATUS_SUCCESS){
		get_config("xrlink_auto_reconnect", sta_auto_reconn_buf, "disable");
		if(!strcmp("enable", sta_auto_reconn_buf)) {
			WMG_DEBUG("xrlink auto reconnect is enable, save connect info now\n");
			set_config("xrlink_last_connected_ssid", con->ssid);
			set_config("xrlink_last_connected_pwd", con->pwd);
			if(con->sec == XR_WIFI_SEC_WEP) {
				set_config("xrlink_last_connected_sec", "WEP");
				set_config("xrlink_last_connected_wep_auth_alg", con->auth_alg == 1 ? "1" : "2");
			} else if(con->sec == XR_WIFI_SEC_WPA_PSK) {
				set_config("xrlink_last_connected_sec", "WPA_PSK");
			} else if(con->sec == XR_WIFI_SEC_WPA2_PSK) {
				set_config("xrlink_last_connected_sec", "WPA2_PSK");
			} else if(con->sec == XR_WIFI_SEC_WPA2_PSK_SHA256) {
				set_config("xrlink_last_connected_sec", "WPA2_PSK_SHA256");
			} else if(con->sec == XR_WIFI_SEC_WPA3_PSK) {
				set_config("xrlink_last_connected_sec", "WPA3_PSK");
			} else {
				set_config("xrlink_last_connected_sec", "NONE");
			}
		}
		free(con);
		return WMG_STATUS_SUCCESS;
	} else {
		xrlink_sta_send_cmd(0, XR_WIFI_HOST_STA_DISCONNECT, NULL, 0, 1);
		clean_dev_and_sta_event();
		free(con);
		return WMG_STATUS_FAIL;
	}
}

static wmg_status_t glue_sta_disconnect(void)
{
	clean_dev_and_sta_event();
	if(!xrlink_sta_send_cmd(0, XR_WIFI_HOST_STA_DISCONNECT, NULL, 0, 1)) {
		if(get_sta_event() ==  WIFI_DISCONNECTED) {
			return WMG_STATUS_SUCCESS;
		}
	}
	WMG_WARNG("xrlink disconnect fail\n");
	return WMG_STATUS_FAIL;
}

static wmg_status_t glue_sta_auto_reconnect(wmg_bool_t enable)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;

	uint8_t cfg_mode = enable;

	status = xrlink_sta_send_cmd(sizeof(xr_sta_auto_reconnect_t),
			XR_WIFI_HOST_STA_AUTO_RECONNECT, &cfg_mode, 0, 0);

	return status;
}

static wmg_status_t glue_sta_get_info(wifi_sta_info_t *sta_info)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_sta_info_t *info;
	uint16_t dev_event_type = XR_WIFI_DEV_ID_MAX;

	status = xrlink_sta_send_cmd(0, XR_WIFI_HOST_STA_GET_INFO, NULL, 1, 0);

	WMG_DEBUG("dev event type:%d\n", dev_event_type);
	info = (xr_wifi_sta_info_t *)get_dev_event(&dev_event_type);
	if (dev_event_type == XR_WIFI_DEV_STA_INFO) {
		sta_info->id = info->id;
		sta_info->rssi = info->rssi;
		sta_info->freq = info->freq;
		sta_info->sec = info->sec;
		memcpy(sta_info->bssid, info->bssid, 6);
		memcpy(sta_info->ssid, info->ssid, strlen(info->ssid));
		memcpy(sta_info->mac_addr, info->mac_addr, 6);
		memcpy(sta_info->ip_addr, info->ip_addr, 4);
		memcpy(sta_info->gw_addr, info->gw, 4);
	} else {
		WMG_ERROR("get dev event is not wifi dev sta info\n");
		status = OS_NET_STATUS_FAILED;
	}

	return status;
}

static wmg_status_t glue_sta_list_networks(wifi_sta_list_t *sta_list)
{
	wmg_status_t status = WMG_STATUS_FAIL;
	char reconn_buf[10] = {0};
	char utf8_ssid_tmp_buf[SSID_MAX_LEN * 4] = {0};
	char utf8_ssid_buf[SSID_MAX_LEN * 4] = {0};

	get_config("xrlink_auto_reconnect", reconn_buf, "disable");

	if(!strcmp("enable", reconn_buf)) {
		sta_list->sys_list_num = 1;
		get_config("xrlink_last_connected_ssid", utf8_ssid_tmp_buf, NULL);

		if((strlen(utf8_ssid_tmp_buf) <= (SSID_MAX_LEN * 4)) &&
				(convert_utf8(utf8_ssid_tmp_buf, utf8_ssid_buf) <= SSID_MAX_LEN)){
			strcpy((sta_list->list_nod[0].ssid), utf8_ssid_buf);
		} else {
			strcpy((sta_list->list_nod[0].ssid), "NULL");
			WMG_WARNG("An illegal ssid was obtained and has been set to NULL\n");
		}

		strcpy((sta_list->list_nod[0].flags), "NULL");
		status = WMG_STATUS_SUCCESS;
	}
	return status;
}

static wmg_status_t glue_sta_remove_networks(char *ssid)
{
	char reconn_buf[10] = {0};
	char ssid_buf[SSID_MAX_LEN * 4] = {0};

	get_config("xrlink_auto_reconnect", reconn_buf, "disable");

	if(!strcmp("enable", reconn_buf)) {
		if(ssid != NULL) {
			WMG_DEBUG("remove network(%s) ...\n", ssid);
			get_config("xrlink_last_connected_ssid", ssid_buf, NULL);
			if(strcmp(ssid, ssid_buf)) {
				WMG_WARNG("Can't find network %d\n",ssid);
				return WMG_STATUS_SUCCESS;
			}
		}

		set_config("xrlink_last_connected_ssid", "");
		set_config("xrlink_last_connected_pwd", "");
		set_config("xrlink_last_connected_sec", "");
		set_config("xrlink_last_connected_wep_auth_alg", "");
		return WMG_STATUS_SUCCESS;
	}
}

static wmg_status_t glue_sta_get_scan_results(get_scan_results_para_t *sta_scan_results_para)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_scan_result_t *scan_result = NULL;
	xr_wifi_scan_info_t *scan_info = NULL;
	int i;
	uint16_t dev_event_type = XR_WIFI_DEV_ID_MAX;
	int try_cnt = 0;

	xr_wifi_scan_param_t *sta_scan_param = malloc(sizeof(xr_wifi_scan_param_t));
	sta_scan_param->enable = 0;
	if (sta_scan_results_para->ssid != NULL) {
		sta_scan_param->enable = 1;
		sta_scan_param->ssid_len = strlen(sta_scan_results_para->ssid);
		memcpy(sta_scan_param->ssid, sta_scan_results_para->ssid, sta_scan_param->ssid_len);
		WMG_DEBUG("scan target ssid:%s\n", sta_scan_param->ssid);
	}
	clean_dev_and_sta_event();
	status = xrlink_sta_send_cmd(sizeof(xr_wifi_scan_param_t), XR_WIFI_HOST_GET_SCAN_RES, sta_scan_param, 1, 0);
	while (try_cnt < 5) {
		scan_result = (xr_wifi_scan_result_t *)get_dev_event(&dev_event_type);
		WMG_DEBUG("dev event type:%d\n", dev_event_type);
		if (dev_event_type == XR_WIFI_DEV_SCAN_RES) {
			scan_info = (xr_wifi_scan_info_t *)scan_result->ap_info;
			*(sta_scan_results_para->bss_num) = scan_result->num;
			for (i = 0; i < *(sta_scan_results_para->bss_num); i++) {
				sta_scan_results_para->scan_results[i].freq = (uint32_t)scan_info[i].freq;
				sta_scan_results_para->scan_results[i].rssi = scan_info[i].rssi;
				sta_scan_results_para->scan_results[i].key_mgmt = scan_info[i].key_mgmt;

				memcpy(sta_scan_results_para->scan_results[i].bssid, scan_info[i].bssid, 6);
				if(scan_info[i].ssid_len <= SSID_MAX_LEN) {
					memcpy(sta_scan_results_para->scan_results[i].ssid, scan_info[i].ssid, scan_info[i].ssid_len);
					sta_scan_results_para->scan_results[i].ssid[scan_info[i].ssid_len] = '\0';
				} else {
					WMG_WARNG("xrlink get ssid too long(%d)\n", scan_info[i].ssid_len);
				}
			}
			break;
		} else {
			WMG_ERROR("get dev event is not wifi dev scan results\n");
			status = OS_NET_STATUS_FAILED;
			try_cnt++;
		}
	}
	free(sta_scan_param);
	return status;
}

#define scan_num 80
/* This function is used to obtain the encryption method by scanning
 * 1.You need to enter ssid or bssid to get the encryption method
 * 2.If you only input bssid but not ssid, this function will put the scanned ssid into ssid_buf,
 *   because wpa3 needs ssid. */
static wifi_secure_t scan_get_secure(int scan_time, char **ssid, char *ssid_buf, uint8_t *bssid)
{
	int get_num = 0, i = 0, get_scan_match_results = 0;
	WMG_DEBUG("linux os scan get secure\n");
	wifi_scan_result_t scan_res[scan_num];
	memset(&scan_res, 0, sizeof(wifi_scan_result_t) * scan_num);
	get_scan_results_para_t get_scan_results;
	memset(&get_scan_results, 0, sizeof(get_scan_results_para_t));
	get_scan_results.scan_results = scan_res;
	get_scan_results.ssid = NULL;
	get_scan_results.bss_num = &get_num;
	get_scan_results.arr_size = scan_num;

	if(glue_sta_get_scan_results(&get_scan_results) == WMG_STATUS_SUCCESS) {
		for (; i < get_num; i++) {
			if(!memcmp(bssid, scan_res[i].bssid, 6)) {
				get_scan_match_results = 1;
				if(*ssid == NULL) {
					if(strlen(scan_res[i].ssid) > SSID_MAX_LEN) {
						WMG_ERROR("scan get ssid is too long(%d)\n", strlen(scan_res[i].ssid));
						return WIFI_SEC_UNKNOWN;
					}
					strcpy(ssid_buf, scan_res[i].ssid);
					*ssid = ssid_buf;
					WMG_DEBUG("Input ssid is null, use bssid to update ssid(%s)\n", *ssid);
				}
			} else if(*ssid != NULL) {
				if(!strcmp(scan_res[i].ssid, *ssid)) {
					get_scan_match_results = 1;
				}
			}
			if(get_scan_match_results) {
				if((scan_res[i].key_mgmt) & WIFI_SEC_WPA3_PSK) {
					return WIFI_SEC_WPA3_PSK;
				}
				if((scan_res[i].key_mgmt) & WIFI_SEC_WPA2_PSK_SHA256) {
					return WIFI_SEC_WPA2_PSK_SHA256;
				}
				if((scan_res[i].key_mgmt) & WIFI_SEC_WPA2_PSK) {
					return WIFI_SEC_WPA2_PSK;
				}
				if((scan_res[i].key_mgmt) & WIFI_SEC_WPA_PSK) {
					return WIFI_SEC_WPA_PSK;
				}
				if((scan_res[i].key_mgmt) & WIFI_SEC_WEP) {
					return WIFI_SEC_WEP;
				}
				if(scan_res[i].key_mgmt == WIFI_SEC_NONE) {
					return WIFI_SEC_NONE;
				}
					break;
				}
		}
	}

	WMG_ERROR("linux os scan get secure failed\n");
	return WIFI_SEC_UNKNOWN;
}

static int glue_sta_vendor_send_data(uint8_t *data, uint32_t len)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_raw_data_t *raw = (xr_wifi_raw_data_t *)malloc(sizeof(xr_wifi_raw_data_t) + len);
	if(NULL == raw) {
		status = WMG_STATUS_NOMEM;
		return status;
	}

	raw->len = len;

	memcpy(raw->data, data, len);

	status = xrlink_sta_send_cmd(sizeof(xr_wifi_raw_data_t) + len, XR_WIFI_HOST_SEND_RAW, raw, 0, 0);

	free(raw);
	return status;
}

static int glue_sta_vendor_register_rx_cb(wifi_vend_cb_t vend_cb)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	((xrlink_sta_private_data_t *)sta_inf_object.sta_private_data)->vend_cb = vend_cb;
	if(vend_cb) {
		status = WMG_STATUS_SUCCESS;
	} else {
		status = WMG_STATUS_INVALID;
	}
	return status;
}

static wmg_status_t glue_platform_extension(int cmd, void* cmd_para,int *erro_code)
{
	switch (cmd) {
		case STA_CMD_CONNECT:
			return glue_sta_connect((wifi_sta_cn_para_t *)cmd_para);
		case STA_CMD_DISCONNECT:
			return glue_sta_disconnect();
		case STA_CMD_SET_AUTO_RECONN:
			{
				wmg_bool_t *enable = (wmg_bool_t *)cmd_para;
				return glue_sta_auto_reconnect(*enable);
			}
		case STA_CMD_GET_INFO:
			return glue_sta_get_info((wifi_sta_info_t *)cmd_para);
		case STA_CMD_LIST_NETWORKS:
			return glue_sta_list_networks((wifi_sta_list_t *)cmd_para);
		case STA_CMD_REMOVE_NETWORKS:
			return glue_sta_remove_networks((char *)cmd_para);
		case STA_CMD_GET_SCAN_RESULTS:
			{
				get_scan_results_para_t *sta_scan_results_para = (get_scan_results_para_t *)cmd_para;
				return glue_sta_get_scan_results(sta_scan_results_para);
			}
		case STA_CMD_VENDOR_SEND_DATA:
			{
				vendor_send_data_para_t *vendor_send_data_para = (vendor_send_data_para_t *)cmd_para;
				return glue_sta_vendor_send_data(vendor_send_data_para->data, vendor_send_data_para->len);
			}
		case STA_CMD_VENDOR_REGISTER_RX_CB:
				return glue_sta_vendor_register_rx_cb((wifi_vend_cb_t)cmd_para);
		default:
		return WMG_STATUS_FAIL;
	}
	return WMG_STATUS_FAIL;
}

static xrlink_sta_private_data_t xrlink_sta_private_data = {
	.sta_event_snfy_handle = NULL,
	.sta_event_snfy_await_valid = false,
	.event = WIFI_UNKNOWN,
	.dev_event_snfy_handle = NULL,
	.dev_event_snfy_await_valid = false,
	.vend_cb = NULL,
};

static wmg_sta_inf_object_t sta_inf_object = {
	.sta_init_flag = WMG_FALSE,
	.sta_auto_reconn = WMG_FALSE,
	.sta_event_cb = NULL,
	.sta_private_data = &xrlink_sta_private_data,

	.sta_inf_init = glue_sta_inf_init,
	.sta_inf_deinit = glue_sta_inf_deinit,
	.sta_inf_enable = glue_sta_inf_enable,
	.sta_inf_disable = glue_sta_inf_disable,
	.sta_platform_extension = glue_platform_extension,
};

wmg_sta_inf_object_t* sta_xrlink_inf_object_register(void)
{
	return &sta_inf_object;
}
