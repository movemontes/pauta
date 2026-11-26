"""Pauta file icon (Treco look): white page with a folded corner, ink outline, hard shadow, staff lines and the mascot.
Usage (system python3 with PIL): python3 make_doc_icon_mascot.py <mascot face png, white background> <out .icns>"""
import os
import subprocess
import sys
import tempfile
from PIL import Image, ImageDraw

INK, WHITE, SHADOW, LINE, SKY = (10, 10, 10, 255), (255, 255, 255, 255), (10, 10, 10, 255), (200, 196, 186, 255), (190, 190, 255, 255)

face = Image.open(sys.argv[1]).convert("RGBA")
w, h = face.size
for p in [(0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)]:
    if face.getpixel(p)[:3] == (255, 255, 255):
        ImageDraw.floodfill(face, p, (255, 0, 255, 255), thresh=40)
px = face.load()
for y in range(h):
    for x in range(w):
        if px[x, y] == (255, 0, 255, 255):
            px[x, y] = (0, 0, 0, 0)
face = face.crop(face.getbbox())

S = 1024
img = Image.new("RGBA", (S, S), (0, 0, 0, 0))
d = ImageDraw.Draw(img)
L, T, R, B, fold, bw, sh = 196, 70, 812, 934, 170, 26, 30
page = [(L, T), (R - fold, T), (R, T + fold), (R, B), (L, B)]
d.polygon([(x + sh, y + sh) for x, y in page], fill=SHADOW)
d.polygon(page, fill=WHITE)
for i in range(5):  # staff
    y = 230 + i * 34
    d.rectangle([L + 70, y, R - 200 if i < 1 else R - 70, y + 8], fill=LINE)
d.polygon([(R - fold, T), (R - fold, T + fold), (R, T + fold)], fill=(235, 232, 224, 255))
d.line(page + [page[0]], fill=INK, width=bw, joint="curve")
d.line([(R - fold, T), (R - fold, T + fold), (R, T + fold)], fill=INK, width=bw, joint="curve")
box = 420
face.thumbnail((box, box), Image.LANCZOS)
img.alpha_composite(face, ((L + R) // 2 - face.width // 2, B - face.height - 70))

with tempfile.TemporaryDirectory() as tmp:
    iconset = os.path.join(tmp, "PautaDocIcon.iconset")
    os.makedirs(iconset)
    for s in (16, 32, 128, 256, 512):
        img.resize((s, s), Image.LANCZOS).save(os.path.join(iconset, f"icon_{s}x{s}.png"))
        img.resize((s * 2, s * 2), Image.LANCZOS).save(os.path.join(iconset, f"icon_{s}x{s}@2x.png"))
    img.save(os.path.join(os.path.dirname(sys.argv[2]), "PautaDocIcon_1024.png"))
    subprocess.run(["iconutil", "-c", "icns", iconset, "-o", sys.argv[2]], check=True)
print("ok", sys.argv[2])
