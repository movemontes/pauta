"""Makes static instances of Archivo (OFL) at Treco's settings: width 125, weight 500 (regular) and 800 (bold).
The family is renamed "Pauta Archivo" so it never clashes with an installed Archivo. Run with a Python that has fontTools."""
import os, sys
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = sys.argv[1]  # path to Archivo-Variable.ttf
OUT = os.path.join(ROOT, "src", "framework", "ui", "data")
FAMILY = "Pauta Archivo"

for style, weight in (("Regular", 500), ("Bold", 800)):
    font = instancer.instantiateVariableFont(TTFont(SRC), {"wght": weight, "wdth": 125})
    name = font["name"]
    for rec in list(name.names):
        if rec.nameID in (16, 17, 21, 22, 25):
            name.removeNames(nameID=rec.nameID)
    ps = FAMILY.replace(" ", "") + "-" + style
    for nid, val in ((1, FAMILY), (2, style), (3, ps), (4, f"{FAMILY} {style}"), (6, ps)):
        name.setName(val, nid, 3, 1, 0x409)
        name.setName(val, nid, 1, 0, 0)
    font["OS/2"].usWeightClass = 700 if style == "Bold" else 400
    font["OS/2"].fsSelection = (font["OS/2"].fsSelection & ~0b1100001) | (0b100000 if style == "Bold" else 0b1000000)
    font["head"].macStyle = 1 if style == "Bold" else 0
    path = os.path.join(OUT, f"PautaArchivo-{style}.ttf")
    font.save(path)
    print("wrote", path)
