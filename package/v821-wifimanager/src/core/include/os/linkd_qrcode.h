/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 */

#ifndef __LINKD_QRCODE_H__
#define __LINKD_QRCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(OS_NET_LINUX_OS) || defined(OS_NET_XRLINK_OS)

#include <VIDEO_FRAME_INFO_S.h>
#include "media/mpi_sys.h"
#include "log/log_print.h"
#include "mm_comm_vi.h"
#include "mpi_vi.h"
#include "mpi_isp.h"
#include <SystemBase.h>
#include <mpi_videoformat_conversion.h>
#include <utils/plat_log.h>
#include <endian.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>

#include <confparser.h>

#include <plat_type.h>
#include <tsemaphore.h>

typedef struct {
	VIDEO_FRAME_INFO_S pstFrameInfo;
	int mThExitFlag;
	VI_DEV Dev;
	VI_CHN Chn;
	int s32MilliSec;
	void *mpContext;	//SampleIdentifyQRCodeContext*
	unsigned int mCurFrameId;
} IdentifyQRCode_Cap_S;

typedef struct {
	int ispdev;
	int DevNum;
	int SrcWidth;
	int SrcHeight;
	int FrameRate;
	unsigned int SrcFormat;
	enum v4l2_colorspace mColorSpace;
	int mViDropFrmCnt;
	int mEncppEnable;
	unsigned int mTestDuration;
} SampleIdentifyQRCodeConfig;

typedef struct {
	SampleIdentifyQRCodeConfig mConfigPara;

	IdentifyQRCode_Cap_S privCap;
} SampleIdentifyQRCodeContext;

#define DEFAULT_SAMPLE_VIRVI_CONF_PATH  "/etc/wifi/qrcode.conf"

#define CFG_Dev_Num	            "dev_num"
#define CFG_Src_Width           "src_width"
#define CFG_Src_Height          "src_height"
#define CFG_Frame_Rate          "frame_rate"
#define CFG_Src_Format          "src_format"
#define CFG_COLOR_SPACE         "color_space"
#define CFG_ViDropFrmCnt        "drop_frm_num"
#define CFG_EncppEnable         "encpp_enable"

#endif /* (OS_NET_LINUX_OS || OS_NET_XRLINK_OS) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LINKD_QRCODE_H__ */
