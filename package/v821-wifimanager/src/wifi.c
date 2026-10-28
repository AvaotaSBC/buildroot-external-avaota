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
#include <sys/types.h>
#if (OS_NET_LINUX_OS || OS_NET_XRLINK_OS)
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <unistd.h>
#include <string.h>
#ifdef OS_NET_FREERTOS_OS
#include "console.h"
#endif
#include "wifimg.h"

#if (OS_NET_LINUX_OS || OS_NET_XRLINK_OS)
#define UNIX_DOMAIN "/tmp/UNIX_WIFI.domain"
#ifdef WMG_CMD_SOCKET_ACK
#define UNIX_DOMAIN_ACK "/tmp/UNIX_WIFI.domain_ack"
#endif
#else
int rtos_main(void *snd_cmd);
#endif

static void print_help()
{
	printf("wifimanager version:%s\n", WMG_VERSION);
#ifdef SUPPORT_STA_MODE
	printf("================sta mode Options===========\n"
		"wifi -o sta\n"
		"\t: open sta mode\n"
		"wifi -s\n"
		"\t: scan wifi\n"
		"wifi -c ssid [passwd]\n"
		"\t: connect to an encrypted or non-encrypted ap\n"
		"wifi -t ssid\n"
		"\t: try to connect to ap that has already been connected\n"
		"wifi -M bssid [passwd]\n"
		"\t: connect to an encrypted or non-encrypted ap use bssid\n"
		"wifi -a [enable/disable]\n"
		"\t: Auto reconnect\n"
		"wifi -l [all]\n"
		"\t: list connected or saved ap information\n"
		"wifi -r [ssid/all]\n"
		"\t: remove a specified network or all networks\n"
		"wifi -d\n"
		"\t: disconnect from ap\n"
		"wifi -f sta\n"
		"\t: close sta mode\n");
#endif
#ifdef SUPPORT_AP_MODE
	printf("================ap mode Options============\n"
		"wifi -o ap [ssid] [passwd] [channel] [wep/wpa/wpa2/wpa-wpa2]\n"
		"\t: open ap mode\n"
		"\t: if ssid and passwd is not set, start the default\n"
		"\t\tconfiguration:(allwinner-ap Aa123456 (channel:6))\n"
		"\t: if only set ssid, start the ap without passwd\n"
		"\t: if [wep/wpa/wpa2/wpa-wpa2] is not set, wpa2 is used by default\n"
		"wifi -s ap\n"
		"\t: scan wifi (Only some systems perform scan in ap mode)\n"
		"wifi -l ap\n"
		"\t: list current ap mode information\n"
		"wifi -d ap\n"
		"\t: disable ap mode\n"
		"wifi -f ap\n"
		"\t: close ap mode\n");
#endif
#ifdef SUPPORT_MONITOR_MODE
	printf("================monitor mode Options=======\n"
		"\t\t\tmonitor mode Options\n"
		"wifi -o monitor [channel] [f:N]\n"
		"\t: open monitor mode\n"
		"\t: if channel is not set, start the default channel:6))\n"
		"\t: f:N print frame number, default is 500\n"
		"wifi -d monitor\n"
		"\t: disable monitor mode\n"
		"wifi -f monitor\n"
		"\t: close monitor mode\n");
#endif
#ifdef SUPPORT_P2P_MODE
	printf("================p2p mode Options===========\n"
		"wifi -o p2p [N:name] [I:intent] [T/L:time] [A:enable]\n"
		"\t: open p2p mode(with devname intent listen(discover) time)\n"
		"\t: [A:enable]:Whether to autconnect when an p2p dev requests to connect\n"
		"wifi -s p2p [T:time/0] [P:social/progressive/full/listen] [get]\n"
		"\t: find(scan) p2p dev(time: How many seconds; 0: stop find(scan))\n"
		"\t: get peers\n"
		"wifi -C macaddr\n"
		"\t: connect to a p2p dev\n"
		"wifi -l p2p\n"
		"\t: list connected p2p dev(only in p2p go mode)\n"
		"wifi -d p2p\n"
		"\t: disable p2p mode\n"
		"wifi -f p2p\n"
		"\t: close p2p mode\n");
#endif
	printf("================other Options==============\n"
		"wifi -D [error/warn/info/debug/dump/exce/exce-mid/exce-max/open/close]\n"
		"\t: set debug level\n"
		"\t: open: print path file function line\n"
		"\t: close: do not print path file function line\n"
		"wifi -G (macaddr/countrycode)\n"
		"\t: get system mac addr or system countrycode\n"
		"wifi -S (macaddr/countrycode) (mxxx/cxxx)\n"
		"\t: set system mac addr or system countrycode\n"
		"\t: mxxx(mac addr like: fa:13:22:15:ef:bc) / cxxx (countrycode like: CN)\n"
		"wifi -i\n"
		"\t: get wifimanager info\n");
#if OS_NET_XRLINK_OS
	printf("wifi -v [length]\n"
		"\t: wifi user vendor msg test\n");
#endif
#ifdef SUPPORT_EXPAND
	printf("wifi -e [linux: XXX]/[xrlink: XXX]/[freertos: XXX]\n"
		"\t: wifi send expand cmd\n");
#endif
#ifdef SUPPORT_LINKD
	printf("wifi -p (softap/ble/xconfig/soundwave/qrcode) [Time]\n"
		"\t: softap/ble/xconfig/soundwave/qrcode distribution network\n"
		"\t: Time: maximum time 600 seconds\n"
		"\t:       if not set, it will be configured as 180 seconds\n");
#endif
	printf("wifi -f\n"
		"\t: close wifimanager\n"
		"wifi -h\n"
		"\t: print help\n"
		"==============================================\n");
}

int main(int argc, char *argv[])
{
	int ret;
	char snd_buf[DATA_BUF_SIZE];
#if (OS_NET_LINUX_OS || OS_NET_XRLINK_OS)
	int connect_fd;
	static struct sockaddr_un srv_addr;
#ifdef WMG_CMD_SOCKET_ACK
	int connect_fd_ack = -1, readcount = -1, flag = 1;
	static struct sockaddr_un srv_addr_ack;
#endif
#endif

	if((argc > 7 || argc <= 1) || (strncmp(argv[1], "-", 1)) || (strlen(argv[1]) != 2)){
		print_help();
		return -1;
	}

	memset(snd_buf, 0, DATA_BUF_SIZE);

	int ch = 0;
	for (;;) {
		ch = getopt(argc, argv, "ac:M:t:C:dfG:il::m:o:p:r:sS:v:e:D:h");
		if (ch < 0)
			break;
		switch (ch) {
			case 'o':
				if(!strcmp(argv[2], "sta")) {
					if(argc != 3) {
						print_help();
						return -1;
					}
					sprintf(snd_buf, "o sta");
#ifdef SUPPORT_AP_MODE
				} else if (!strcmp(argv[2], "ap")) {
					if(argc == 4) {
						sprintf(snd_buf, "o ap %s",argv[3]);
					} else if(argc == 5) {
						sprintf(snd_buf, "o ap %s %s",argv[3], argv[4]);
					} else if(argc == 6) {
						sprintf(snd_buf, "o ap %s %s %s",argv[3], argv[4], argv[5]);
					} else if(argc == 7) {
						sprintf(snd_buf, "o ap %s %s %s %s",argv[3], argv[4], argv[5], argv[6]);
					} else {
						sprintf(snd_buf, "o ap");
					}
#endif
#ifdef SUPPORT_MONITOR_MODE
				} else if(!strcmp(argv[2], "monitor")) {
					if(argc == 3) {
						sprintf(snd_buf, "o monitor");
					} else if(argc == 4){
						sprintf(snd_buf, "o monitor %s",argv[3]);
					} else if(argc == 5){
						sprintf(snd_buf, "o monitor %s %s",argv[3], argv[4]);
					} else {
						print_help();
						return -1;
					}
#endif
#ifdef SUPPORT_P2P_MODE
				} else if(!strcmp(argv[2], "p2p")) {
					if(argc == 3) {
						sprintf(snd_buf, "o p2p");
					} else if(argc == 4) {
						sprintf(snd_buf, "o p2p %s",argv[3]);
					} else if(argc == 5) {
						sprintf(snd_buf, "o p2p %s %s",argv[3], argv[4]);
					} else if(argc == 6) {
						sprintf(snd_buf, "o p2p %s %s %s",argv[3], argv[4], argv[5]);
					} else if(argc == 7) {
						sprintf(snd_buf, "o p2p %s %s %s %s",argv[3], argv[4], argv[5], argv[6]);
					} else {
						print_help();
						return -1;
					}
#endif
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
			case 'f':
				if(argc == 2) {
					sprintf(snd_buf, "f");
				} else if(argc == 3) {
					sprintf(snd_buf, "f %s",argv[2]);
				} else if (argc > 3){
					print_help();
					return -1;
				}
				goto send_cmd;
			case 's':
				if(argc == 3) {
					sprintf(snd_buf, "s %s",argv[2]);
				} else if (argc == 4){
					sprintf(snd_buf, "s %s %s",argv[2], argv[3]);
				} else if (argc == 5){
					sprintf(snd_buf, "s %s %s %s",argv[2], argv[3], argv[4]);
				} else {
					sprintf(snd_buf, "s");
				}
				goto send_cmd;
#ifdef SUPPORT_STA_MODE
			case 'c':
				if(argc == 3) {
					sprintf(snd_buf, "c %s",argv[2]);
				} else if (argc == 4) {
					sprintf(snd_buf, "c %s %s",argv[2], argv[3]);
				} else if (argc == 5) {
					sprintf(snd_buf, "c %s %s %s",argv[2], argv[3], argv[4]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
			case 't':
				if(argc == 3) {
					sprintf(snd_buf, "t %s",argv[2]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
			case 'M':
				if(argc == 3) {
					sprintf(snd_buf, "M %s",argv[2]);
				} else if (argc == 4) {
					sprintf(snd_buf, "M %s %s",argv[2], argv[3]);
				} else if (argc == 5) {
					sprintf(snd_buf, "M %s %s %s",argv[2], argv[3], argv[4]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
#endif
#ifdef SUPPORT_P2P_MODE
			case 'C':
				if(argc == 3) {
					sprintf(snd_buf, "C %s",argv[2]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
#endif
			case 'd':
				if(argc > 3) {
					print_help();
					return -1;
				} else if (argc == 2) {
					sprintf(snd_buf, "d");
				} else {
					sprintf(snd_buf, "d %s",argv[2]);
				}
				goto send_cmd;
			case 'a':
				if(argc == 3) {
					sprintf(snd_buf, "a %s", argv[2]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
			case 'l':
				if(argc == 2) {
					sprintf(snd_buf, "l");
				} else if (argc == 3) {
					if(!strcmp(argv[2], "all")) {
						sprintf(snd_buf, "l all");
					} else if(!strcmp(argv[2], "ap")) {
						sprintf(snd_buf, "l ap");
					} else if(!strcmp(argv[2], "p2p")) {
						sprintf(snd_buf, "l p2p");
					} else {
						print_help();
						return -1;
					}
				}
				goto send_cmd;
			case 'r':
				if(argc == 3) {
					if(!strcmp(argv[2], "all")) {
						sprintf(snd_buf, "r all");
					} else {
						sprintf(snd_buf, "r %s", argv[2]);
					}
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
			case 'G':
				if(argc > 3) {
					print_help();
					return -1;
				}
				sprintf(snd_buf, "G %s", argv[2]);
				goto send_cmd;
			case 'S':
				if(argc == 4) {
					sprintf(snd_buf, "S %s %s", argv[2], argv[3]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
			case 'i':
				if(argc > 2) {
					print_help();
					return -1;
				}
				sprintf(snd_buf, "i");
				goto send_cmd;
#if OS_NET_XRLINK_OS
			case 'v':
				if(argc == 3) {
					sprintf(snd_buf, "v %s", argv[2]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
#endif
#ifdef SUPPORT_EXPAND
			case 'e':
				if(argc == 3) {
                    sprintf(snd_buf, "e %s",argv[2]);
                } else if (argc == 4) {
                    sprintf(snd_buf, "e %s %s",argv[2], argv[3]);
                } else if (argc == 5) {
                    sprintf(snd_buf, "e %s %s %s",argv[2], argv[3], argv[4]);
                } else {
                    print_help();
                    return -1;
                }
				goto send_cmd;
#endif
#ifdef SUPPORT_LINKD
			case 'p':
				if(argc == 3) {
					sprintf(snd_buf, "p %s", argv[2]);
				} else if (argc == 4) {
					sprintf(snd_buf, "p %s %s",argv[2], argv[3]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
#endif
			case 'D':
				if(argc == 3) {
					sprintf(snd_buf, "D %s", argv[2]);
				} else {
					print_help();
					return -1;
				}
				goto send_cmd;
			case '?':
				printf("Unknown option: %c\n",(char)optopt);
			case 'h':
				print_help();
				return -1;
		}
	}

send_cmd:
#if (OS_NET_LINUX_OS || OS_NET_XRLINK_OS)
	//create unix socket
	connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(connect_fd < 0) {
		perror("Can't create wifi deamon communication socket\n");
		return -1;
	}
	srv_addr.sun_family = AF_UNIX;
	strcpy(srv_addr.sun_path,UNIX_DOMAIN);

	//connect server
	ret = connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if(ret == -1) {
		perror("Can't connect to the wifi daemon server");
		close(connect_fd);
		return ret;
	}

#ifdef WMG_CMD_SOCKET_ACK
	//create unix socket ack
	connect_fd_ack = socket(PF_UNIX, SOCK_STREAM, 0);
	if(connect_fd_ack > 0) {
		srv_addr_ack.sun_family = AF_UNIX;
		strcpy(srv_addr_ack.sun_path,UNIX_DOMAIN_ACK);
	} else {
		perror("Can't create wifi deamon communication ack socket\n");
		close(connect_fd);
		return -1;
	}

	//connect server
	if(connect(connect_fd_ack, (struct sockaddr*)&srv_addr_ack, sizeof(srv_addr_ack)) == -1) {
		perror("Can't connect to the wifi daemon ack server");
		close(connect_fd_ack);
		return -1;
	}
#endif

	write(connect_fd, snd_buf, sizeof(snd_buf));

#ifdef WMG_CMD_SOCKET_ACK
	memset(snd_buf, 0, DATA_BUF_SIZE);
	printf("=======================wifi dump log=========================\n");
	do {
		readcount = read(connect_fd_ack, snd_buf, DATA_BUF_SIZE - 1);
		snd_buf[readcount] = '\0';
		if ((readcount >= 7) && (!strncmp(&snd_buf[readcount - 7], "cmd end", 7))) {
			flag = 0;
			if (readcount != 7) {
				snd_buf[readcount - 7] = '\0';
				//printf("[test:%d] %s",readcount, snd_buf);
				printf("%s", snd_buf);
			}
		} else {
			//printf("[test:%d] %s",readcount, snd_buf);
			printf("%s", snd_buf);
		}
	} while (flag);
	printf("=======================wifi dump end=========================\n");
	close(connect_fd_ack);
#endif
	close(connect_fd);
#else
	rtos_main(snd_buf);
#endif //OS_NET_LINUX_OS
	return 0;
}

#ifdef OS_NET_FREERTOS_OS
FINSH_FUNCTION_EXPORT_CMD(main, wifi, wifi testcmd);
#endif
