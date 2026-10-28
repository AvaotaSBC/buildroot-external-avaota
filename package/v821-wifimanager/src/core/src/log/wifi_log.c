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
#ifdef OS_NET_FREERTOS_OS
#include <stdio.h>
#include <wifi_log.h>
#else
#include <syslog.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdbool.h>
#include <wifi_log.h>
#include <unistd.h>
#endif

#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

typedef struct {
	wmg_log_level_t wmg_debug_level;
	int wmg_log_para;
} global_log_type_t;

static global_log_type_t global_log = {
	.wmg_debug_level = WMG_MSG_INFO,
	.wmg_log_para = 0,
};

static void wmg_printf(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

#ifndef OS_NET_FREERTOS_OS
static int syslog_priority(int level)
{
	switch (level) {
	case WMG_MSG_EXCESSIVE:
	case WMG_MSG_MSGDUMP:
	case WMG_MSG_DEBUG:
		return LOG_DEBUG;
	case WMG_MSG_INFO:
		return LOG_NOTICE;
	case WMG_MSG_WARNING:
		return LOG_WARNING;
	case WMG_MSG_ERROR:
		return LOG_ERR;
	}
	return LOG_INFO;
}
#endif

void wmg_print(int level, int para_enable, const char *tag, const char *file, const char *func, int line, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	if(level <= global_log.wmg_debug_level) {
#ifndef OS_NET_FREERTOS_OS
		vsyslog(syslog_priority(level), fmt, ap);
		if(para_enable && (global_log.wmg_log_para != 0)) {
			char strtimestamp[64];
			struct timeval tv;
			struct tm nowtm;
			gettimeofday(&tv, NULL);
			localtime_r(&tv.tv_sec, &nowtm);
			strftime(strtimestamp, sizeof(strtimestamp), "%Y-%m-%d %H:%M:%S", &nowtm);
			wmg_printf("%s:%03d: ", strtimestamp, tv.tv_usec / 1000);
		}
#endif

		if(tag != NULL) {
			wmg_printf("%s", tag);
		}

#ifndef OS_NET_FREERTOS_OS
		if(para_enable && (global_log.wmg_log_para != 0)) {
			if(file != NULL) {
				wmg_printf("[%s:%s:%d]", file, func, line);
			}
		}
#endif

		vprintf(fmt, ap); //main output
		fflush(stdout);
	}

	va_end(ap);
	return;
}

void wmg_set_debug_level(wmg_log_level_t level)
{
	global_log.wmg_debug_level = level;
}

int wmg_get_debug_level()
{
	return global_log.wmg_debug_level;
}

void wmg_set_log_para(int log_para)
{
	global_log.wmg_log_para = log_para;
}

int wmg_get_log_para()
{
	return global_log.wmg_log_para;
}

/*
 * This interface is used to transfer standard output to other io devices
 * @fd(> 0): witch io devices fd
 * @fd(-1): printing switches back to standard output
 * */
void wmg_set_log_fd(int fd)
{
	static int old_stdout = -1;
	fflush(stdout);
	usleep(5000);
	if(fd == -1) {
		if(old_stdout != -1) {
			dup2(old_stdout, 1);
			close(old_stdout);
			old_stdout = -1;
		}
	} else {
		if(old_stdout == -1) {
			old_stdout = dup(1);
			dup2(fd, 1);
		}
	}
}
