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

#ifndef _LINUX_AP_H
#define _LINUX_AP_H

#if __cplusplus
extern "C" {
#endif

#include <wmg_p2p.h>

typedef struct {
	wmg_bool_t auto_connect;
	char p2p_mac_addr[18];
	char p2p_dev_name[P2P_DEV_NAME_MAX_LEN];
} auto_connect_t;

wmg_p2p_inf_object_t* p2p_linux_inf_object_register(void);

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
   i(__extension__                                                              \
     ({ long int __result;                                                     \
        do __result = (long int) (expression);                                 \
        while (__result == -1L && errno == EINTR);                             \
        __result; }))
#endif


#if __cplusplus
};  // extern "C"
#endif

#endif  // _LINUX_AP_H
