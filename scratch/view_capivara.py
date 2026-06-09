from PIL import Image

img = Image.open("Assets/capivara.png").convert("RGBA")
w, h = img.size
pixels = img.load()

# Check transparency mapping
for y in range(h):
    row = ""
    for x in range(w):
        r, g, b, a = pixels[x, y]
        if a < 128:
            row += " "
        else:
            brightness = (r + g + b) / 3
            if brightness < 128:
                row += "#"
            else:
                row += "."
    print(f"{y:2d}: {row}")
