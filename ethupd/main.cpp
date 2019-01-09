#include <iostream>
#include <string.h>
#include "eth.h"
#include <stdio.h>
#include "fcntl.h"
#include "sys/types.h"
#include "unistd.h"

using namespace std;
 uint8_t buf[512];
int main(int argc, char *argv[])
{
    uint8_t ret_val=0;

    string ip;
    uint16_t port = 50000;
    cout << "UPDATER ver.0.1" << endl;
    if(argc != 4){
        cout<<"usage:"<<endl;
        cout<<"     <device ip>  <device port>  <core image path>"<<endl;
        return -1;
    }

    ip = argv[1];
    port = atoi(argv[2]);


    cout<<"device ip: "<<ip<<endl;
    cout<<"device port: "<<port<<endl;
    cout<<"core image path: "<<argv[3]<<endl;

    //try to open file
    int fd = open(argv[3],O_RDONLY);
    if(fd == -1){
        cout<<"file open error"<<endl;
        return -1;
    }

    eth * e = new eth(ip, port);

    buf[0] = 'u';
    buf[1] = 'p';
    buf[2] = 'd';
    if(e->send_data((char *)buf,3) == 3){
        cout<<"upd command send"<<endl;
    }
    ret_val = e->receive_ack();
    if(ret_val){
        cout<<"no ack"<<endl;
        return -1;
    }
    ssize_t res=0;
    while(1){
        res = read(fd,buf,512);

        ret_val = e->send_data(buf,512);
        memset(buf,0,512);
        if(e->receive_ack()){
            cout<<"transmission fuckup, abort..."<<endl;
            //TODO how to implement abort if no connection?
            //maybe add to bootloader a crc check
            return -1;
        }

        if(ret_val < 512){
            break;
        }



    }


    buf[0] = 'c';
    buf[1] = 'r';
    buf[2] = 'c';
    if(e->send_data((char *)buf,3) == 3){
        cout<<"crc calculation command send"<<endl;
    }

    return 0;
}

