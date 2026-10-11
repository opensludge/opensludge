#!/usr/bin/env python3
"""Make a macOS icon (.icns) from a small XPM icon, such as the 32x32 icon
of a game from its Windows executable.

Usage: xpm-to-icns.py <icon.xpm> <output.icns>

We scale the picture up by whole numbers without smoothing, so pixel art
stays sharp, and make the .icns with iconutil, which is part of macOS.
"""

import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib


def read_xpm(path):
    """Return the width, the height and the rows of RGBA tuples of an XPM
    picture. We read only the colour (c) key of each colour."""
    with open(path, encoding="latin-1") as f:
        strings = re.findall(r'"((?:[^"\\]|\\.)*)"', f.read())
    width, height, colour_count, chars_per_pixel = (int(v) for v in strings[0].split()[:4])

    colours = {}
    for line in strings[1:1 + colour_count]:
        code = line[:chars_per_pixel]
        keys = line[chars_per_pixel:].split()
        value = keys[keys.index("c") + 1] if "c" in keys else keys[-1]
        if value.lower() == "none":
            colours[code] = (0, 0, 0, 0)
        elif value.startswith("#") and len(value) == 7:
            colours[code] = tuple(int(value[i:i + 2], 16) for i in (1, 3, 5)) + (255,)
        elif value.startswith("#") and len(value) == 13:
            colours[code] = tuple(int(value[i:i + 2], 16) for i in (1, 5, 9)) + (255,)
        else:
            sys.exit(f"Unsupported XPM colour: {value}")

    rows = []
    for line in strings[1 + colour_count:1 + colour_count + height]:
        rows.append([colours[line[x * chars_per_pixel:(x + 1) * chars_per_pixel]] for x in range(width)])
    return width, height, rows


def write_png(path, size, rows, source_width, source_height):
    """Write the picture scaled to size x size pixels, with nearest-neighbour
    sampling."""
    raw = bytearray()
    for y in range(size):
        raw.append(0)
        row = rows[y * source_height // size]
        for x in range(size):
            raw.extend(row[x * source_width // size])

    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(bytes(raw), 9)))
        f.write(chunk(b"IEND", b""))


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    source, output = sys.argv[1:]
    width, height, rows = read_xpm(source)

    work = tempfile.mkdtemp()
    try:
        iconset = os.path.join(work, "icon.iconset")
        os.mkdir(iconset)
        for size in (16, 32, 128, 256, 512):
            write_png(os.path.join(iconset, f"icon_{size}x{size}.png"), size, rows, width, height)
            write_png(os.path.join(iconset, f"icon_{size}x{size}@2x.png"), size * 2, rows, width, height)
        subprocess.run(["iconutil", "-c", "icns", "-o", output, iconset], check=True)
    finally:
        shutil.rmtree(work)


if __name__ == "__main__":
    main()
