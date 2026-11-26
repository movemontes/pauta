"""Pixel-art file placeholders for the home page (no original-program logo)."""
import os
from PIL import Image, ImageDraw, ImageFont
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
IMG = os.path.join(ROOT, "src", "project", "qml", "MuseScore", "Project", "internal", "ScoresPage", "images")
FONT = os.path.join(ROOT, "src", "framework", "ui", "data", "PautaPixel-Regular.ttf")
K = (27, 26, 31); W = (248, 245, 238); S = (190, 182, 170); L = (150, 160, 172)
P = 10  # pixel size: 32 x 44 grid -> 320 x 440
def make(label, accent, out):
    img = Image.new("RGBA", (320, 440), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    def r(x0, y0, x1, y1, c):
        d.rectangle([x0 * P, y0 * P, (x1 + 1) * P - 1, (y1 + 1) * P - 1], fill=c)
    r(0, 0, 31, 43, K); r(1, 1, 30, 42, W)
    r(1, 42, 30, 42, S); r(30, 9, 30, 42, S)
    for i in range(9):              # folded corner
        r(23 + i, i, 31, i, (0, 0, 0, 0))
        r(22 + i, i, 22 + i, i, K)
    r(22, 0, 22, 9, K); r(22, 9, 31, 9, K)
    for i in range(1, 8):
        r(23, 1 + i, 22 + i, 1 + i, S)
    for y in (14, 17, 20, 23, 26):  # staff
        r(4, y, 27, y, L)
    r(9, 23, 14, 25, K); r(10, 22, 13, 22, K); r(10, 26, 13, 26, K)  # note
    r(14, 11, 15, 24, K); r(16, 12, 17, 13, K); r(18, 14, 19, 15, K); r(19, 16, 19, 19, K)
    if not label:
        img.save(os.path.join(IMG, out)); return
    r(4, 31, 27, 39, accent)         # label band
    r(4, 31, 27, 31, K); r(4, 39, 27, 39, K)
    f = ImageFont.truetype(FONT, 80)
    tw = d.textlength(label, font=f)
    d.text(((320 - tw) / 2, 300), label, font=f, fill=(255, 255, 255))
    img.save(os.path.join(IMG, out))
make("GP", (47, 143, 91), "Placeholder_GP.png")
make("MEI", (122, 79, 176), "Placeholder_MEI.png")
make("MIDI", (192, 57, 43), "Placeholder_MIDI.png")
make("XML", (47, 111, 192), "Placeholder_MXML.png")
make("", None, "Placeholder_Other.png")
print("ok")
