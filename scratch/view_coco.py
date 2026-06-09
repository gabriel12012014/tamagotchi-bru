from PIL import Image

img = Image.open("Assets/coco.png").convert("RGBA")
w, h = img.size
pixels = img.load()

for y in range(h):
    row = ""
    for x in range(w):
        r, g, b, a = pixels[x, y]
        if a < 128:
            row += " " # Transparent
        else:
            brightness = (r + g + b) / 3
            # print color
            if brightness < 128:
                row += "#" # Dark
            else:
                row += "." # Light
    print(f"{y:2d}: {row}")
