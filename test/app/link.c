/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#include "utils/base/link.h"
#include <stdio.h>
#include <string.h>

void usage()
{
    printf("app_link tcp|udp ip\n");
}

int main(int argc, const char * argv[])
{
    int mode = LINK_UDP;
    const char * ip = "127.0.0.1";

    if (argc == 1) {
        usage();
        return 0;
    }
    if (argc > 1) {
        if(strcmp(argv[1], "tcp") == 0) {
            mode = LINK_TCP;
        } else if(strcmp(argv[1], "udp") == 0) {
            mode = LINK_UDP;
        } else {
            printf("Wrong mode\n");
            usage();
            return 1;
        }
    }
    if (argc > 2) {
        ip = argv[2];
    }

    const uint16_t port = 8888;
    const uint16_t timeout = 5000;

    const char * hello = "Hello\n";
    const size_t hello_len = strlen(hello);

    char buffer[256];

    struct link link;

    int err = mode == LINK_UDP ?
              link_init_udp(&link, ip, port, timeout) :
              link_init_tcp(&link, ip, port, timeout);
    int result;
    if(err) {
        printf("init error: %d\n", err);
        return err;
    }

    printf("u - up, d - down, w - write, r - read\n");
    int c;

    while((c=getchar()) != 'q') {
        switch(c) {
        case 'u':
            err = link_up(&link);
            if(err)
                printf("up error: %d\n", err);
            break;
        case 'd':
            link_down(&link);
            break;
        case 'r':
            result = link_recv(&link, buffer, sizeof(buffer));
            printf("read result: %d\n", result);
            break;
        case 'w':
            result = link_send(&link, hello, hello_len);
            if(result <= 0)
                printf("write result: %d\n", result);
            break;
        }
    }
    return 0;
}
