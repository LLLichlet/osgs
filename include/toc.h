/* osgs — Table of Contents: maps game names to on-disk locations. */

#ifndef TOC_H
#define TOC_H

#include "kernel.h"

#define TOC_SECTOR 64
#define TOC_MAGIC  0x5347534F  /* "OSGS" */
#define TOC_MAX    40

typedef struct {
    char     name[12];    /* game name, null-padded */
    char     desc[32];    /* short description, null-terminated */
    uint16_t sector;      /* starting LBA sector */
    uint8_t  nsectors;    /* sectors on disk */
    uint8_t  flags;       /* bit0 = compressed, bit1-7 reserved */
    uint16_t mem_sz;      /* uncompressed size in bytes */
} toc_entry;              /* 50 bytes */

typedef struct {
    uint32_t   magic;
    uint8_t    version;
    uint8_t    count;
    uint8_t    reserved[2];
    /* toc_entry entries[count] follows in memory */
} toc_header;             /* 8 bytes */

/* load TOC from disk into a static buffer, validate magic.
   returns 0 on success, non-zero on error. */
int  toc_init(void);

/* returns number of game entries. */
int  toc_count(void);

/* get pointer to i-th entry (0 <= i < count). */
const toc_entry *toc_get(int i);

/* find entry by name, returns NULL if not found. */
const toc_entry *toc_find(const char *name);

#endif
