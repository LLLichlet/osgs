## What's inside

- **TOC (Table of Contents)** — fixed-index file system mapping game names to on-disk locations
  - 50-byte compact entries: name, description, sector, size, flags
  - 40 game slots in 4 sectors (2KB), validated by "OSGS" magic
- **Disk I/O module** — BIOS INT 13h sector reading with LBA-to-CHS conversion
- **Separate game binaries** — games are no longer compiled into the kernel
  - Each game is a standalone raw binary loaded to `0x2000:0x0000`
  - `game_entry.asm` — far-call entry point, segments setup, `retf` return
  - Shared `gamekit` library (vga + keyboard + system) linked into each game
- **Game manager rewrite** — TOC-based `list` and `run` with disk loading
- **Boot sector upgrade** — 63 sectors (31.5KB) loaded across 4 track-safe reads

## Disk layout

```
Sector  0       Boot
Sector  1-63    Kernel (max 31.5KB)
Sector  64-67   TOC (2KB)
Sector  68+     Game binaries (sector-aligned)
```

## Tech

OpenWatcom C (small memory model) + NASM, x86 real mode.
Kernel: 2.7KB. Pong standalone binary: 1.8KB (192 bytes smaller!).
1.44MB floppy image. Room for ~40 games.

## Build

```
python build.py img
python build.py run
```
