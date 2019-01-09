#include "crc_32.h"
#include <stdio.h>
#include <iostream>
#include <iomanip>
using namespace std;
#define WIDTH (8*sizeof(uint32_t))
#define TOPBIT (1<<(WIDTH-1))
#define POLINOMIAL 0x4C11DB7
crc_32::crc_32()
{

}



void crc_32::fill_table(void){
    uint32_t reminder=0;

    //compute reminder for each possible dividend.
    for(int dividend = 0; dividend<256; ++dividend){
        //start with the dividend followed by zeros.
        reminder = dividend << (WIDTH-8);

        //perform modulo-2 devision, a bit at a time
        for(uint8_t bit = 8; bit > 0; --bit){
            if(reminder & TOPBIT){
                reminder = (reminder<<1)^ POLINOMIAL;
            }
            else{
                reminder = (reminder<<1);
            }
        }
        crc_table[dividend] = reminder;
    }

    ////print
    for(int i=0; i<256; i++){
        cout<<setfill('0')<<std::setw(8)<<std::hex<<crc_table[i]<<", ";
        if(i%4==0){
            cout<<endl;
        }
    }



}
