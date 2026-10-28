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


#ifndef __WIFIMG_GLUE_H__
#define __WIFIMG_GLUE_H__

typedef enum {
	ACWIFI_REG_CB = 0,
	ACWIFI_ON,
	ACWIFI_OFF,
	ACWIFI_STA_CONNECT,
	ACWIFI_STA_DISCONNECT,
	ACWIFI_AUTO_RECONNECT,
	ACWIFI_STA_GET_INFO,
	ACWIFI_LIST_NETWORKS,
	ACWIFI_REMOVE_NETWORKS,
	ACWIFI_AP_ENABLE,
	ACWIFI_AP_DISABLE,
	ACWIFI_AP_GET_CONFIG,
	ACWIFI_MON_ENABLE,
	ACWIFI_MON_SET_CH,
	ACWIFI_MON_DISABLE,
	ACWIFI_SET_SCAN_PARAM,
	ACWIFI_GET_SCAN_RESULTS,
	ACWIFI_SET_MAC,
	ACWIFI_GET_MAC,
	ACWIFI_SEND_80211_FRAME,
	ACWIFI_SET_COUNTRY_CODE,
	ACWIFI_GET_COUNTRY_CODE,
	ACWIFI_SET_PS_MODE,
	ACWIFI_VENDOR_SEND_DATA,
	ACWIFI_VENDOR_REG_CB,
} wifimg_call_t;

#endif
