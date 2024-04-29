#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
   int http_send(const char *msg, size_t msg_len);
#ifdef __cplusplus
}
#endif /* __cplusplus */