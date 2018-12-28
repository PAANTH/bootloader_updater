#include "eth.h"
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>

#define BUFFLEN 3
eth::eth(string ip, uint16_t remote_port)
{
    init_receive();
    init_send(ip, remote_port);

}


void eth::init_receive(){

    if((local_socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1){
        die("Can not create socket");
    }

    memset((char*)&si_local,0,sizeof(si_local));
    si_local.sin_family = AF_INET;
    si_local.sin_port = htons(55556);
    si_local.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(local_socket,(struct sockaddr*)&si_local,sizeof(si_local)) == -1){
        //cout<<errno<<endl;
        die("Can not bind socket");
    }
    cout<<"init rcv is ok"<<endl;
}


void eth::init_send(string ip, uint16_t remote_port){
    send_len = sizeof(si_remote);


    si_remote.sin_family = AF_INET;
    si_remote.sin_port = htons(remote_port);
    inet_pton(AF_INET,(char *)&ip,&(si_remote.sin_addr));


    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    cout<<errno<<endl;
    int rv = setsockopt(local_socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    if( rv < 0){
        //cout<<rv<<endl;
        //cout<<errno<<endl;
        die("Can not set sock opt");
    }


    cout<<"init send is ok"<<endl;
}

void eth::die(string text){
    cout<<text<<endl;
    exit(1);
}

/**
* @returns - 0 if ack; 1 if nack; 2 if error
*/
uint8_t eth::receive_ack(void){
    uint8_t ret_val=0;
    int16_t rec_len=0;
    rec_len = recvfrom(local_socket,recv_buff,BUFFLEN,0,(struct sockaddr *)&si_remote,(socklen_t *)&(send_len));
    if(rec_len == 3){ //if size is ok
        if((recv_buff[0]==0xAA)&&(recv_buff[2]==0xAA)&&(recv_buff[3]==0xAA)){
            ret_val = 0;
        }
        else if((recv_buff[0]==0xBB)&&(recv_buff[2]==0xBB)&&(recv_buff[3]==0xBB)){
            ret_val = 1;
        }
    }
    else{ //else error
        ret_val = 2;
    }
    return ret_val;
}

uint8_t eth::send_data(char * buf, uint32_t count){
    int ret = sendto(local_socket,buf,count,0,(struct sockaddr*)&si_remote,sizeof(si_remote));
    if(ret<0){
        cout<<"Send failure"<<endl;
    }
}
