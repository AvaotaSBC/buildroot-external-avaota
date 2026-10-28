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
#include "log_core.h"
#include "log_core_inner.h"
#include "log_backend_hub.h"
#include <pthread.h>

static FILE *out_file = NULL;
static char *last_path = NULL;
static int error_times = 0;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static int log_hub_open_file(const char *path)
{
    if (!path) {
        return 0;
    }

    if (last_path == NULL || strcmp(last_path, path) != 0) {
        /* Save our path to enable re-open */
        free(last_path);
        last_path = strdup(path);
    }

    out_file = fopen(path, "a");
    if (out_file == NULL) {
        printf("btmg_debug_open_file: Failed to open "
               "output file, using standard output");
        return -1;
    }
    error_times = 0;
    return 0;
}

static void log_hub_close_file(void)
{
    if (!out_file)
        return;
    fclose(out_file);
    out_file = NULL;
    free(last_path);
    last_path = NULL;
}

void log_hub_vprintf(const char *format, va_list args)
{
    if (global_log_type.type & LOG_BACKEND_STDOUT_SYNC) {
        vprintf(format, args);
        fflush(stdout);
    } else if (global_log_type.type & LOG_BACKEND_STDOUT_ASYNC) {
        // todo: not support yet
        // todo: using cas atomic concurrent queue. work queue
    }
    if (global_log_type.type & LOG_BACKEND_FILE_SYNC) {
        pthread_mutex_lock(&log_mutex);
        if (out_file) {
            // nwrite = vsnprintf(tail, size_left, format, args);
            vfprintf(out_file, format, args);
            fflush(out_file);
            // fprintf(out_file, "\n");
        } else {
            if (error_times > 20) {
                pthread_mutex_unlock(&log_mutex);
                return;
            }
            if (error_times == 20) {
                error_times++;
                log_hub_open_file(DEFAULT_LOG_FILE);
                printf("error try to use default path [%s][%d]",__func__,__LINE__);
            } else if ((last_path != NULL) && (log_hub_open_file(last_path) == 0)) {
                vfprintf(out_file, format, args);
                fflush(out_file);
            } else {
                error_times++;
            }
        }
        pthread_mutex_unlock(&log_mutex);
    } else if (global_log_type.type & LOG_BACKEND_FILE_ASYNC) {
        // todo: not support yet
        // todo: using cas atomic concurrent queue. work queue
    }
    if (global_log_type.type & LOG_BACKEND_SYSLOG_SYNC) {
        // todo: not support yet
    }
}

int log_hub_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format); //初始化参数指针
    log_hub_vprintf(format, args);
    va_end(args);
}

void log_hub_fflush(void *file)
{
    if (file) {
        fflush(file);
    } else {
        fflush(out_file);
    }

    return;
}

int log_hub_start(const char *path)
{
    if (path == NULL) {
        return 0;
    }
    pthread_mutex_lock(&log_mutex);
    if (path && !out_file) {
        log_hub_open_file(path);
        pthread_mutex_unlock(&log_mutex);
        return 0;
    } else if (!strcmp(path, last_path) && out_file){
        // path same and out_file exist nothing to do
        pthread_mutex_unlock(&log_mutex);
        return 0;
    } else if (strcmp(path, last_path)) {
        // path diff reopen
        log_hub_close_file();
        log_hub_open_file(path);
        pthread_mutex_unlock(&log_mutex);
        return 0;
    }
    pthread_mutex_unlock(&log_mutex);
    return 0;
}

int log_hub_stop()
{
    pthread_mutex_lock(&log_mutex);
    if (out_file) {
        log_hub_close_file();
    }
    pthread_mutex_unlock(&log_mutex);
    return 0;
}
