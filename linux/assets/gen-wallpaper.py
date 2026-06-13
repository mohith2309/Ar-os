#!/usr/bin/env python3
"""Generate the arOS 3 lavender-glass wallpapers (light + dark/lock)."""
import math, struct, zlib, sys, os

W, H = 2560, 1440

def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))

def blob(buf, cx, cy, r, col, amax):
    r2 = r * r
    x0, x1 = max(0, cx - r), min(W, cx + r)
    y0, y1 = max(0, cy - r), min(H, cy + r)
    for y in range(y0, y1):
        row = y * W * 3
        dy = y - cy
        for x in range(x0, x1):
            dx = x - cx
            d2 = dx * dx + dy * dy
            if d2 >= r2:
                continue
            a = amax * (r2 - d2) / r2
            a = a * (r2 - d2) / r2 / 255.0
            i = row + x * 3
            for k in range(3):
                buf[i + k] = int(buf[i + k] * (1 - a) + col[k] * a)

def make(dark):
    top    = (0x3A, 0x1B, 0x66) if dark else (0xE9, 0xE0, 0xF8)
    bottom = (0x1A, 0x0B, 0x30) if dark else (0xCD, 0xA9, 0xEA)
    buf = bytearray(W * H * 3)
    for y in range(H):
        c = lerp(top, bottom, y / (H - 1))
        row = y * W * 3
        for x in range(W):
            i = row + x * 3
            buf[i], buf[i+1], buf[i+2] = c
    if dark:
        blob(buf, int(W*.5), int(H*.55), int(W*.40), (0x8A,0x4C,0xC8), 230)
        blob(buf, int(W*.30), int(H*.30), int(W*.22), (0xA7,0x66,0xE0), 180)
        blob(buf, int(W*.70), int(H*.72), int(W*.26), (0x6B,0x34,0xA8), 200)
    else:
        blob(buf, int(W*.22), int(H*.72), int(W*.34), (0xC7,0x9C,0xE8), 190)
        blob(buf, int(W*.78), int(H*.20), int(W*.28), (0xF2,0xE8,0xFD), 210)
        blob(buf, int(W*.66), int(H*.86), int(W*.30), (0xDB,0xA6,0xE2), 170)
        blob(buf, int(W*.14), int(H*.18), int(W*.18), (0xF0,0xE4,0xFA), 150)
    return bytes(buf)

def write_png(path, rgb):
    def chunk(tag, data):
        c = tag + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xffffffff)
    raw = bytearray()
    for y in range(H):
        raw.append(0)
        raw += rgb[y*W*3:(y+1)*W*3]
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", W, H, 8, 2, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 6))
    png += chunk(b"IEND", b"")
    open(path, "wb").write(png)

if __name__ == "__main__":
    d = os.path.dirname(os.path.abspath(__file__))
    write_png(os.path.join(d, "wallpaper.png"), make(False))
    write_png(os.path.join(d, "wallpaper-dark.png"), make(True))
    print("wrote wallpaper.png + wallpaper-dark.png")
