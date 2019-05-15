/* Copyright (c) 2019
 * Alexander Shirokov
 * Schneider Electric
 * See LICENSE for details. */

#include "tekon/tekon.h"
#include "utils/base/link.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

float u32_to_f32(uint32_t value)
{
    //[-Werror=strict-aliasing]
    float res = 0;
    memcpy(&res, &value, 4);
    return res;
}

void print_parameter(const struct tekon_parameter * parameter)
{
    printf("Device: %d\n",parameter->device);
    printf("Address: %d [0x%x]\n", parameter->address,
           parameter->address);
    printf("UI32: %u\nF32: %f\n",
           parameter->value,
           u32_to_f32(parameter->value));
}

void print_message(const struct message * message)
{
    assert(message);
    printf("Gateway: %d\n", message->gateway);
    printf("Type: %d\n", message->type);
    printf("Dir: %d\n", message->dir);
    switch(message->type) {
    case TEKON_MSG_READEM_PAR_11: {
        printf("Nelem: %d\n", message->nelements);
        print_parameter(message->payload.parameters);
    }
    break;
    case TEKON_MSG_READEM_PAR_LIST_1C: {
        size_t i;
        for(i = 0; i < message->nelements; i++) {
            print_parameter(&message->payload.parameters[i]);
        }
    }

    break;
    default:
        break;
    }
}

void merge_req_resp(struct message * result, const struct message * req, const struct message * resp)
{
    assert(req);
    assert(resp);
    assert(result);
    memcpy(result, resp, sizeof(*result));

    switch(req->type) {
    case TEKON_MSG_READEM_PAR_11:
        result->payload.parameters[0].address = req->payload.parameters[0].address;
        result->payload.parameters[0].device = req->payload.parameters[0].device;
        break;
    case TEKON_MSG_READEM_PAR_LIST_1C: {
        size_t i;
        for(i = 0; i < resp->nelements; i++) {
            result->payload.parameters[i].address = req->payload.parameters[i].address;
            result->payload.parameters[i].device = req->payload.parameters[i].device;
        }
    }
    break;
    default:
        break;
    }
}

void single_reading(struct link * link)
{
    const uint8_t gateway = 2;
    const uint8_t device = 3;
    const uint16_t address = 0x8003;

    const uint8_t onum = rand() % 16;
    uint8_t inum = 0;
    char in[512];
    char out[512];

    struct message request;
    struct message response;

    tekon_req_11(&request, gateway, device, address);
    int result = tekon_req_pack(out, sizeof(out), &request, onum);
    result = link_send(link, out, result);
    printf("Send result %d\n", result);
    if(result <= 0)
        return;

    result = link_recv(link, in, sizeof(in));
    printf("Recv result %d\n", result);
    if( result <= 0)
        return;

    result = tekon_resp_unpack(in, result, &response, TEKON_MSG_READEM_PAR_11, &inum);
    printf("Unpack result %d\n", result);
    if(result <= 0)
        return;

    struct message resmsg;
    merge_req_resp(&resmsg, &request, &response);
    print_message(&resmsg);

}

void multiple_reading(struct link * link)
{
    const uint8_t gateway = 2;
    const uint8_t devices[8] = {3,3,3,3,3,3,3,3};
    const uint16_t addresses[8] = {0x8003,0x8004, 0x8005, 0x8006, 0x8007, 0x8008, 0x8009, 0x800a};

    const uint8_t onum = rand() % 16;
    uint8_t inum = 0;
    char in[512];
    char out[512];

    struct message request;
    struct message response;

    tekon_req_1c(&request, gateway, devices, addresses, NULL, 8);
    int result = tekon_req_pack(out, sizeof(out), &request, onum);
    result = link_send(link, out, result);
    printf("Send result %d\n", result);
    if(result <= 0)
        return;

    result = link_recv(link, in, sizeof(in));
    printf("Recv result %d\n", result);
    if( result <= 0)
        return;

    result = tekon_resp_unpack(in, result, &response, TEKON_MSG_READEM_PAR_LIST_1C, &inum);
    printf("Unpack result %d\n", result);
    if(result <= 0)
        return;

    struct message resmsg;
    merge_req_resp(&resmsg, &request, &response);
    print_message(&resmsg);

}



int main()
{
    const char * ip = "10.0.0.3";
    uint16_t port = 51960;
    const uint16_t timeout = 1000;
    enum link_type type = LINK_UDP;

    struct link link;
    int err = type == LINK_UDP ?
              link_init_udp(&link, ip, port, timeout) :
              link_init_tcp(&link, ip, port, timeout);

    if(err) {
        printf("init error: %d\n", err);
        return err;
    }


    printf("u - up, d - down, s - single read, m - multiple read\n");
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
        case 's':
            single_reading(&link);
            break;
        case 'm':
            multiple_reading(&link);
            break;
        }
    }

    return 0;
}


