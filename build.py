import subprocess
import sys
import os
import struct

ROOT = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(ROOT, "src")
INC = os.path.join(ROOT, "include")
BUILD = os.path.join(ROOT, "build")

WCC_FLAGS = ["-q", "-ms", "-s", "-os", "-0", "-i=" + INC]

# kernel C sources (without game code - games are standalone binaries)
KERNEL_C = ["kernel.c", "vga.c", "keyboard.c", "shell.c",
            "system.c", "game.c", "disk.c", "toc.c"]

# gamekit C sources (linked into each game binary)
GAMEKIT_C = ["vga.c", "keyboard.c", "system.c"]

# games: {name: description}
GAMES = {
    "pong": "Classic Pong - 2 Player",
}

# disk layout constants
SECTOR_SZ     = 512
TOC_SECTOR    = 64          # TOC starts at sector 64
KERNEL_SECTORS = 63         # kernel: sectors 1-63 (32KB - 512B)
TOC_SECTORS   = 4           # TOC: 4 sectors (2048 bytes)
GAME_START    = TOC_SECTOR + TOC_SECTORS  # 68

def run(cmd, *, check=True):
    print(f"\033[36m>>> {' '.join(cmd)}\033[0m")
    subprocess.run(cmd, check=check)


def compile_c(src_rel, obj_rel):
    """Compile a C source file to OMF .obj."""
    src_path = os.path.join(SRC, src_rel)
    obj_path = os.path.join(BUILD, obj_rel)
    os.makedirs(os.path.dirname(obj_path), exist_ok=True)
    run(["wcc", *WCC_FLAGS, "-fo=" + obj_path, src_path])
    return obj_path


def build_boot():
    """Assemble boot sector -> flat binary."""
    boot_bin = os.path.join(BUILD, "boot.bin")
    run(["nasm", "-f", "bin",
         os.path.join(SRC, "boot.asm"),
         "-o", boot_bin])
    return boot_bin


def build_kernel():
    """Compile and link kernel -> kernel.bin. Returns path and size."""
    # entry.asm -> entry.obj
    entry_obj = os.path.join(BUILD, "entry.obj")
    run(["nasm", "-f", "obj",
         os.path.join(SRC, "entry.asm"),
         "-o", entry_obj])

    objs = [entry_obj]
    for src in KERNEL_C:
        obj = compile_c(src, src.replace(".c", ".obj"))
        objs.append(obj)

    kernel_bin = os.path.join(BUILD, "kernel.bin")
    linker_args = [
        "wlink",
        "OPTION", "QUIET",
        "FORMAT", "RAW", "BIN",
        "NAME", kernel_bin,
        "FILE", ",".join(objs),
    ]
    run(linker_args)

    size = os.path.getsize(kernel_bin)
    max_sz = KERNEL_SECTORS * SECTOR_SZ
    if size > max_sz:
        print(f"ERROR: kernel too large ({size} > {max_sz} bytes)")
        sys.exit(1)
    print(f"Kernel: {size} bytes ({size / SECTOR_SZ:.1f} sectors)")
    return kernel_bin


def build_game(game_name, gamekit_objs):
    """Compile and link a single game -> .bin. Returns path and size."""
    # compile game source
    game_obj = compile_c(
        os.path.join("games", game_name + ".c"),
        os.path.join("games", game_name + ".obj"))

    # game_entry.asm -> .obj (shared, compile once if not exists)
    entry_obj = os.path.join(BUILD, "game_entry.obj")
    if not os.path.exists(entry_obj):
        run(["nasm", "-f", "obj",
             os.path.join(SRC, "game_entry.asm"),
             "-o", entry_obj])

    # link: entry first -> _start at offset 0
    game_bin = os.path.join(BUILD, "games", game_name + ".bin")
    os.makedirs(os.path.dirname(game_bin), exist_ok=True)
    objs = [entry_obj] + gamekit_objs + [game_obj]
    linker_args = [
        "wlink",
        "OPTION", "QUIET",
        "FORMAT", "RAW", "BIN",
        "NAME", game_bin,
        "FILE", ",".join(objs),
    ]
    run(linker_args)

    size = os.path.getsize(game_bin)
    print(f"  {game_name}: {size} bytes ({size / SECTOR_SZ:.1f} sectors)")
    return game_bin, size


def build_toc(games_info):
    """Generate TOC binary from game info list.
    Each entry: (name, desc, sector, nsectors, flags, mem_sz)
    Returns path to toc.bin."""
    toc_bin = os.path.join(BUILD, "toc.bin")

    with open(toc_bin, "wb") as f:
        # header
        f.write(b"OSGS")                  # magic
        f.write(bytes([1]))               # version
        f.write(bytes([len(games_info)])) # entry count
        f.write(bytes([0, 0]))            # reserved

        for name, desc, sector, nsectors, flags, mem_sz in games_info:
            name_bytes = name.encode("ascii").ljust(12, b"\0")[:12]
            desc_bytes = desc.encode("ascii").ljust(32, b"\0")[:32]
            f.write(name_bytes)
            f.write(desc_bytes)
            f.write(struct.pack("<H", sector))
            f.write(bytes([nsectors]))
            f.write(bytes([flags]))
            f.write(struct.pack("<H", mem_sz))

    toc_sz = os.path.getsize(toc_bin)
    max_sz = TOC_SECTORS * SECTOR_SZ
    if toc_sz > max_sz:
        print(f"ERROR: TOC too large ({toc_sz} > {max_sz} bytes)")
        sys.exit(1)
    print(f"TOC: {toc_sz} bytes, {len(games_info)} entries")
    return toc_bin


def build():
    """Compile all binaries."""
    os.makedirs(BUILD, exist_ok=True)

    # 1. boot sector
    build_boot()

    # 2. kernel
    build_kernel()

    # 3. gamekit objects (compiled once, linked into each game)
    gamekit_objs = [compile_c(src, "gamekit_" + src.replace(".c", ".obj"))
                    for src in GAMEKIT_C]

    # 4. compile each game
    games_info = []
    current_sector = GAME_START
    for name in GAMES:
        game_bin, size = build_game(name, gamekit_objs)
        nsectors = (size + SECTOR_SZ - 1) // SECTOR_SZ
        games_info.append((name, GAMES[name], current_sector,
                           nsectors, 0, size))
        current_sector += nsectors

    # 5. TOC
    build_toc(games_info)

    print("Build OK")


def img():
    """Build disk image: boot + kernel + TOC + games + pad."""
    build()

    boot_bin  = os.path.join(BUILD, "boot.bin")
    kernel_bin = os.path.join(BUILD, "kernel.bin")
    toc_bin    = os.path.join(BUILD, "toc.bin")
    img_file   = os.path.join(BUILD, "osgs.img")

    with open(img_file, "wb") as img:
        # sector 0: boot
        with open(boot_bin, "rb") as f:
            data = f.read()
            assert len(data) == 512, \
                f"boot.bin must be 512 bytes, got {len(data)}"
            img.write(data)

        # sectors 1-63: kernel (zero-pad to 32KB boundary)
        with open(kernel_bin, "rb") as f:
            img.write(f.read())
        pos = img.tell()
        target = (TOC_SECTOR) * SECTOR_SZ
        if pos < target:
            img.write(b"\x00" * (target - pos))

        # sectors 64-67: TOC (zero-pad to 4-sector boundary)
        with open(toc_bin, "rb") as f:
            img.write(f.read())
        pos = img.tell()
        target = GAME_START * SECTOR_SZ
        if pos < target:
            img.write(b"\x00" * (target - pos))

        # sectors 68+: game binaries (each sector-aligned)
        for name in GAMES:
            game_bin = os.path.join(BUILD, "games", name + ".bin")
            with open(game_bin, "rb") as f:
                img.write(f.read())
            # sector-align
            pos = img.tell()
            remainder = pos % SECTOR_SZ
            if remainder:
                img.write(b"\x00" * (SECTOR_SZ - remainder))

        # pad to 1.44MB
        total = img.tell()
        remaining = 1474560 - total
        if remaining > 0:
            img.write(b"\x00" * remaining)

    size = os.path.getsize(img_file)
    print(f"Image: {img_file} ({size} bytes = {size / 1024:.1f} KB)")


def run_qemu():
    img()
    img_file = os.path.join(BUILD, "osgs.img")
    run(["qemu-system-i386", "-drive",
         f"file={img_file},format=raw,if=floppy,index=0", "-boot", "a"])


def clean():
    import shutil
    if os.path.exists(BUILD):
        shutil.rmtree(BUILD)
    print("Cleaned")


def main():
    if len(sys.argv) < 2:
        build()
        return

    cmd = sys.argv[1]
    if cmd == "build":
        build()
    elif cmd == "img":
        img()
    elif cmd == "run":
        run_qemu()
    elif cmd == "clean":
        clean()
    else:
        print(f"unknown: {cmd}")
        sys.exit(1)


if __name__ == "__main__":
    main()
