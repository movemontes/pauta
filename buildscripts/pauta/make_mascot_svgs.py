"""Pauta: cut the mascot character sheet (4 x 3 poses) into separate, clean SVG files.
Steps per pose: crop, snap every pixel to black / white / blue, make the outside transparent,
trace with vtracer, and use exactly #0000FF for the blue so the app can swap it for the accent color.
Usage: venv/bin/python make_mascot_svgs.py <sheet image> <out dir>"""
import os
import re
import sys
import tempfile

import numpy as np
import vtracer
from PIL import Image, ImageDraw

NAMES = ["normal", "oi", "aponta-esquerda", "aponta-direita",
         "pensando", "surpreso", "comemorando", "triste",
         "bravo", "regendo", "dormindo", "rosto"]
INK, WHITE, BLUE = (10, 10, 10), (255, 255, 255), (0, 0, 255)

src, out = sys.argv[1], sys.argv[2]
sheet = Image.open(src).convert("RGB")
W, H = sheet.size
cw, ch = W // 4, H // 3


def snap(img):
    a = np.asarray(img).astype(int)
    r, g, b = a[..., 0], a[..., 1], a[..., 2]
    res = np.zeros_like(a)
    blue = (b - np.maximum(r, g) > 60)
    dark = (~blue) & ((r + g + b) < 300)
    res[...] = WHITE
    res[dark] = INK
    res[blue] = BLUE
    return Image.fromarray(res.astype("uint8"))


for i, name in enumerate(NAMES):
    x, y = (i % 4) * cw, (i // 4) * ch
    cell = sheet.crop((x, y, x + cw, y + ch))
    cell = cell.resize((cw * 2, ch * 2), Image.LANCZOS)
    cell = snap(cell)
    # tight crop to the drawing, with a margin
    arr = np.asarray(cell)
    ys, xs = np.where((arr != 255).any(axis=2))
    m = 12
    box = (max(xs.min() - m, 0), max(ys.min() - m, 0), min(xs.max() + m, cell.width), min(ys.max() + m, cell.height))
    cell = cell.crop(box)
    # outside -> transparent (flood from the border)
    rgba = cell.convert("RGBA")
    marker = (255, 0, 255, 255)
    w, h = rgba.size
    for px, py in [(0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)] + [(k, 0) for k in range(0, w, 20)] + [(k, h - 1) for k in range(0, w, 20)] \
            + [(0, k) for k in range(0, h, 20)] + [(w - 1, k) for k in range(0, h, 20)]:
        if rgba.getpixel((px, py)) == (255, 255, 255, 255):
            ImageDraw.floodfill(rgba, (px, py), marker)
    a = np.asarray(rgba).copy()
    bg = (a[..., 0] == 255) & (a[..., 1] == 0) & (a[..., 2] == 255)
    a[bg] = (0, 0, 0, 0)
    rgba = Image.fromarray(a)
    with tempfile.TemporaryDirectory() as tmp:
        png = os.path.join(tmp, "p.png")
        rgba.save(png)
        svg = os.path.join(tmp, "p.svg")
        vtracer.convert_image_to_svg_py(png, svg, colormode="color", hierarchical="stacked", mode="spline",
                                        filter_speckle=8, color_precision=6, layer_difference=48,
                                        corner_threshold=60, length_threshold=4.0, splice_threshold=45, path_precision=2)
        text = open(svg).read()

    def fix(m):
        c = m.group(1).lstrip("#")
        rgb = tuple(int(c[k:k + 2], 16) for k in (0, 2, 4))
        if rgb[2] - max(rgb[0], rgb[1]) > 60:
            return 'fill="#0000FF"'
        if sum(rgb) < 300:
            return 'fill="#0A0A0A"'
        return 'fill="#FFFFFF"'
    text = re.sub(r'fill="(#[0-9A-Fa-f]{6})"', fix, text)
    # scalable: viewBox from the pixel size
    text = re.sub(r'<svg([^>]*?)width="(\d+)" height="(\d+)"', r'<svg\1viewBox="0 0 \2 \3"', text, count=1)
    open(os.path.join(out, f"mascote-{name}.svg"), "w").write(text)
    print(name, len(text) // 1024, "KB", rgba.size)
