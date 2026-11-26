"""Pauta: static Archivo files for the Treco-style look.
Usage: python make_archivo_fonts.py <Archivo-Variable.ttf> <out dir>
 - Pauta Titulo: width 125, weight 900 (big titles)
 - Pauta Texto Regular/Bold: width 100, weight 500/700 (used when Clash Grotesk is not inside the app)
"""
import sys
from fontTools.ttLib import TTFont
from fontTools.varLib.instancer import instantiateVariableFont

src, out = sys.argv[1], sys.argv[2]


def make(family, style, wdth, wght, filename, bold):
    font = instantiateVariableFont(TTFont(src), {"wdth": wdth, "wght": wght})
    name = font["name"]
    for rec in list(name.names):
        if rec.nameID in (1, 2, 3, 4, 6, 16, 17, 21, 22, 25):
            name.removeNames(nameID=rec.nameID)
    full = f"{family} {style}"
    ps = full.replace(" ", "")
    for nid, val in ((1, family), (2, style), (3, ps), (4, full), (6, ps)):
        name.setName(val, nid, 3, 1, 0x409)
        name.setName(val, nid, 1, 0, 0)
    font["OS/2"].usWeightClass = 700 if bold else 400
    font["OS/2"].fsSelection = (font["OS/2"].fsSelection & ~0b1100001) | (0b100000 if bold else 0b1000000)
    font["head"].macStyle = 1 if bold else 0
    if "STAT" in font:
        del font["STAT"]
    font.save(f"{out}/{filename}")
    print("ok", filename)


make("Pauta Titulo", "Regular", 125, 900, "PautaTitulo-Regular.ttf", False)
make("Pauta Texto", "Regular", 100, 500, "PautaTexto-Regular.ttf", False)
make("Pauta Texto", "Bold", 100, 700, "PautaTexto-Bold.ttf", True)
