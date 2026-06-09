from PIL import Image

img = Image.open("Assets/capivara.png").convert("RGBA")
w, h = img.size
pixels = img.load()

# Let's inspect the original eye region and print the grid to terminal
print("Original capivara grid around eye:")
for y in range(2, 8):
    row = ""
    for x in range(10, 17):
        r, g, b, a = pixels[x, y]
        row += "#" if a >= 128 else "."
    print(f"y={y}: {row} (x=10..16)")

# Let's clear the eye region
# The original eye in capivara.png is around y=4,5, x=12,13,14
for y in [3, 4, 5]:
    for x in [11, 12, 13, 14, 15]:
        pixels[x, y] = (0, 0, 0, 0)

# Draw a closed eye: a horizontal line representing sleeping eyes at y=4
pixels[11, 4] = (0, 0, 0, 255)
pixels[12, 4] = (0, 0, 0, 255)
pixels[13, 4] = (0, 0, 0, 255)
pixels[14, 4] = (0, 0, 0, 255)

print("\nModified capivara grid around eye:")
for y in range(2, 8):
    row = ""
    for x in range(10, 17):
        r, g, b, a = pixels[x, y]
        row += "#" if a >= 128 else "."
    print(f"y={y}: {row} (x=10..16)")

# Save the new asset
img.save("Assets/capivara_sleep.png")
print("Saved Assets/capivara_sleep.png")
