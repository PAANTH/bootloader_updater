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

/**
* @brief receive configuration initialization
* @param none
* @return none
*/
void eth::init_receive(void){

    if((local_socket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP)) == -1){
        die("Can not create socket");
    }

    memset((char*)&si_local,0,sizeof(si_local));
    si_local.sin_family = AF_INET;
    si_local.sin_port = htons(55556);
    si_local.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(local_socket,(struct sockaddr*)&si_local,sizeof(si_local)) == -1){
        die("Can not bind socket");
    }
    cout<<"init rcv is ok"<<endl;
}

/**
*@brief send configuration initialization
* @param ip - destination ip address
* @param remote_port - destination port
* @return none
*/
void eth::init_send(string ip, uint16_t remote_port){
    send_len = sizeof(si_remote);


    si_remote.sin_family = AF_INET;
    si_remote.sin_port = htons(remote_port);
    inet_pton(AF_INET,(char *)&ip,&(si_remote.sin_addr));


    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int rv = setsockopt(local_socket,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
    if( rv < 0){

        die("Can not set sock opt");
    }


    cout<<"init send is ok"<<endl;
}


/**
* @brief die and exit; call in case of error
* @param text - error message text
* @return none
*/
void eth::die(string text){
    cout<<text<<endl;
    exit(1);
}

/**
* @brief waits for ack from mcu
* @param none
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



/**
* @brief waits for ack from mcu
* @param none
* @returns - 0 if crc ok; 1 if not ok; 2 if error
*/
uint32_t eth::receive_crc(uint32_t original_crc){
    uint32_t recv_crc=0;
    int16_t rec_len=0;
    rec_len = recvfrom(local_socket,recv_buff,sizeof(uint32_t),0,(struct sockaddr *)&si_remote,(socklen_t *)&(send_len));
    if(rec_len != sizeof(uint32_t)){ //if size is ok
        count<<"Received size is not OK"<<endl;
        return 2;
    }
    recv_crc  = *((uint32_t*)recv_buff);
    if(recv_crc != original_crc){
        count<<"Received crc is not OK"<<endl;
        return 1;
    }
    count<<"OK"<<endl;
    return 0;
}


/**
* @brief sends a given amount of data to a ip/port set in launch
* @param buf - uint8_t buffer to a buffer to send
* @param count - amount of bytes to send
* @return ret - amount af bytes succesfully sent; if negative then error
*/
uint8_t eth::send_data(char * buf, uint32_t count){
    int ret = sendto(local_socket,buf,count,0,(struct sockaddr*)&si_remote,sizeof(si_remote));
    if(ret<0){
        cout<<"Send failure"<<endl;
    }
    return ret;
}
