"""File placeholders for the home page, Treco look: white page, ink outline, staff lines, ink label block.
Usage (system python with PIL): python3 make_placeholders.py <Archivo-Variable.ttf>"""
import os
import sys
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
IMG = os.path.join(ROOT, "src", "project", "qml", "MuseScore", "Project", "internal", "ScoresPage", "images")
INK = (10, 10, 10)
WHITE = (255, 255, 255)
LINE = (200, 196, 186)
SKY = (190, 190, 255)
W, H = 320, 440


def font(size):
    f = ImageFont.truetype(sys.argv[1], size)
    f.set_variation_by_axes([900, 125])
    return f


def make(label, out):
    img = Image.new("RGBA", (W, H), WHITE + (255,))
    d = ImageDraw.Draw(img)
    for s in range(3):  # three short staves
        top = 70 + s * 80
        for i in range(5):
            y = top + i * 9
            d.rectangle([40, y, W - 40, y + 1], fill=LINE)
    if label:
        f = font(54)
        tw = d.textlength(label, font=f)
        bx0, by0 = 40, H - 120
        d.rectangle([bx0, by0 + 34, bx0 + tw + 20, by0 + 62], fill=SKY)  # highlighter under the label
        d.text((bx0 + 10, by0), label, font=f, fill=INK)
    d.rectangle([0, 0, W - 1, H - 1], outline=INK, width=6)
    img.save(os.path.join(IMG, out))
    print("ok", out)


for label, out in (("GP", "Placeholder_GP.png"), ("MEI", "Placeholder_MEI.png"), ("MIDI", "Placeholder_MIDI.png"),
                   ("XML", "Placeholder_MXML.png"), ("", "Placeholder_Other.png")):
    make(label, out)
