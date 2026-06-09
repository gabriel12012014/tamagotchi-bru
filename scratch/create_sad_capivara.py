from PIL import Image

img = Image.open("Assets/capivara.png").convert("RGBA")
w, h = img.size
pixels = img.load()

# Let's modify the eye to an 'X' (dead/almost dying eye)
# Original eye was at y=4, x=12,13 and y=5, x=12,13,14.
# Let's clear the original eye area (make transparent)
for y in [3, 4, 5, 6]:
    for x in [11, 12, 13, 14, 15]:
        pixels[x, y] = (0, 0, 0, 0) # Transparent

# Draw the 'X' eye (using dark outlines, i.e., black color with full opacity)
# X centered around y=4, x=13
pixels[12, 3] = (0, 0, 0, 255)
pixels[14, 3] = (0, 0, 0, 255)
pixels[13, 4] = (0, 0, 0, 255)
pixels[12, 5] = (0, 0, 0, 255)
pixels[14, 5] = (0, 0, 0, 255)

# Add a tear pixel at y=7, x=13 (below the eye)
# Let's make it white (which maps to outlines/white pixel)
pixels[13, 7] = (0, 0, 0, 255) # Tear outline
pixels[13, 8] = (0, 0, 0, 255) # Tear drop

# Print the resulting grid to verify
for y in range(h):
    row = ""
    for x in range(w):
        r, g, b, a = pixels[x, y]
        if a < 128:
            row += " "
        else:
            row += "#"
    print(f"{y:2d}: {row}")

# Save the new asset
img.save("Assets/capivara_sad.png")
print("Saved Assets/capivara_sad.png")
