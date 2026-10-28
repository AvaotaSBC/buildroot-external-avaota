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
#include <os_net_queue.h>
#include <ring_buff.h>

os_net_status_t os_net_queue_create(os_net_queue_t *queue, uint32_t queue_len, uint32_t msg_size)
{
    if (ring_buff_init(queue, queue_len * msg_size) != 0) {
        return OS_NET_STATUS_NOMEM;
    }
    ring_buff_start(queue);
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_queue_send(os_net_queue_t *queue, void *msg, uint32_t msg_size, int timeout)
{
    if (ring_buff_put(queue, msg, msg_size, 0, RB_SG_NOMAL) == 0) {
        return OS_NET_STATUS_NOMEM;
    }
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_queue_receive(os_net_queue_t *queue, void *msg, uint32_t msg_size,
                                     int timeout)
{
    if (ring_buff_get(queue, msg, msg_size, timeout, 0) == 0) {
        return OS_NET_STATUS_FAILED;
    }
    return OS_NET_STATUS_OK;
}

os_net_status_t os_net_queue_delete(os_net_queue_t *queue)
{
    ring_buff_stop(queue);
    ring_buff_deinit(queue);

    return OS_NET_STATUS_OK;
}
