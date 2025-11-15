#!/usr/bin/env python3
"""
Simple FAT16 image generator for testing USB MSC in QEMU.
Creates build/usb.img (16 MiB) and writes a single file /HELLO.TXT with test content.
This is minimal and intended for emulator testing only.
"""
import os
import struct

OUT_PATH = os.path.join(os.path.dirname(__file__), '..', 'build', 'usb.img')
IMAGE_SIZE = 16 * 1024 * 1024  # 16 MiB
BYTES_PER_SECTOR = 512
RESERVED_SECTORS = 1
NUM_FATS = 2
ROOT_ENTRY_COUNT = 512  # number of 32-byte root dir entries
MEDIA_BYTE = 0xF8
SECTORS_PER_TRACK = 63
NUM_HEADS = 255
HIDDEN_SECTORS = 0
VOLUME_LABEL = b'NO NAME    '
FS_TYPE = b'FAT16   '

def ceildiv(a, b):
    return (a + b - 1) // b


def choose_geometry(total_sectors):
    # Try a reasonable sectors-per-cluster for 16MiB image
    for spc in (1,2,4,8,16,32,64):
        sectors_per_fat = 1
        while True:
            root_dir_sectors = ceildiv(ROOT_ENTRY_COUNT * 32, BYTES_PER_SECTOR)
            data_sectors = total_sectors - RESERVED_SECTORS - (NUM_FATS * sectors_per_fat) - root_dir_sectors
            if data_sectors <= 0:
                break
            clusters = data_sectors // spc
            if clusters < 4085:
                # FAT12 region; try smaller spc (we want FAT16)
                break
            needed_spf = ceildiv((clusters + 2) * 2, BYTES_PER_SECTOR)
            if needed_spf == sectors_per_fat:
                # stable
                return spc, sectors_per_fat, root_dir_sectors, clusters
            sectors_per_fat = needed_spf
            # prevent infinite loop
            if sectors_per_fat > total_sectors:
                break
    raise RuntimeError('Failed to pick FAT16 geometry')


def build_bpb(total_sectors, spc, spf, root_dir_sectors):
    # Construct BIOS Parameter Block for FAT16
    bpb = bytearray(512)
    # Jump instruction + NOP
    bpb[0:3] = b"\xEB\x3C\x90"
    bpb[3:11] = b"mkfat16 "
    struct.pack_into('<H', bpb, 11, BYTES_PER_SECTOR)
    bpb[13] = spc
    struct.pack_into('<H', bpb, 14, RESERVED_SECTORS)
    bpb[16] = NUM_FATS
    struct.pack_into('<H', bpb, 17, ROOT_ENTRY_COUNT)
    # total sectors (16-bit) set to 0 if using 32-bit field
    struct.pack_into('<H', bpb, 19, 0)
    bpb[21] = MEDIA_BYTE
    struct.pack_into('<H', bpb, 22, spf)
    struct.pack_into('<H', bpb, 24, SECTORS_PER_TRACK)
    struct.pack_into('<H', bpb, 26, NUM_HEADS)
    struct.pack_into('<I', bpb, 28, HIDDEN_SECTORS)
    struct.pack_into('<I', bpb, 32, total_sectors)
    # FAT16 Extended BPB
    bpb[36] = 0  # drive number (unused)
    bpb[37] = 0  # reserved
    bpb[38] = 0x29  # boot signature
    struct.pack_into('<I', bpb, 39, 0x12345678)  # volume id
    bpb[43:54] = VOLUME_LABEL
    bpb[54:62] = FS_TYPE
    # boot sector signature
    bpb[510:512] = b"\x55\xAA"
    return bpb


def make_image():
    os.makedirs(os.path.join(os.path.dirname(__file__), '..', 'build'), exist_ok=True)
    total_sectors = IMAGE_SIZE // BYTES_PER_SECTOR
    spc, spf, root_dir_sectors, clusters = choose_geometry(total_sectors)
    print(f'Chosen: sectors/cluster={spc}, sectors/FAT={spf}, root_dir_sectors={root_dir_sectors}, clusters={clusters}')

    bpb = build_bpb(total_sectors, spc, spf, root_dir_sectors)

    img = bytearray(IMAGE_SIZE)
    # write boot sector
    img[0:512] = bpb

    fat0_offset = RESERVED_SECTORS * BYTES_PER_SECTOR
    fat_size_bytes = spf * BYTES_PER_SECTOR
    # Initialize FATs
    # FAT is little-endian 16-bit entries
    # Set first two reserved entries
    # FAT[0] = media descriptor | 0xFF00; FAT[1] = 0xFFFF
    struct.pack_into('<H', img, fat0_offset + 0, 0xFFF8 | (MEDIA_BYTE & 0xFF))
    struct.pack_into('<H', img, fat0_offset + 2, 0xFFFF)
    # Mark cluster 2 as EOF (for one small file)
    struct.pack_into('<H', img, fat0_offset + 4, 0xFFFF)
    # Mirror into FAT #2
    fat1_offset = fat0_offset + fat_size_bytes
    img[fat1_offset:fat1_offset+fat_size_bytes] = img[fat0_offset:fat0_offset+fat_size_bytes]

    # Root dir offset
    root_dir_offset = fat1_offset + fat_size_bytes
    # Create a single file HELLO.TXT in root
    filename = b'HELLO   TXT'  # 8.3 name (padded)
    attr = 0x20  # archive
    first_cluster = 2
    content = b'Hello from Quantum-OS!\n'
    filesize = len(content)
    import time
    ctime = int(time.time()) & 0xFFFFFFFF
    # Directory entry (32 bytes)
    de = bytearray(32)
    de[0:11] = filename
    de[11] = attr
    # time/date fields left zero (simpler)
    struct.pack_into('<H', de, 26, first_cluster)
    struct.pack_into('<I', de, 28, filesize)
    img[root_dir_offset:root_dir_offset+32] = de

    # Data region offset
    data_offset = root_dir_offset + (root_dir_sectors * BYTES_PER_SECTOR)
    cluster2_offset = data_offset + (first_cluster - 2) * spc * BYTES_PER_SECTOR
    img[cluster2_offset:cluster2_offset+filesize] = content

    # Write image to disk
    with open(OUT_PATH, 'wb') as f:
        f.write(img)
    print(f'Wrote {OUT_PATH} ({len(img)} bytes)')


if __name__ == '__main__':
    make_image()
