"""Pauta app icon with the mascot face (Treco look: cream tile, ink outline, hard shadow).
Usage (system python3 with PIL): python3 make_icon_mascot.py <mascot face png, 1024, white background> <out png>
The face PNG comes from: qlmanage -t -s 1024 -o <dir> src/appshell/resources/mascote/mascote-rosto.svg"""
import sys
from PIL import Image, ImageDraw

face = Image.open(sys.argv[1]).convert("RGBA")
# white outside -> transparent
w, h = face.size
marker = (255, 0, 255, 255)
for p in [(0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)]:
    if face.getpixel(p)[:3] == (255, 255, 255):
        ImageDraw.floodfill(face, p, marker, thresh=40)
px = face.load()
for y in range(h):
    for x in range(w):
        if px[x, y] == marker:
            px[x, y] = (0, 0, 0, 0)
face = face.crop(face.getbbox())

S = 1024
icon = Image.new("RGBA", (S, S), (0, 0, 0, 0))
d = ImageDraw.Draw(icon)
m, r, sh, bw = 92, 180, 28, 30  # macOS-like margin, corner radius, shadow, border
d.rounded_rectangle([m + sh, m + sh, S - m + sh - 16, S - m + sh - 16], r, fill=(10, 10, 10, 255))
d.rounded_rectangle([m, m, S - m - 16, S - m - 16], r, fill=(253, 249, 238, 255), outline=(10, 10, 10, 255), width=bw)
box = S - 2 * m - 16 - 2 * 110
face.thumbnail((box, box), Image.LANCZOS)
ox = m + (S - 2 * m - 16 - face.width) // 2
oy = m + (S - 2 * m - 16 - face.height) // 2 + 10
icon.alpha_composite(face, (ox, oy))
icon.save(sys.argv[2])
print("ok", sys.argv[2])
