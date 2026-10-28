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
#include <time.h>
#include <os_net_utils.h>
#include <os_net_sem.h>

os_net_status_t os_net_sem_create(os_net_sem_t *sem, uint32_t init_count, uint32_t max_count)
{
    if (sem_init(sem, 0, init_count) != 0)
        return OS_NET_STATUS_NOMEM;
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_sem_delete(os_net_sem_t *sem)
{
    if (sem_destroy(sem) != 0) {
        return OS_NET_STATUS_FAILED;
    }
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_sem_wait(os_net_sem_t *sem, os_net_time_t wait_ms)
{
	struct timespec ts;
	if(wait_ms == 0 || wait_ms == OS_NET_WAIT_FOREVER) {
		if(sem_wait(sem) != 0)
			return OS_NET_STATUS_FAILED;
		return OS_NET_STATUS_OK;
	}

	if(clock_gettime(CLOCK_REALTIME, &ts) < 0) {
		return OS_NET_STATUS_FAILED;
	}

	ts.tv_sec += wait_ms / 1000;
	ts.tv_nsec += wait_ms % 1000 * 1000;

	if (sem_timedwait(sem, (const struct timespec *) &ts) != 0)
		return OS_NET_STATUS_FAILED;

    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_sem_release(os_net_sem_t *sem)
{
    if (sem_post(sem) != 0) {
        return OS_NET_STATUS_FAILED;
    }
    return OS_NET_STATUS_OK;
}
