#ifndef ETH_H
#define ETH_H

#include "stdlib.h"
#include <iostream>
#include "string.h"
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;
class eth
{
public:
    eth(string ip, uint16_t remote_port);
    uint8_t receive_ack(void);
    uint8_t send_data(char * buf, uint32_t count);
private:
    void init_send(string ip, uint16_t remote_port);
    void init_receive(void);
    struct sockaddr_in si_local, si_remote;
    int local_socket;
    int send_len;
    void die(string text);
    uint8_t recv_buff[3];
};

#endif // ETH_H
