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
#include <xrlink_get_config.h>
#include <os_net_thread.h>
#include <api_action.h>
#include <wifimg_glue.h>
#include <wmg_ap.h>
#include <wifi_log.h>

typedef struct {
	snfy_handle_t *ap_event_snfy_handle;
	wmg_bool_t ap_event_snfy_await_valid;
	wifi_ap_event_t event;
	snfy_handle_t *dev_event_snfy_handle;
	wmg_bool_t dev_event_snfy_await_valid;
	wifi_vend_cb_t vend_cb;
	uint8_t ap_ip_addr[4];
	uint8_t ap_netmask[4];
	uint8_t ap_gw[4];
} xrlink_ap_private_data_t;

static wmg_ap_inf_object_t ap_inf_object;
static xrlink_ap_private_data_t xrlink_ap_private_data;

static os_net_status_t xrlink_ap_send_cmd(uint32_t len, cfg_host_op_t type, void *param, int dev_event_flag, int ap_event_flag)
{
	if(dev_event_flag) {
		((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_await_valid = true;
	}
	if(ap_event_flag) {
		((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_await_valid = true;
	}
	return xrlink_send_cmd(len, type, param);
}

static void event_notify_to_ap_dev(wifi_ap_event_t event)
{
	if (ap_inf_object.ap_event_cb) {
		ap_inf_object.ap_event_cb(event);
	}
}

static void ap_event_notify(wifi_ap_event_t event)
{
	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->event = event;
	if(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_await_valid) {
		WMG_DEBUG("** ap: ap event effective monitoring time period, generating snfy **\n");
		snfy_ready(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_handle,
			&(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->event));
	}
	event_notify_to_ap_dev(event);
}

static wifi_ap_event_t get_ap_event(void)
{
	wifi_ap_event_t *ap_event = NULL;
	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_await_valid = true;
	ap_event = snfy_await(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_handle,
				WAIT_DEV_NORMOL_TO_MS);
	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_await_valid = false;
	if(ap_event != NULL) {
		return *ap_event;
	} else {
		return WIFI_UNKNOWN;
	}
}

static void dev_event_notify(char *data)
{
	if(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_await_valid) {
		WMG_DEBUG("** ap: dev event effective monitoring time period, generating snfy **\n");
		snfy_ready(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_handle, data);
	}
}

static void* get_dev_event(uint16_t *type)
{
	char *data = NULL;
	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_await_valid = true;
	data = snfy_await(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_handle,
		WAIT_DEV_NORMOL_TO_MS);
	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_await_valid = false;
	if(data != NULL) {
		xr_cfg_payload_t *payload = (xr_cfg_payload_t*) data;
		*type = payload->type;
		return payload->param;
	}
	return data;
}

static void xr_wifi_ap_host_network_ip_clean()
{
	char cmd[64] = {0};
	memset(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_ip_addr,
	'\0', sizeof(uint8_t) * 4);
	memset(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_netmask,
	'\0', sizeof(uint8_t) * 4);
	memset(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_gw,
	'\0', sizeof(uint8_t) * 4);

	sprintf(cmd, "ifconfig %s 0.0.0.0", AP_NETWORK_CARD);
	system(cmd);
}

static void xr_wifi_ap_host_network_update(xr_wifi_ap_network_card_info_t *ap_info)
{
	char config_ip_cmd[32 * 3];
	char ip_addr[32];
	char netmask[32];
	char gw[32];

	sprintf(ip_addr, "%d.%d.%d.%d",ap_info->ip_addr[0], ap_info->ip_addr[1],
			ap_info->ip_addr[2], ap_info->ip_addr[3]);
	memcpy(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_ip_addr,
	ap_info->ip_addr, sizeof(uint8_t) * 4);

	sprintf(netmask, "%d.%d.%d.%d",ap_info->netmask[0], ap_info->netmask[1],
			ap_info->netmask[2], ap_info->netmask[3]);
	memcpy(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_netmask,
	ap_info->netmask, sizeof(uint8_t) * 4);

	sprintf(gw, "%d.%d.%d.%d",ap_info->gw[0], ap_info->gw[1],
			ap_info->gw[2], ap_info->gw[3]);
	memcpy(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_gw,
	ap_info->gw, sizeof(uint8_t) * 4);

	WMG_DEBUG("get ip address:%s\n", ip_addr);
	WMG_DEBUG("get netmask:%s\n", netmask);
	WMG_DEBUG("get gw:%s\n", gw);

	sprintf(config_ip_cmd,"ifconfig %s %s netmask %s", AP_NETWORK_CARD, ip_addr, netmask);
	system(config_ip_cmd);

	sprintf(config_ip_cmd,"route add default gw %s",gw);
	system(config_ip_cmd);
}

static void xr_wifi_ap_cb(char *data,uint32_t len)
{
	xr_cfg_payload_t *payload = (xr_cfg_payload_t*) data;
	xr_wifi_ap_state_t ap_state;
	xr_wifi_raw_data_t *raw_data;

	WMG_DEBUG("ap: payload type:%d\n",payload->type);
	if(payload->type == XR_WIFI_DEV_AP_STATE) {
		ap_state = *(xr_wifi_ap_state_t *)payload->param;
		WMG_DEBUG("wifi ap state:%d\n", ap_state);
		switch(ap_state) {
			case XR_WIFI_AP_ON:
				xr_wifi_ap_host_network_update((xr_wifi_ap_network_card_info_t *)payload->param);
				ap_event_notify(WIFI_AP_ENABLED);
				break;
			case XR_WIFI_AP_OFF:
				xr_wifi_ap_host_network_ip_clean();
				ap_event_notify(WIFI_AP_DISABLED);
				break;
			case XR_WIFI_AP_STA_DISCONNECTED:
				ap_event_notify(WIFI_AP_STA_DISCONNECTED);
				break;
			case XR_WIFI_AP_STA_CONNECTED:
				ap_event_notify(WIFI_AP_STA_CONNECTED);
				break;
			case XR_WIFI_AP_UNKNOWN:
				ap_event_notify(WIFI_AP_UNKNOWN);
				break;
		}
	} else if(payload->type == XR_WIFI_DEV_IND_RAW_DATA) {
		raw_data = (xr_wifi_raw_data_t *) payload->param;
		if(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->vend_cb) {
			((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->vend_cb(raw_data->data, raw_data->len);
		}
	} else {
		dev_event_notify(data);
	}
}

static wmg_status_t glue_ap_inf_init(ap_event_cb_t ap_event_cb)
{
	wmg_status_t status = WMG_STATUS_SUCCESS;

	ap_inf_object.ap_event_cb = ap_event_cb;

	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_handle = snfy_new();
	if(!(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_handle)) {
		WMG_ERROR("dev event snfy_new failed\n");
		return WMG_STATUS_FAIL;
	}

	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_handle = snfy_new();
	if(!(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_handle)) {
		snfy_free(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_handle);
		WMG_ERROR("dev status snfy_new failed\n");
		return WMG_STATUS_FAIL;
	}

	xr_wifi_init(xr_wifi_ap_cb, WIFI_AP);

	return status;
}

static wmg_status_t glue_ap_inf_deinit(void *data)
{
	wmg_status_t status = WMG_STATUS_SUCCESS;

	xr_wifi_deinit();

	if(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_handle) {
		snfy_free(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->dev_event_snfy_handle);
	}
	if(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_handle) {
		snfy_free(((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_event_snfy_handle);
	}

	return WMG_STATUS_SUCCESS;
}

static wmg_status_t glue_ap_inf_enable(void *data)
{
	if (xr_wifi_on(WIFI_AP) == OS_NET_STATUS_OK) {
		return WMG_STATUS_SUCCESS;
	}

	return WMG_STATUS_FAIL;
}

static wmg_status_t glue_ap_inf_disable(void *data)
{
	if (xr_wifi_off(WIFI_AP) == OS_NET_STATUS_OK) {
		return WMG_STATUS_SUCCESS;
	}

	return WMG_STATUS_FAIL;
}

static wmg_status_t glue_ap_enable(wifi_ap_config_t *ap_config)
{
	wmg_status_t status = WMG_STATUS_FAIL;
	uint8_t ip_addr[4] = {0};
	char max_num_sta_buf[10] = {0};
	uint8_t max_num_sta = MAX_NUM_STA_DEFAULT;

	xr_wifi_ap_config_t *cfg = (xr_wifi_ap_config_t *)malloc(sizeof(xr_wifi_ap_config_t));
	memset(cfg, 0, sizeof(xr_wifi_ap_config_t));

	cfg->channel = ap_config->channel;
	cfg->sec = ap_config->sec;
	if(!memcmp(ap_config->ip_addr, ip_addr, 4)) {
		cfg->ip_addr[0] = 192;
		cfg->ip_addr[1] = 168;
		cfg->ip_addr[2] = 31;
		cfg->ip_addr[3] = 1;
	} else {
		memcpy(cfg->ip_addr, ap_config->ip_addr, 4);
	}

	memcpy(cfg->gw, cfg->ip_addr, 4);
	cfg->netmask[0] = 255;
	cfg->netmask[1] = 255;
	cfg->netmask[2] = 255;
	cfg->netmask[3] = 0;

	memcpy(cfg->ssid, ap_config->ssid, strlen(ap_config->ssid));
	if (cfg->sec) {
		memcpy(cfg->pwd, ap_config->psk, strlen(ap_config->psk));
	}

	if ((ap_config->max_num_sta >= 1) && (ap_config->max_num_sta <= MAX_NUM_STA_DEFAULT)) {
		max_num_sta = ap_config->max_num_sta;
	} else {
		if (!get_config("max_num_sta", max_num_sta_buf, NULL)) {
			max_num_sta = atoi(max_num_sta_buf);
			if ((max_num_sta == 0) || (max_num_sta > MAX_NUM_STA_DEFAULT)) {
				WMG_WARNG("The configuration(%s) is illegal(%d). Use the default configuration: %d\n",
						max_num_sta_buf, max_num_sta, MAX_NUM_STA_DEFAULT);
				max_num_sta = MAX_NUM_STA_DEFAULT;
			}
		}
	}

	cfg->max_num_sta = max_num_sta;

	if(!xrlink_ap_send_cmd(sizeof(xr_wifi_ap_config_t), XR_WIFI_HOST_AP_ENABLE, cfg, 0, 1)) {
		wifi_ap_event_t ap_event = get_ap_event();
		if(ap_event == WIFI_AP_ENABLED) {
			status = WMG_STATUS_SUCCESS;
			WMG_DEBUG("xrlink ap enable success\n");
		} else {
			status = WMG_STATUS_FAIL;
		}
	} else {
		WMG_ERROR("xrlink: ap send enable cmd fail\n");
	}

	free(cfg);

	return status;
}

static wmg_status_t glue_ap_disable()
{
	wmg_status_t status = WMG_STATUS_FAIL;
	bool sta_disconnet_event = false;

	if(!xrlink_ap_send_cmd(0, XR_WIFI_HOST_AP_DISABLE, NULL, 0, 1)) {
		do {
			sta_disconnet_event = false;
			wifi_ap_event_t ap_event = get_ap_event();
			if(ap_event == WIFI_AP_DISABLED) {
				status = WMG_STATUS_SUCCESS;
				WMG_DEBUG("xrlink ap disable success\n");
			} else {
				if(ap_event == WIFI_AP_STA_DISCONNECTED) {
					WMG_DEBUG("xrlink ap disable get sta disconnet event\n");
					sta_disconnet_event = true;
				} else {
					status = WMG_STATUS_FAIL;
				}
			}
		} while (sta_disconnet_event);
	} else {
		WMG_ERROR("xrlink: ap send disable cmd fail\n");
	}

	return status;
}

static wmg_status_t glue_ap_get_config(wifi_ap_config_t *ap_config)
{
	uint16_t dev_event_type = XR_WIFI_DEV_ID_MAX;
	int i, max_num;
	char mac[32] = {0};
	xr_wifi_ap_sta_info_t *ap_sta_info;

	xrlink_ap_send_cmd(0, XR_WIFI_HOST_AP_GET_INF, NULL, 1, 0);
	xr_wifi_ap_info_t *info = get_dev_event(&dev_event_type);
	if(dev_event_type == XR_WIFI_DEV_AP_INF) {
		memcpy(ap_config->ssid, info->ssid, strlen(info->ssid));
		memcpy(ap_config->psk, info->pwd, strlen(info->pwd));
		ap_config->sec = info->sec;
		ap_config->channel = info->channel;
		memcpy(ap_config->ip_addr, ((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_ip_addr,
		sizeof(uint8_t) * 4);
		memcpy(ap_config->gw_addr, ((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->ap_gw,
		sizeof(uint8_t) * 4);
		ap_config->sta_num = info->sta_num;
		WMG_DEBUG("get sta number:%d, buff max number:%d\n", info->sta_num, ap_config->sta_dev_list_num);
		max_num = info->sta_num <= ap_config->sta_dev_list_num ? info->sta_num : ap_config->sta_dev_list_num;
		ap_sta_info = (xr_wifi_ap_sta_info_t *) info->dev_list;
		for(i = 0; i < info->sta_num; i++) {
			memcpy(ap_config->dev_list[i].sta_mac, ap_sta_info[i].addr, sizeof(uint8_t) * 6);
			sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X",
				ap_sta_info[i].addr[0],ap_sta_info[i].addr[1],ap_sta_info[i].addr[2],
				ap_sta_info[i].addr[3],ap_sta_info[i].addr[4],ap_sta_info[i].addr[5]);
			WMG_DEBUG("mac addres:%s\n", mac);
		}
		return WMG_STATUS_SUCCESS;
	};
	return WMG_STATUS_FAIL;
}

static wmg_status_t glue_ap_get_scan_results(get_scan_results_para_t *sta_scan_results_para)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_scan_result_t *scan_result = NULL;
	xr_wifi_scan_info_t *scan_info = NULL;
	int i;
	char bssid[18 + 1];
	uint16_t dev_event_type = XR_WIFI_DEV_ID_MAX;

	status = xrlink_ap_send_cmd(0, XR_WIFI_HOST_GET_SCAN_RES, NULL, 1, 0);
	scan_result = (xr_wifi_scan_result_t *)get_dev_event(&dev_event_type);
	if (dev_event_type == XR_WIFI_DEV_SCAN_RES) {
		scan_info = (xr_wifi_scan_info_t *)scan_result->ap_info;
		*(sta_scan_results_para->bss_num) = scan_result->num;
		for (i = 0; i < *(sta_scan_results_para->bss_num); i++) {
			sta_scan_results_para->scan_results[i].freq = scan_info[i].freq;
			sta_scan_results_para->scan_results[i].rssi = scan_info[i].rssi;
			sta_scan_results_para->scan_results[i].key_mgmt = scan_info[i].key_mgmt;

			sprintf(bssid,"%02X:%02X:%02X:%02X:%02X:%02X%c",
				scan_info[i].bssid[0], scan_info[i].bssid[1], scan_info[i].bssid[2],
				scan_info[i].bssid[3], scan_info[i].bssid[4], scan_info[i].bssid[5], '\0');

			memcpy(sta_scan_results_para->scan_results[i].bssid, bssid, 18);
			memcpy(sta_scan_results_para->scan_results[i].ssid, scan_info[i].ssid, scan_info[i].ssid_len);
		}
	} else {
		WMG_ERROR("get dev event is not wifi dev scan results\n");
		status = OS_NET_STATUS_FAILED;
	}

	return status;
}

static wmg_status_t glue_ap_vendor_send_data(uint8_t *data, uint32_t len)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	xr_wifi_raw_data_t *raw = (xr_wifi_raw_data_t *)malloc(sizeof(xr_wifi_raw_data_t) + len);
	if(NULL == raw) {
		status = WMG_STATUS_NOMEM;
		return status;
	}

	raw->len = len;

	memcpy(raw->data, data, len);

	status = xrlink_ap_send_cmd(sizeof(xr_wifi_raw_data_t) + len, XR_WIFI_HOST_SEND_RAW, raw, 0, 0);

	free(raw);
	return status;
}

static wmg_status_t glue_ap_vendor_register_rx_cb(wifi_vend_cb_t vend_cb)
{
	wmg_status_t status = WMG_STATUS_UNHANDLED;
	((xrlink_ap_private_data_t *)ap_inf_object.ap_private_data)->vend_cb = vend_cb;
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
		case AP_CMD_ENABLE:
			return glue_ap_enable((wifi_ap_config_t *)cmd_para);
		case AP_CMD_DISABLE:
			return glue_ap_disable();
		case AP_CMD_GET_CONFIG:
			return glue_ap_get_config((wifi_ap_config_t *)cmd_para);
		case AP_CMD_SET_SCAN_PARAM:
			return WMG_STATUS_FAIL;
		case AP_CMD_GET_SCAN_RESULTS:
			{
				get_scan_results_para_t *ap_scan_results_para = (get_scan_results_para_t *)cmd_para;
				return glue_ap_get_scan_results(ap_scan_results_para);
			}
		case AP_CMD_VENDOR_SEND_DATA:
			{
				vendor_send_data_para_t *vendor_send_data_para = (vendor_send_data_para_t *)cmd_para;
				return glue_ap_vendor_send_data(vendor_send_data_para->data, vendor_send_data_para->len);
			}
		case AP_CMD_VENDOR_REGISTER_RX_CB:
				return glue_ap_vendor_register_rx_cb((wifi_vend_cb_t)cmd_para);
		default:
		return WMG_STATUS_FAIL;
	}
	return WMG_STATUS_FAIL;
}

static xrlink_ap_private_data_t xrlink_ap_private_data = {
	.ap_event_snfy_handle = NULL,
	.ap_event_snfy_await_valid = false,
	.dev_event_snfy_handle = NULL,
	.dev_event_snfy_await_valid = false,
	.vend_cb = NULL,
	.ap_ip_addr = {0},
	.ap_netmask = {0},
	.ap_gw = {0},
};

static wmg_ap_inf_object_t ap_inf_object = {
	.ap_init_flag = WMG_FALSE,
	.ap_event_cb = NULL,
	.ap_private_data = &xrlink_ap_private_data,

	.ap_inf_init = glue_ap_inf_init,
	.ap_inf_deinit = glue_ap_inf_deinit,
	.ap_inf_enable = glue_ap_inf_enable,
	.ap_inf_disable = glue_ap_inf_disable,
	.ap_platform_extension = glue_platform_extension,
};

wmg_ap_inf_object_t* ap_xrlink_inf_object_register(void)
{
	return &ap_inf_object;
}
