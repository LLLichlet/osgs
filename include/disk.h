/* osgs — disk I/O via BIOS INT 13h. */

#ifndef DISK_H
#define DISK_H

#include "kernel.h"

#define DISK_SECTOR_SZ 512

/* read sectors from floppy disk 0 into memory.
   sector: starting LBA sector
   count:  number of sectors to read (1-255)
   seg:    destination segment
   off:    destination offset
   returns 0 on success, non-zero on error. */
int disk_read(uint16_t sector, uint8_t count, uint16_t seg, uint16_t off);

#endif
