import os
import glob
from PIL import Image

png_files = glob.glob("Assets/*.png")
for f in png_files:
    img = Image.open(f).convert("RGBA")
    w, h = img.size
    pixels = img.load()
    
    has_dark = False
    has_light = False
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            if a >= 128:
                brightness = (r + g + b) / 3
                if brightness < 128:
                    has_dark = True
                else:
                    has_light = True
    print(f"{os.path.basename(f):25s} | has_dark: {str(has_dark):5s} | has_light: {str(has_light):5s}")
