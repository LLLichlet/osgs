/* osgs — Table of Contents: maps game names to on-disk locations. */

#include "toc.h"
#include "disk.h"

/* TOC buffer: header (8) + max entries (40 * 50 = 2000) = 2008 bytes */
static uint8_t      toc_buf[2048];
static toc_header  *toc_hdr;
static toc_entry   *toc_entries;
static int          toc_cnt;

int toc_init(void) {
    int err;
    uint16_t ds_seg;

    /* DS = kernel data segment */
    __asm { mov ds_seg, ds }
    err = disk_read(TOC_SECTOR, 4, ds_seg, (uint16_t)(uintptr_t)toc_buf);
    if (err) return err;

    toc_hdr = (toc_header *)toc_buf;

    /* validate magic and version */
    if (toc_hdr->magic != TOC_MAGIC) return -1;
    if (toc_hdr->version != 1)       return -2;

    toc_cnt     = toc_hdr->count;
    toc_entries = (toc_entry *)(toc_buf + sizeof(toc_header));

    if (toc_cnt > TOC_MAX) toc_cnt = TOC_MAX;
    return 0;
}

int toc_count(void) {
    return toc_cnt;
}

const toc_entry *toc_get(int i) {
    if (i < 0 || i >= toc_cnt) return NULL;
    return &toc_entries[i];
}

const toc_entry *toc_find(const char *name) {
    int i;
    for (i = 0; i < toc_cnt; ++i) {
        const char *a = name;
        const char *b = toc_entries[i].name;
        int j;
        /* compare up to 12 chars */
        for (j = 0; j < 12; ++j) {
            if (*a != *b) break;
            if (*a == '\0') return &toc_entries[i];
            ++a; ++b;
        }
    }
    return NULL;
}
