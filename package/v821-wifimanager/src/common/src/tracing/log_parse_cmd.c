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
#include "log_parse_cmd.h"

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <string.h>

static int log_thread_state = 0;
static int epollfd = -1;
static int fd = -1;
pthread_t log_thread_handle;
static int (*extern_parase_function)(char *key, char *op, char *val) = NULL;
static char path[128];

#define DEFINE_TESET2(log_struct, key, op, val)                                                    \
    {                                                                                              \
        LOG_DEBUG("key = %s,op='%s',val=%s", key, op, val);                                        \
        if (strcmp(op, "+=") == 0) {                                                               \
            if (strcmp(key, "type") == 0) {                                                        \
                log_struct.type |= (log_hub_type_t)strtol(val, NULL, 0);                           \
            } else if (strcmp(key, "setting") == 0) {                                              \
                log_struct.setting |= (int)strtol(val, NULL, 0);                                   \
            } else if (strcmp(key, "level") == 0) {                                                \
                log_struct.level |= (log_level_t)strtol(val, NULL, 0);                             \
                LOG_DEBUG("level = %ld\n", strtol(val, NULL, 0));                                  \
            } else {                                                                               \
                LOG_ERROR("error key = %s", key);                                                  \
            }                                                                                      \
        } else if (strcmp(op, "-=") == 0) {                                                        \
            if (strcmp(key, "type") == 0) {                                                        \
                log_struct.type &= ~(log_hub_type_t)strtol(val, NULL, 0);                          \
            } else if (strcmp(key, "setting") == 0) {                                              \
                log_struct.setting &= ~(int)strtol(val, NULL, 0);                                  \
            } else if (strcmp(key, "level") == 0) {                                                \
                log_struct.level &= ~(log_level_t)strtol(val, NULL, 0);                            \
                LOG_DEBUG("level = %ld\n", strtol(val, NULL, 0));                                  \
            } else {                                                                               \
                LOG_ERROR("error key = %s", key);                                                  \
            }                                                                                      \
        } else if (strcmp(op, "=") == 0) {                                                         \
            if (strcmp(key, "type") == 0) {                                                        \
                log_struct.type = (log_hub_type_t)strtol(val, NULL, 0);                            \
            } else if (strcmp(key, "setting") == 0) {                                              \
                log_struct.setting = (int)strtol(val, NULL, 0);                                    \
            } else if (strcmp(key, "level") == 0) {                                                \
                log_struct.level = (log_level_t)strtol(val, NULL, 0);                              \
                LOG_DEBUG("level = %ld\n", strtol(val, NULL, 0));                                  \
            } else {                                                                               \
                LOG_ERROR("error key = %s", key);                                                  \
            }                                                                                      \
        } else {                                                                                   \
            LOG_ERROR("errror operator = %s", op);                                                 \
        }                                                                                          \
    }

static char *strdelspace(char *in)
{
    char *out = NULL;
    char *p = in;
    while ((*p == ' ') || (*p == '\t'))
        p++;
    out = p;
    while (1) {
        if (*p == ' ')
            break;
        if (*p == '\n')
            break;
        if (*p == '\0')
            break;
        if (*p == '\t')
            break;
        p++;
    }
    *p = '\0';
    return out;
}

static void log_process_cmdline(char *input_str, uint32_t len)
{
    // 	echo "type = LOG_LEVEL_WARNING"> debug
    // 	echo "level = 5"> debug
    // 	echo "setting+=LOG_SETTING_FFLUSH" > debug
    // echo 5 > debug
    do {
        if (len > 64) {
            return;
        }
        if (input_str[0] == '\n') {
            return;
        }
        if (len == 2) {
            int debug_level = atoi(input_str);
            if (debug_level >= 0 && debug_level <= 5) {
                global_log_type.level = (log_level_t)debug_level;
                return;
            } else {
                break;
            }
        }
    } while (0);

    static char key[64] = { 0 };
    static char val[64] = { 0 };
    memset(key, 0, sizeof(key));
    memset(val, 0, sizeof(val));
    char op[3] = { 0 };

    if (strstr(input_str, "+=")) {
        sscanf(input_str, "%30[0-9a-zA-Z\t ]+=%30s", key, val);
        strcpy(op, "+=");
    } else if (strstr(input_str, "-=")) {
        sscanf(input_str, "%30[0-9a-zA-Z\t ]-=%30s", key, val);
        strcpy(op, "-=");
    } else if (strstr(input_str, "=")) {
        sscanf(input_str, "%30[0-9a-zA-Z\t ]=%30s", key, val);
        strcpy(op, "=");
    }
    char *k = strdelspace(key);
    char *v = strdelspace(val);
    LOG_DEBUG("log_core: %s, key = %s, val = %s\n", input_str, k, v);

    DEFINE_TESET2(global_log_type, k, op, v);
    if (extern_parase_function) {
        extern_parase_function(k, op, v);
    }
    return;
}

static void *parse_log_main(void *arg)
{
    if (arg != NULL && strlen(arg) > 0 && strlen(arg) < 127) {
        strcpy(path, arg);
        path[127] = '\0';
        LOG_DEBUG("debug path = %s", path);
    } else {
        strcpy(path, DEFAULT_LOG_CMD_FIFO);
        LOG_DEBUG("debug path = %s", path);
    }

    while (log_thread_state == 1 || log_thread_state == 2) { // simple sync operator
        usleep(10 * 10);
    }
    log_thread_state = 1;
    if (path == NULL) {
        return NULL;
    }
    if (access(path, F_OK) == -1) {
        if (mkfifo(path, 0777) < 0) {
            LOG_ERROR("mkfifio fail,%s\n", strerror(errno));
        }
    }

    struct epoll_event ev;
    epollfd = epoll_create(64);
    int ret = 0;

    if (epollfd < 0) {
        LOG_ERROR("error epollfd = %d", epollfd);
        return NULL;
    }
    fd = open(path, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) {
        LOG_ERROR("error fd = %d", fd);
        return NULL;
    }
    ev.data.fd = fd;
    ev.events = POLLIN | POLLHUP | POLLERR; // cannot set as output

    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret) {
        LOG_ERROR("error epoll_ctl,%d,%d", ret, errno);
    }
    struct epoll_event evs[64];
    while (log_thread_state == 1) {
        LOG_DEBUG("log_thread_state");
        int n = epoll_wait(epollfd, evs, 64, -1);
        if (n <= 0) {
            continue;
        }
        if (evs[0].events & EPOLLERR || evs[0].events & EPOLLHUP) {
            LOG_ERROR("POLLHUP");
            close(evs[0].data.fd);
            continue;
        } else if (fd == evs[0].data.fd) {
            if (evs[0].events & POLLIN) {
                LOG_DEBUG("POLLIN");
                char buf[1024] = { 0 };
                size_t len = 0;
                len = read(fd, buf, sizeof(buf));
                if (len <= 0) {
                    if (errno == EAGAIN) {
                        LOG_ERROR("fifo read no_data");
                    } else {
                        LOG_ERROR("fifo read %s", strerror(errno));
                    }
                } else {
                    LOG_DEBUG("fifo read len=%lu", len);
                    buf[len - 1] = '\0';
                    log_process_cmdline(buf, strlen(buf) + 1);
                }
            }
        }
        // if (evs.events & EPOLLET) {
        // 	LOG_ERROR("EPOLLET\n");
        // }
        // if (evs.events & EPOLLOUT) {
        // 	write(fd,"a",2);
        // 	LOG_ERROR("EPOLLOUT\n");
        // }
        LOG_DEBUG("epoll_wait ret=%d evs.events=%d", n, evs[0].events);
        usleep(100);
    }
    log_thread_state = 2;
    return NULL;
}

int log_parse_cmd_start(char *fifo_path)
{
    int ret = 0;
    ret = pthread_create(&log_thread_handle, NULL, parse_log_main, fifo_path);
    if (ret != 0) {
        LOG_ERROR("Couldn't create bt_routine worker :%s", strerror(ret));
        return -1;
    }
    return 0;
}

int log_parse_cmd_stop(void)
{
    log_thread_state = 0;
    pthread_detach(log_thread_handle);
    pthread_cancel(log_thread_handle);
    usleep(10);
    if (epollfd >= 0) {
        close(epollfd);
        epollfd = -1;
    }
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
    if (path != NULL) {
        unlink(path);
    }
    log_thread_state = 0;
    return 0;
}

/**
 * @input: callback_function
 * @param: echo a1 +=  1b > logcore,[key = a1, op = '+=' val=1b]
 *         op in ['=','+=','-=']  
 **/
int log_parse_cmd_register_function(int (*fun)(char *key, char*op, char*val))
{
    extern_parase_function = fun;
    return 0;
}
