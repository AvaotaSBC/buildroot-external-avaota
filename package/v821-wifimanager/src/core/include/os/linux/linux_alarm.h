/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 */

#ifndef __LINUX_ALARM_H__
#define __LINUX_ALARM_H__

#include "wmg_common.h"

#if __cplusplus
extern "C" {
#endif

/* MAX_MEMBER must be set to a multiple of 8 and cannot exceed 32 */
#define MAX_MEMBER 8
typedef void (*linux_alarm_cb_t)(void *alarm_cb_para);

#if __cplusplus
}
#endif

#endif /* __LINUX_ALARM_H__ */
