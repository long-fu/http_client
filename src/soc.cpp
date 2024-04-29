
#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h>
#include <vector>
// #include <sockets.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <string.h>
#include <netdb.h>

#include "soc.h"




int get_eth_name_from_ip(char *eth_name,const char *ip_addr)
{
    struct ifaddrs *ifaddr;
    int family, s;
    char host[1024];
    // printf("ipaddr %s", ip_addr);
    if (getifaddrs(&ifaddr) == -1)
    {
        // perror("getifaddrs");
        // exit(EXIT_FAILURE);
        // ERROR_LOG() << "[SOC] getifaddrs " << errno << " msg: " << strerror(errno);
        return -1;
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET || family == AF_INET6)
        {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST,
                            NULL, 0, NI_NUMERICHOST);
            if (s != 0)
            {
                // printf("getnameinfo() failed: %s\n", gai_strerror(s));
                // exit(EXIT_FAILURE);
                // ERROR_LOG() << "[SOC] getnameinfo() failed: %s " << errno << " msg: " << gai_strerror(s);
                return -1;
            }
            // printf("%s:%s", host, ip_addr);
            if (!strcmp(host, ip_addr))
            {
                printf("%-8s ", ifa->ifa_name);
                printf("address: <%s>\n", host);
                // memcpy();
                strcpy(eth_name, ifa->ifa_name);
            }
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}

// ethx src_ip src_port 参数无效
int socket_create(char *ethx, char *src_ip, int src_port, const char *dest_ip, int dest_port)
{

  int sock = -1;
  int ret;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    // ERROR_LOG() << "[SOC] socket error code: " << errno << " msg: " << strerror(errno);
    return sock;
  }
  // printf("ip addr %s\r\n", src_ip);
  char eth_name[32] = {0x0};
  get_eth_name_from_ip(eth_name, src_ip);
  struct ifreq interface;
  strcpy(interface.ifr_name, eth_name);
  if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, &interface, sizeof(interface)) < 0)
  {
    fprintf(stderr,"[setsockopt SO_BINDTODEVICE error] code:'%d' msg:'%s'\n" ,errno, strerror(errno));
  }

  struct sockaddr_in servaddr = {0};
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(dest_port);
  servaddr.sin_addr.s_addr = inet_addr(dest_ip);

  struct timeval tv_send;
  tv_send.tv_sec = 80;
  tv_send.tv_usec = 0;

  struct timeval tv_rcvt;
  tv_rcvt.tv_sec = 30;
  tv_rcvt.tv_usec = 0;

  // 设置发送超时
  if ((ret= setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv_send, sizeof(tv_send))) < 0)
  {
    // ERROR_LOG() << "[SOC] setsockopt SO_SNDTIMEO error code: " << errno << " msg: " << strerror(errno);
    close(sock);
    return ret;
  }

  // 设置接收超时
  if ((ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv_rcvt, sizeof(tv_rcvt))) < 0)
  {
    // ERROR_LOG() << "[SOC] connect SO_RCVTIMEO error code: " << errno << " msg: " << strerror(errno);
    close(sock);
    return ret;
  }

  int flags = fcntl(sock, F_GETFL, 0);

  fcntl(sock, F_SETFL, flags | O_NONBLOCK); // 设置阻塞

  int n = connect(sock, (const struct sockaddr *)&servaddr, sizeof(servaddr)); // 连接服务端

  if (n < 0)
  {
    if (errno != EINPROGRESS && errno != EWOULDBLOCK)
    {
      // ERROR_LOG() << "[SOC] connect error code: " << errno << " msg: " << strerror(errno);
      close(sock);
      return n;
    }
    struct timeval tv;
    fd_set wset;

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    FD_ZERO(&wset);
    FD_SET(sock, &wset); // 把socket添加到select中进行监听

    n = select(sock + 1, NULL, &wset, NULL, &tv);
    if (n < 0)
    {
      // ERROR_LOG() << "[SOC] connect error code: " << errno << " msg: " << strerror(errno);
      close(sock);
      return n; // 出错
    }
    else if (0 == n)
    {
      // ERROR_LOG() << "[SOC] connect timeout error code: " << errno << " msg: " << strerror(errno);
      close(sock);
      return n; // 超时
    }
  }

  fcntl(sock, F_SETFL, flags & ~O_NONBLOCK); // 恢复为阻塞模式

  return sock;
}

ssize_t socket_send(int __fd, const void *__buf, size_t __n, int __flags)
{
  return send(__fd, __buf, __n, __flags);
}

ssize_t socket_recv(int __fd, void *__buf, size_t __n, int __flags)
{
  return recv(__fd, __buf, __n, __flags);
}

int socket_destroy(int sock)
{
  int ret = 0;
  ret = close(sock);
  printf("close soc %d\n", ret);
  return ret;
}