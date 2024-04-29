#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "soc.h"
#include "http.h"

extern char g_host[32];

// 1024
#define READ_BUF_SIZE 1024
static char *g_read_buf = NULL;

// 1024 * 1024 * 32 + 512
#define HEADER_BUF_SIZE 33554944
static char *g_header_buf = NULL;

// 1024 * 1024 * 32
#define BODY_BUF_SIZE 33554432

static char *g_body_buf = NULL;

int http_send(const char *msg, size_t msg_len)
{
    int ret = 0;

    if (msg == NULL || msg_len == 0)
    {
        return 0;
    }

    if (g_header_buf == NULL)
    {
        g_header_buf = (char *)malloc(HEADER_BUF_SIZE);
    }

    if (g_body_buf == NULL)
    {
        g_body_buf = (char *)malloc(BODY_BUF_SIZE);
    }


    memset(g_header_buf, 0x0, HEADER_BUF_SIZE);

    memset(g_body_buf, 0x0, BODY_BUF_SIZE);
    // INFO_LOG() << alarmTime << " ==> " << algCode;
    sprintf(g_body_buf, "{\"msg\":\"%s\"}", msg);

    char *header = g_header_buf;

    strcat(header, "POST /api/smartbox/AlarmPos HTTP/1.1\r\n");
    strcat(header, "Cache-Control:no-cache\r\n");
    strcat(header, "Connection:close\r\n");
    strcat(header, "Accept-Encoding:gzip,deflate,br\r\n");
    strcat(header, "Accept:*/*\r\n");
    strcat(header, "Content-Type:application/json\r\n");
    strcat(header, "User-Agent:Mozilla/5.0\r\n");
    
    // host 一定需要
    strcat(header, "Host: ");
    strcat(header,"192.168.2.4");
    strcat(header, "\r\n");

    strcat(header, "Content-Length:");
    
    char cl[32] = {0};
    sprintf(cl, "%ld", strlen(g_body_buf));
    
    strcat(header, cl);
    strcat(header, "\r\n\r\n");

    strcat(header, g_body_buf);
    strcat(header, "\r\n\r\n");

    int fd = socket_create("eth1", g_host, 7890, "192.168.2.78", 8080);

    if (fd < 0)
    {
        // ERROR_LOG() << "[HTTP] soc 创建失败: " << fd;
        printf("soc 创建失败\n");
        return -1;
    }

    // INFO_LOG() << "[HTTP] http buf (" << strlen(header) <<")";
    printf("http buf %d\r\n", strlen(header));
    ret = socket_send(fd, header, strlen(header), 0);
    printf("http buf ret %d\r\n", ret);
    if (ret != strlen(header))
    {
        // ERROR_LOG() << "[HTTP] 数据发送失败: " << ret ;
        printf("数据发送失败\n");
        return -1;
    }
    else
    {
        printf("数据发送成功\n");
    }

    if (g_read_buf == NULL)
    {
        g_read_buf = (char *)malloc(READ_BUF_SIZE);
    }

    memset(g_read_buf, 0x0, READ_BUF_SIZE);

    ret = socket_recv(fd, g_read_buf,READ_BUF_SIZE,0);

    // INFO_LOG() << "[HTTP] 接收的消息 (" << ret << ")" << "\n" << g_read_buf;
    socket_destroy(fd);

    return 0;
}