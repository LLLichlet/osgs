/* osgs — kernel core types and global constants. */

#ifndef KERNEL_H
#define KERNEL_H

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long  uint32_t;
typedef signed char    int8_t;
typedef signed short   int16_t;
typedef signed long    int32_t;

#define NULL  ((void *)0)
#define true  1
#define false 0
typedef uint8_t bool;

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_COLS    80
#define VGA_ROWS    25

#endif
