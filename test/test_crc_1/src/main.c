#include <stdio.h>
#include <stdint.h>
#include "crc8.h"
#include <string.h>

// msg buffer size should be big enough for another crc byte
// returns new msg size
size_t append_crc_to_msg(char *msg, size_t msg_size, size_t buf_size, uint8_t crc)
{
    if (msg_size + 1 >= buf_size) return -1;
    msg[msg_size++] = crc;
    msg[msg_size] = '\0';
    return msg_size;
}

int main(void)
{
    int size_bytes;
    scanf("%512s", res);
    
    for (size_bytes = 0; res[size_bytes] != '\0'; size_bytes++)
    {
        printf("%x ", res[size_bytes]);
        printf("%c \n", res[size_bytes]);
    }

    uint8_t crc = crc8_calc((uint8_t *)res, size_bytes); 

    printf("\n%x\n", crc);

    size_bytes = append_crc_to_msg(res, size_bytes, 512, crc);

    printf("%s\n", res);

    // calculate crc for combined message (expect 0)
    uint8_t crc_rx = crc8_calc((uint8_t *)res, size_bytes); 

    printf("%d\n", crc_rx);


    return 0;
}
