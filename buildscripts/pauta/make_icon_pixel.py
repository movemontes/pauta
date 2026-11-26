"""Pixel-art Pauta icon (32x32 grid scaled up): beveled frame, beige paper, a black eighth note.
Inspired by pixel-art app icons, not a copy of any brand. Writes share/icons/AppIcon/PautaPixel_1024.png"""
import os
from PIL import Image

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "share", "icons", "AppIcon")
N = 32
K = (27, 26, 31, 255)       # ink
F = (123, 142, 160, 255)    # frame
FL = (169, 184, 198, 255)   # frame light
FD = (79, 95, 110, 255)     # frame dark
B = (214, 205, 191, 255)    # beige paper
BD = (158, 149, 138, 255)   # paper shadow
O = (240, 162, 58, 255)     # orange accent
T = (0, 0, 0, 0)

g = [[T] * N for _ in range(N)]
def rect(x0, y0, x1, y1, c):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            g[y][x] = c

# body with 1px black outline, rounded corners (cut 2 px)
rect(2, 2, 29, 29, K)
rect(3, 3, 28, 28, F)
rect(3, 3, 28, 3, FL); rect(3, 3, 3, 28, FL)      # bevel light
rect(3, 28, 28, 28, FD); rect(28, 3, 28, 28, FD)  # bevel dark
for (x, y) in [(2, 2), (29, 2), (2, 29), (29, 29)]:
    g[y][x] = T
# paper card
rect(6, 6, 25, 25, K)
rect(7, 7, 24, 24, B)
rect(7, 24, 24, 24, BD); rect(24, 7, 24, 24, BD)
# five staff lines
for y in (10, 13, 16, 19, 22):
    rect(8, y, 23, y, BD)
# eighth note: head (ellipse-ish), stem, flag
rect(10, 19, 14, 21, K); rect(11, 18, 13, 18, K); rect(11, 22, 13, 22, K)
rect(14, 9, 15, 20, K)                       # stem
rect(16, 10, 17, 11, K); rect(18, 12, 19, 13, K); rect(19, 14, 19, 16, K)  # flag
# orange spark (accent)
rect(20, 8, 21, 9, O)

img = Image.new("RGBA", (N, N))
for y in range(N):
    for x in range(N):
        img.putpixel((x, y), g[y][x])
big = img.resize((1024, 1024), Image.NEAREST)
os.makedirs(OUT, exist_ok=True)
big.save(os.path.join(OUT, "PautaPixel_1024.png"))

# macOS app icon (.icns) with hard pixel edges at every size
import subprocess, tempfile
with tempfile.TemporaryDirectory() as tmp:
    iconset = os.path.join(tmp, "PautaAppIcon.iconset")
    os.makedirs(iconset)
    for pt in (16, 32, 128, 256, 512):
        for scale in (1, 2):
            px = pt * scale
            name = f"icon_{pt}x{pt}{'@2x' if scale == 2 else ''}.png"
            img.resize((px, px), Image.NEAREST).save(os.path.join(iconset, name))
    subprocess.run(["iconutil", "-c", "icns", iconset, "-o", os.path.join(OUT, "PautaAppIcon.icns")], check=True)
# small copy for the loading screen / about box
img.resize((256, 256), Image.NEAREST).save(os.path.join(ROOT, "src", "appshell", "qml", "MuseScore", "AppShell", "resources", "pauta_pixel_icon.png"))
print("ok")
