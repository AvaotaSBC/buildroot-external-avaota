/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <os_net_utils.h>
#include <os_net_thread.h>

os_net_status_t os_net_thread_create(os_net_thread_t *thread_handle, const char *name,
                                     os_net_thread_func_t func, void *arg, os_net_thread_prio_t pri,
                                     uint32_t task_size)
{
    if (pthread_create(thread_handle, NULL, func, arg) != 0) {
        return OS_NET_STATUS_NOMEM;
    }
#ifdef _GNU_SOURCE
    if (pthread_setname_np(*thread_handle, name, strlen(name)) != 0) {
        OS_NET_ERROR("set name failed\n");
        return OS_NET_STATUS_NOMEM;
    }
#endif
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_thread_delete(os_net_thread_t *thread_handle)
{
    if (pthread_cancel(*thread_handle) != 0) {
        return OS_NET_STATUS_FAILED;
    }
    if (pthread_join(*thread_handle, NULL) != 0) {
        return OS_NET_STATUS_FAILED;
    }
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_thread_get_pid(os_net_thread_pid_t *pid)
{
    *pid = pthread_self();
    return OS_NET_STATUS_OK;
}
