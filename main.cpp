#include <iostream>
#include "src/http.h"
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char g_host[32];

int main(int, char**){
    // std::cout << "Hello, from main!\n";
    // g_host = "";
    sprintf(g_host,"%s", "192.168.2.4");
    // printf("%s", g_host);
    char msg[] = "hello the client";
    http_send(msg,strlen(msg));
}
