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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "log_worker.h"
#include "log_queue_block.h"

struct worker_work {
    void *(*function)(void *);
    void *arg;
    void **ret;
};

static log_queue_t worker_queue;
static pthread_t worker_handle;

int log_worker_init()
{
    if (log_queue_init(&worker_queue) != 0) {
        return -1;
    }
    if (pthread_create(&worker_handle, NULL, log_worker_loop, NULL)) {
        return -1;
    }
    return 0;
}

int log_worker_deinit()
{
    log_queue_destroy(&worker_queue);
    pthread_cancel(worker_handle);
    return 0;
}

void *log_worker_loop(void *arg)
{
    struct worker_work work;
    struct worker_work *work_ptr = &work;

    // if (!&worker_queue)
    //     return;

    while (1) {
        log_queue_pop(&worker_queue, (void *)&work_ptr);
        if (work.function) {
            *work.ret = work.function(work.arg);
        }
    }
    log_queue_destroy(&worker_queue);
    return NULL;
}

int log_worker_schedule(void *(*function)(void *), void *arg, void **ret)
{
    struct worker_work work;

    if (!function) {
        return ENOEXEC;
    }

    work.function = function;
    work.arg = arg;
    work.ret = ret;
    if (log_queue_push(&worker_queue, &work) != 0) {
        return ENOMEM;
    }

    return 0;
}
