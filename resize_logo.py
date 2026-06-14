from PIL import Image

img = Image.open("Assets/logo-brunagotchi.png")
# Original is 130x35
# Let's resize to 100 wide, keeping aspect ratio
wpercent = (100/float(img.size[0]))
hsize = int((float(img.size[1])*float(wpercent)))
img = img.resize((100, hsize), Image.Resampling.NEAREST)
img.save("Assets/logo-brunagotchi.png")
print(f"Resized to {img.size[0]}x{img.size[1]}")
