#ifndef CRC_32_H
#define CRC_32_H
#include "stdint.h"

class crc_32
{
public:
    uint32_t crc_table[256];
    crc_32();
    void fill_table(void);
    void get_crc(uint32_t* buff, int len);
};

#endif // CRC_32_H
