import subprocess
import sys
import os

ROOT = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(ROOT, "src")
BUILD = os.path.join(ROOT, "build")


def run(cmd, *, check=True):
    print(f"\033[36m>>> {' '.join(cmd)}\033[0m")
    subprocess.run(cmd, check=check)


def build():
    os.makedirs(BUILD, exist_ok=True)

    # assemble boot sector -> flat binary
    run(["nasm", "-f", "bin", os.path.join(SRC, "boot.asm"),
         "-o", os.path.join(BUILD, "boot.bin")])

    # assemble kernel entry -> flat binary
    run(["nasm", "-f", "bin", os.path.join(SRC, "entry.asm"),
         "-o", os.path.join(BUILD, "kernel.bin")])

    print("Build OK")


def img():
    build()

    boot_bin = os.path.join(BUILD, "boot.bin")
    kernel_bin = os.path.join(BUILD, "kernel.bin")
    img_file = os.path.join(BUILD, "osgs.img")

    with open(img_file, "wb") as img:
        # sector 0: boot sector
        with open(boot_bin, "rb") as f:
            data = f.read()
            assert len(data) == 512, f"boot.bin must be 512 bytes, got {len(data)}"
            img.write(data)

        # sector 1+: kernel
        with open(kernel_bin, "rb") as f:
            img.write(f.read())

        # pad to 1.44MB floppy
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
