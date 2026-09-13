#!/usr/bin/env python3

def tiled_index(cw, x, y):
    return (((y >> 5) * 32 * cw + ((x >> 5) << 10)
             + (x & 3) + ((y & 1) << 2)
             + (((x & 31) >> 2) << 3)
             + (((y & 31) >> 1) << 6))
            ^ ((y & 8) << 2))


def verify(width, height):
    cw = (width + 31) & ~31
    ch = (height + 31) & ~31
    indices = [tiled_index(cw, x, y)
               for y in range(height)
               for x in range(width)]
    assert len(indices) == width * height
    assert len(set(indices)) == width * height, (width, height, "collision")
    assert min(indices) >= 0
    assert max(indices) < cw * ch, (width, height, max(indices), cw * ch)


def main():
    for mode in ((640, 480), (1280, 720), (1360, 768), (1920, 1080)):
        verify(*mode)
    print("XEX framebuffer tiling: PASS")


if __name__ == "__main__":
    main()
