#include "crc_32.h"

#define WIDTH (8*sizeof(uint32_t))
crc_32::crc_32()
{

}



void fill_table(void){
    uint32_t reminder=0;

    //compute reminder for each possible dividend.
    for(int dividend = 0; dividend<256; ++dividend){

    }
}
