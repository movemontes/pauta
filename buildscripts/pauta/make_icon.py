"""Draws Pauta's app icon in Treco's style (blue rounded square, white block letter, hard black shadow)
and writes share/icons/AppIcon/PautaAppIcon.icns plus a 1024 px PNG."""
import os, subprocess, sys, tempfile
from PIL import Image, ImageDraw

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
OUT = os.path.join(ROOT, "share", "icons", "AppIcon")
BLUE, WHITE, BLACK = (0, 0, 255, 255), (255, 255, 255, 255), (0, 0, 0, 255)


def draw(size=1024):
    s = size / 1024
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle([100 * s, 100 * s, 924 * s, 924 * s], radius=185 * s, fill=BLUE)

    # Block "P": stem + bowl with a square hole. Same proportions as Treco's "T".
    def letter(dx, dy, color):
        def r(x0, y0, x1, y1):
            d.rectangle([(x0 + dx) * s, (y0 + dy) * s, (x1 + dx) * s - 1, (y1 + dy) * s - 1], fill=color)
        r(300, 272, 458, 752)          # stem
        r(300, 272, 700, 392)          # top of bowl
        r(580, 272, 724, 592)          # right side of bowl
        r(300, 472, 700, 592)          # bottom of bowl
    letter(38, 38, BLACK)
    letter(0, 0, WHITE)
    return img


def main():
    os.makedirs(OUT, exist_ok=True)
    big = draw(1024)
    big.save(os.path.join(OUT, "PautaAppIcon_1024.png"))
    with tempfile.TemporaryDirectory() as tmp:
        iconset = os.path.join(tmp, "PautaAppIcon.iconset")
        os.makedirs(iconset)
        for pt in (16, 32, 128, 256, 512):
            for scale in (1, 2):
                px = pt * scale
                name = f"icon_{pt}x{pt}{'@2x' if scale == 2 else ''}.png"
                draw(px).save(os.path.join(iconset, name)) if px < 1024 else big.save(os.path.join(iconset, name))
        subprocess.run(["iconutil", "-c", "icns", iconset, "-o", os.path.join(OUT, "PautaAppIcon.icns")], check=True)
    print("ok", OUT)


if __name__ == "__main__":
    sys.exit(main())
