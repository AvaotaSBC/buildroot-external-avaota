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
#include <os_net_sync_notify.h>
#include <string.h>

os_net_status_t snfy_free(snfy_handle_t *handle);

snfy_handle_t *snfy_new(void)
{
    snfy_handle_t *ret = malloc(sizeof(snfy_handle_t));
    if (!ret) {
        goto error;
	} else {
		memset(ret, '\0', sizeof(snfy_handle_t));
	}

    if (os_net_sem_create(&ret->sem, 0, 0) != OS_NET_STATUS_OK)
        goto error;

    ret->ready = true;
    return ret;
error:
    snfy_free(ret);
    return NULL;
}

os_net_status_t snfy_ready(snfy_handle_t *handle, void *value)
{
    if (NULL == handle)
        return OS_NET_STATUS_PARAM_INVALID;
    if (handle->ready == false)
        return OS_NET_STATUS_FAILED;
    handle->ready = true;
    handle->result = value;
    os_net_sem_release(&handle->sem);
}

void *snfy_await(snfy_handle_t *handle, os_net_time_t wait_ms)
{
    if (NULL == handle)
        return NULL;
    if(os_net_sem_wait(&handle->sem, wait_ms) != OS_NET_STATUS_OK)
        return NULL;

    void *result = handle->result;

    return result;
}

os_net_status_t snfy_free(snfy_handle_t *handle)
{
    if (NULL == handle)
        return OS_NET_STATUS_PARAM_INVALID;
    os_net_sem_delete(&handle->sem);
    free(handle);
    return OS_NET_STATUS_OK;
}
