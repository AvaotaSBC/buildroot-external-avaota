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
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <wifi_log.h>
#include <wmg_common.h>
#include <xrlink_get_config.h>

static char *del_left_trim(char *str)
{
	assert(str != NULL);
	for(;*str != '\0' && isblank(*str); ++str);
	return  str;
}

static char *del_both_trim(char * str)
{
	char *p;
	char * szOutput;
	szOutput = del_left_trim(str);
	for(p = szOutput + strlen(szOutput) - 1; p >= szOutput && isblank(*p); --p)
		*(++p) = '\0';
	return  szOutput;
}

wmg_status_t get_config(char *config_name, char *config_buf, char *config_default)
{
	FILE * fp = NULL;
	char s[256] = {0};
	char *delim = "=";
	char *p;
	char ch;
	char config_file[256] = {0};

	/*open config file*/
	sprintf(config_file, "%s/wifimg.config", WMG_CONFIG_PATH);
	if((fp = fopen(config_file, "r")) == NULL) {
		WMG_WARNG("Fail to open config file(%s/wifimg.config)!, open default config file(%s/wifimg.def) now\n", WMG_CONFIG_PATH, WMG_CONFIG_PATH);
		memset(config_file, 0, 256);
		sprintf(config_file, "%s/wifimg.def", WMG_CONFIG_PATH);
		if((fp = fopen(config_file, "r")) == NULL) {
			WMG_WARNG("Fail to open default config file(%s/wifimg.def)!\n", WMG_CONFIG_PATH);
			goto get_config_fail;
		} else {
			WMG_DEBUG("open config file(%s/wifimg.def)!\n", WMG_CONFIG_PATH);
		}
	} else {
		WMG_DEBUG("open config file(%s/wifimg.config)!\n", WMG_CONFIG_PATH);
	}

	while (!feof(fp)) {
		if((p = fgets(s, sizeof(s), fp)) != NULL) {
			ch=del_left_trim(s)[0];

			if(ch == '#' || isblank(ch) || ch == '\n' )
				continue ;
			p=strtok(s, delim);
			if(p) {
				if(!strcmp(config_name, del_both_trim(p))) {
					while((p = strtok(NULL, delim))) {
						strcpy(config_buf, p);
						if(config_buf[strlen(config_buf) - 1] == '\n') {
							config_buf[strlen(config_buf) - 1] = '\0';
						}
						WMG_DEBUG("get config: %s=%s\n", config_name, config_buf);
						fclose(fp);
						return WMG_STATUS_SUCCESS;
					}
				} else {
					p = strtok(NULL, delim);
				}
			}
		}
	}
get_config_fail:
	if(config_default != NULL) {
		strcpy(config_buf, config_default);
		WMG_WARNG("Can't get config: %s, used default config: %s\n", config_name, config_default);
	} else {
		WMG_WARNG("Can't get config: %s\n", config_name);
	}
	fclose(fp);
	return WMG_STATUS_FAIL;
}

wmg_status_t set_config(char *config_name, char *config_buf)
{
	wmg_status_t ret = WMG_STATUS_FAIL;
	FILE *fp_src = NULL, *fp_des = NULL;
	char s[256] = {0};
	char replace_line[256] = {0};
	int get_config_flag = 0;
	char *delim = "=";
	char *p = NULL, *p_tmp = NULL;
	char ch;
	char config_file[256] = {0};
	char config_file_new[256] = {0};

	/*open config file and new config file*/
	sprintf(config_file, "%s/wifimg.config", WMG_CONFIG_PATH);
	sprintf(config_file_new, "%s/wifimg_new.config", WMG_CONFIG_PATH);
	if((fp_src = fopen(config_file, "r")) == NULL) {
		WMG_WARNG("Fail to open config file(%s/wifimg.config)!, open default config file(%s/wifimg.def) now\n", WMG_CONFIG_PATH, WMG_CONFIG_PATH);
		memset(config_file, 0, 256);
		memset(config_file_new, 0, 256);
		sprintf(config_file, "%s/wifimg.def", WMG_CONFIG_PATH);
		sprintf(config_file_new, "%s/wifimg_new.def", WMG_CONFIG_PATH);
		if((fp_src = fopen(config_file, "r")) == NULL) {
			WMG_WARNG("Fail to open default config file(%s/wifimg.def)!\n", WMG_CONFIG_PATH);
			goto set_config_fail;
		} else {
			WMG_DEBUG("open config file(%s/wifimg.def)!\n", WMG_CONFIG_PATH);
			if((fp_des = fopen(config_file_new, "w")) == NULL) {
				WMG_WARNG("Fail to open default config_new file(%s/wifimg_new.def)!\n", WMG_CONFIG_PATH);
				goto set_config_fail;
			}
		}
	} else {
		WMG_DEBUG("open config file(%s/wifimg.config)!\n", WMG_CONFIG_PATH);
		if((fp_des = fopen(config_file_new, "w")) == NULL) {
			WMG_WARNG("Fail to open config_new file(%s/wifimg_new.config)!\n", WMG_CONFIG_PATH);
			goto set_config_fail;
		}
	}

	while(!feof(fp_src)) {
		if((p = fgets(s, sizeof(s), fp_src)) != NULL) {
			WMG_DEBUG("config: %s\n", p);
			ch=del_left_trim(s)[0];

			/* Not that we need the data, write it back to a new file */
			if(ch == '#' || isblank(ch) || ch == '\n') {
				fputs(s,fp_des);
				continue ;
			}

			memcpy(replace_line , s, 256);
			p = strtok(replace_line, delim);
			if(p) {
				if(!strcmp(config_name, del_both_trim(p))) {
					get_config_flag = 1;
					fputs(config_name, fp_des);
					fputs("=", fp_des);
					fputs(config_buf, fp_des);
					fputs("\n", fp_des);
					WMG_DEBUG("set config: %s=%s\n", config_name, config_buf);
					ret = WMG_STATUS_SUCCESS;
				}
			}
			if(!get_config_flag) {
				fputs(s,fp_des);
			}
			get_config_flag = 0;
		}
	}

	fflush(fp_des);

	if(remove(config_file) == 0) {
		WMG_DEBUG("remove config file %s\n", config_file);
		if(rename(config_file_new, config_file) == 0) {
			WMG_DEBUG("rename config file success\n");
		} else {
			WMG_ERROR("rename config file fail\n");
			ret = WMG_STATUS_FAIL;
			goto set_config_fail;
		}
	} else {
		WMG_ERROR("remove config file %s fail\n", config_file);
		ret = WMG_STATUS_FAIL;
		goto set_config_fail;
	}

set_config_fail:
	if(ret != WMG_STATUS_SUCCESS) {
		WMG_WARNG("Can't set config: %s=%s\n", config_name, config_buf);
	}
	if(fp_src != NULL) {
		fclose(fp_src);
	}
	if(fp_des != NULL) {
		fclose(fp_des);
	}
	return ret;
}
