/*
 * Copyright (C) 2022 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <wifi_log.h>

#include "msg_netlink.h"
#define XR_NETLINK_PREFIX_LEN (3)
#define XR_NETLINK_PREFIX_CMD "cmd"
#define XR_NETLINK_ACK_OK     "ack"

static int nlmsg_recv(nlmsg_priv_t *priv)
{
	struct nlmsghdr *nlh = NULL;
	struct sockaddr_nl src_addr;
	struct iovec iov;
	int len = 0;

	struct msghdr msg;

	nlh = (struct nlmsghdr*) priv->rx;

	iov.iov_base = (void*) nlh;
	iov.iov_len = NLMSG_SPACE(TX_RX_MAX_LEN);
	memset(&src_addr, 0, sizeof(struct sockaddr_nl));

	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = (void *)&src_addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	len = recvmsg(priv->fd, &msg, 0);
	if(len < 0) {
		WMG_ERROR("recvmsg error!\n");
		return -1;
	}

	len = nlh->nlmsg_len - NLMSG_SPACE(0);
	if(len < 0) {
		WMG_ERROR("len < 0 !\n");
		return -1;
	}

	if(len > TX_RX_MAX_LEN) {
		WMG_ERROR("len(%d) overflow!(%d)\n", len, TX_RX_MAX_LEN);
		return -1;
	}

	if (memcmp((char *)NLMSG_DATA(nlh), XR_NETLINK_PREFIX_CMD,
				XR_NETLINK_PREFIX_LEN) == 0) {

		WMG_DUMP("recv cmd\n");

		memcpy(priv->rx, (char *)NLMSG_DATA(nlh) + XR_NETLINK_PREFIX_LEN, len);

		return len;

	} else if (memcmp((char *)NLMSG_DATA(nlh), XR_NETLINK_ACK_OK,
				XR_NETLINK_PREFIX_LEN) == 0) {

		WMG_DEBUG("recv ack\n");

		os_net_sem_release(&priv->sem);

		return 0;
	}

	WMG_ERROR("recv unknown data\n");
	return -1;
}

static void *nlmsg_thread_handle(void *arg)
{
	int len = 0;

	WMG_DEBUG("nlmsg thread start.\n");
	nlmsg_priv_t *priv = (nlmsg_priv_t *) arg;
	for(;;) {
		len = nlmsg_recv(priv);
		if(len > 0) {
			if(priv->recv_cb) {
				priv->recv_cb(priv->rx,len);
			}
		}
	}

	return NULL;
}

#define OS_NETLINK_SEM_LEN (24)
nlmsg_priv_t *nlmsg_init(recv_cb_t cb)
{
	nlmsg_priv_t *priv = NULL;
	struct sockaddr_nl addr;
	int fd = -1;
	os_net_thread_pid_t pid;
	int ret = -1;
	os_net_thread_get_pid(&pid);

	priv = (nlmsg_priv_t *)malloc(sizeof(nlmsg_priv_t));
	if(!priv) {
		WMG_ERROR("No memory.\n");
		return NULL;
	}

	memset(priv, 0, sizeof(nlmsg_priv_t));

	priv->rx = (char *)malloc(TX_RX_MAX_LEN);
	if(!priv->rx) {
		free(priv);
		return NULL;
	}

	if (os_net_sem_create(&priv->sem, 0,
				OS_NETLINK_SEM_LEN) != OS_NET_STATUS_OK) {
            OS_NET_ERROR("create netlink sem failed.\n");

		free(priv->rx);
		free(priv);

		return NULL;
	}

	priv->fd = socket(AF_NETLINK, SOCK_RAW, NLMSG_PROTO);
	if(!priv->fd) {
		WMG_ERROR("NLMSG create failed.\n");
        os_net_sem_delete(&priv->sem);
		free(priv->rx);
		free(priv);
		return NULL;
	}

	memset(&addr, 0, sizeof(struct sockaddr_nl));
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = (uint32_t)pid;
	addr.nl_groups = 0;

	priv->rx_pid = pid;

	WMG_DEBUG("netlink thread pid:%d\n", priv->rx_pid);

	ret = bind(priv->fd, (struct sockaddr *)&addr,
			sizeof(struct sockaddr_nl));
	if(ret < 0) {
		close(priv->fd);
		WMG_ERROR("NLMSG bind failed.\n");
		goto error;
	}

	priv->recv_cb = cb;

	if(os_net_thread_create(&priv->thread, "nlmsg", nlmsg_thread_handle,
				priv,0,4096) != OS_NET_STATUS_OK) {
		goto error;
	}
	return priv;
error:
    os_net_sem_delete(&priv->sem);
	close(priv->fd);
	free(priv->rx);
	free(priv);
	return NULL;
}

os_net_status_t nlmsg_deinit(nlmsg_priv_t *priv)
{

	WMG_DEBUG("nlmsg_deinit\r\n");

	os_net_thread_delete(&priv->thread);

	if(priv->fd > 0) {
		close(priv->fd);
	}

	if(priv && priv->rx) {
		free(priv->rx);
	}

	if(priv) {
        os_net_sem_delete(&priv->sem);
		free(priv);
	}

	return OS_NET_STATUS_OK;
}

os_net_status_t nlmsg_send(nlmsg_priv_t *priv, char *data, uint32_t len)
{
	struct nlmsghdr *nlh = NULL;
	int nlmsg_len = 0;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl dest_addr;
	int ret;

	if(!priv) {
		WMG_ERROR("handle is invaild");
		return OS_NET_STATUS_PARAM_INVALID;
	}

	if(priv->fd < 0) {
		WMG_ERROR("handle is invaild, priv->fd=%d\n", priv->fd);
		return OS_NET_STATUS_PARAM_INVALID;
	}

	/*dest address*/
	memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; //for linux kernel
	dest_addr.nl_groups = 0;

	/*fill the netlink message header*/
	nlmsg_len = NLMSG_SPACE(len);

	nlh = (struct nlmsghdr *)malloc(nlmsg_len);

	memset(nlh, 0, nlmsg_len);
	nlh->nlmsg_len = nlmsg_len;
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_pid = (uint32_t)priv->rx_pid; //self pid

	/*fill in the netlink message payload*/
	memcpy(NLMSG_DATA(nlh), data, len);

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(struct msghdr));

	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;

	//log_hex_dump(__func__,20,data,len);

	WMG_DEBUG("send msg len:%d\n", len);

	WMG_DEBUG("send msg start\n");
	ret = sendmsg(priv->fd, &msg, 0);
	if (ret < 0) {
		WMG_ERROR("Send error:%d.\n", ret);
		free(nlh);
		return OS_NET_STATUS_FAILED;
	}

	free(nlh);
	ret = os_net_sem_wait(&priv->sem, OS_NET_WAIT_FOREVER);

    if (ret != OS_NET_STATUS_OK) {
		WMG_ERROR("netlink wait error:%d\n", ret);
		return OS_NET_STATUS_FAILED;
	}
	WMG_DEBUG("send msg end\n");

	return OS_NET_STATUS_OK;
}

