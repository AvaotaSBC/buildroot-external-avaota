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
#include <os_net_utils.h>
#include <os_net_mutex.h>

os_net_status_t os_net_mutex_create(os_net_mutex_t *mutex)
{
    if (pthread_mutex_init(mutex, NULL) != 0) {
        return OS_NET_STATUS_NOMEM;
    }
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_mutex_delete(os_net_mutex_t *mutex)
{
    if (pthread_mutex_destroy(mutex) != 0) {
        return OS_NET_STATUS_FAILED;
    }
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_mutex_lock(os_net_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_mutex_unlock(os_net_mutex_t *mutex)
{
    pthread_mutex_unlock(mutex);
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_mutex_recursive_create(os_net_recursive_mutex_t *mutex)
{
    if (pthread_mutexattr_init(&mutex->attr) != 0) {
        return OS_NET_STATUS_NOMEM;
    }
    if (pthread_mutexattr_settype(&mutex->attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
        return OS_NET_STATUS_NOMEM;
    }
    if (pthread_mutex_init(&mutex->mutex, NULL) != 0) {
        return OS_NET_STATUS_NOMEM;
    }
    pthread_mutexattr_destroy(&mutex->attr);
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_mutex_recursive_delete(os_net_recursive_mutex_t *mutex)
{
    if (pthread_mutex_destroy(&mutex->mutex) != 0) {
        return OS_NET_STATUS_FAILED;
    }
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_mutex_recursive_lock(os_net_recursive_mutex_t *mutex)
{
    pthread_mutex_lock(&mutex->mutex);
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_mutex_recursive_unlock(os_net_recursive_mutex_t *mutex)
{
    pthread_mutex_unlock(&mutex->mutex);
    return OS_NET_STATUS_OK;
}
