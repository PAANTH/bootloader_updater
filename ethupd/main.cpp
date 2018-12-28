#include <iostream>
#include <string.h>
#include "eth.h"
using namespace std;

int main(int argc, char *argv[])
{
    string path;
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
    path = argv[3];

    cout<<"device ip: "<<ip<<endl;
    cout<<"device port: "<<port<<endl;
    cout<<"core image path: "<<path<<endl;

    eth * e = new eth(ip, port);
//    if(e->receive_ack() == 0){
//        cout<<"ACK RECEIVED"<<endl;
//    }
    uint8_t buf[5];
    memset(buf,0xFA,sizeof(buf));
    if(e->send_data((char *)buf,5) == 5){
        cout<<"Data send"<<endl;
    }

    cout<<"all"<<endl;


    return 0;
}
