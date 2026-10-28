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
#ifndef __LOG_QUEUE_BLOCK_H__
#define __LOG_QUEUE_BLOCK_H__

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#include <pthread.h>

struct log_queue_node {
    struct log_queue_node *next;
    struct log_queue_node *prev;
    void *data;
};

typedef struct log_queue {
    struct log_queue_node *front;
    struct log_queue_node *back;
    unsigned long size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} log_queue_t;

/*
 * Initializes blocking queue instance.
 * 
 * Returns 0 on success.
 * Returns error code on error.
 *  See pthread_{mutex,cond}_init
 */
int log_queue_init(log_queue_t *b);

/*
 * Pushes data into blocking queue.
 * 
 * Returns 0 on success.
 * Returns error code on error.
 *  See pthread_mutex_[un]lock
 */
int log_queue_push(log_queue_t *b, void *data);

/*
 * Pops data from blocking queue.
 * Blocks until data available.
 * 
 * Returns 0 on success and *data is set.
 * Returns error code on error and *data is not modified.
 *  See pthread_{mutex_[un]lock,cond_wait}
 */
int log_queue_pop(log_queue_t *b, void **data);

/*
 * Destroy instance.
 * Free all heap allocated nodes.
 * 
 * Returns 0 on success.
 * Returns error code on error.
 *  See pthread_{mutex,cond}_destroy
 */
int log_queue_destroy(log_queue_t *b);

/*
 * Thread-safely get queue size.
 * Sets *size with queue size.
 * 
 * Returns 0 on success.
 */
int log_queue_size(log_queue_t *b, unsigned long *size);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif /* __LOG_QUEUE_BLOCK_H__ */