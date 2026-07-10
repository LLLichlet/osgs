/* osgs — disk I/O via BIOS INT 13h. */

#include "disk.h"

int disk_read(uint16_t sector, uint8_t nsec, uint16_t dst_seg, uint16_t dst_off) {
    uint8_t result;
    /* convert LBA to CHS.
       1.44MB floppy: 80 cylinders, 2 heads, 18 sectors per track. */
    uint16_t cyl  = sector / 36;
    uint16_t rem  = sector % 36;
    uint8_t  head = (uint8_t)(rem / 18);
    uint8_t  sec  = (uint8_t)(rem % 18) + 1;

    result = 0;
    __asm {
        push es
        mov  ax, dst_seg
        mov  es, ax
        mov  bx, dst_off
        mov  ah, 02h
        mov  al, nsec
        mov  ch, byte ptr cyl
        mov  cl, sec
        mov  dh, head
        mov  dl, 0
        int  13h
        jc   read_err
        mov  result, 0
        jmp  read_done
    read_err:
        mov  result, ah
    read_done:
        pop  es
    }
    return result;
}
