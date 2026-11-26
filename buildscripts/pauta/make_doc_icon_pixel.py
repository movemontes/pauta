"""Pixel-art Pauta document icon (.mscz files): white page with folded corner, staff and a note.
Writes share/icons/AppIcon/PautaDocIcon.icns"""
import os, subprocess, tempfile
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "share", "icons", "AppIcon")
N = 32
K = (27, 26, 31, 255)       # ink
W = (248, 245, 238, 255)    # paper
S = (190, 182, 170, 255)    # shade
L = (150, 160, 172, 255)    # staff lines
O = (240, 162, 58, 255)     # orange
T = (0, 0, 0, 0)
g = [[T] * N for _ in range(N)]
def rect(x0, y0, x1, y1, c):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            g[y][x] = c
# page outline + paper
rect(5, 1, 26, 30, K)
rect(6, 2, 25, 29, W)
rect(6, 29, 25, 29, S); rect(25, 9, 25, 29, S)
# folded corner (top right)
for i in range(7):
    for x in range(26 - 7 + i, 27):
        g[1 + i][x] = T
    g[1 + i][19 + i] = K
rect(19, 1, 19, 8, K); rect(19, 8, 26, 8, K)
for i in range(6):
    rect(20, 2 + i, 19 + i, 2 + i, S)
# staff
for y in (13, 16, 19, 22, 25):
    rect(8, y, 23, y, L)
# note
rect(10, 22, 14, 24, K); rect(11, 21, 13, 21, K); rect(11, 25, 13, 25, K)
rect(14, 11, 15, 23, K)
rect(16, 12, 17, 13, K); rect(18, 14, 19, 15, K); rect(19, 16, 19, 18, K)
rect(20, 11, 21, 12, O)
img = Image.new("RGBA", (N, N))
for y in range(N):
    for x in range(N):
        img.putpixel((x, y), g[y][x])
with tempfile.TemporaryDirectory() as tmp:
    iconset = os.path.join(tmp, "PautaDocIcon.iconset")
    os.makedirs(iconset)
    for pt in (16, 32, 128, 256, 512):
        for scale in (1, 2):
            px = pt * scale
            img.resize((px, px), Image.NEAREST).save(os.path.join(iconset, f"icon_{pt}x{pt}{'@2x' if scale == 2 else ''}.png"))
    subprocess.run(["iconutil", "-c", "icns", iconset, "-o", os.path.join(OUT, "PautaDocIcon.icns")], check=True)
img.resize((512, 512), Image.NEAREST).save(os.path.join(tempfile.gettempdir(), "pauta_doc_preview.png"))
print("ok")
