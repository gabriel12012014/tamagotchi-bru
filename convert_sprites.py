import os
import glob
from PIL import Image

def rgb_to_rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

def convert_png_to_c_array(png_path, name):
    img = Image.open(png_path).convert("RGBA")
    w, h = img.size
    
    out_c = f"// === PIXEL ART {name.upper()} ({w}x{h}) ===\n"
    out_c += f"const int {name}_width = {w};\n"
    out_c += f"const int {name}_height = {h};\n"
    out_c += f"const unsigned short {name}_sprite[{w * h}] PROGMEM={{\n"
    
    out_js = f"// === PIXEL ART {name.upper()} ({w}x{h}) ===\n"
    out_js += f"const {name}_width = {w};\n"
    out_js += f"const {name}_height = {h};\n"
    out_js += f"const {name}_sprite = [\n"
    
    pixels = img.load()
    c_vals = []
    js_vals = []
    
    for y in range(h):
        for x in range(w):
            r, g, b, a = pixels[x, y]
            if a < 128:
                val = 0x0000 # Transparent
            else:
                if r == 0 and g == 0 and b == 0:
                    val = 0xFFFF # Map black to white so it's visible on dark background
                else:
                    val = rgb_to_rgb565(r, g, b)
                    if val == 0x0000:
                        val = 0xFFFF
            
            c_str = f"0x{val:04X}"
            c_vals.append(c_str)
            js_vals.append(c_str)
            
    # Format C array
    lines = []
    for i in range(0, len(c_vals), 16):
        lines.append(", ".join(c_vals[i:i+16]))
    out_c += ",\n".join(lines)
    out_c += "\n};\n\n"
    
    # Format JS array
    lines = []
    for i in range(0, len(js_vals), 16):
        lines.append(", ".join(js_vals[i:i+16]))
    out_js += ",\n".join(lines)
    out_js += "\n];\n\n"
    
    return out_c, out_js

def main():
    png_files = glob.glob("Assets/*.png")
    if not png_files:
        print("No PNG files found in Assets folder.")
        return
        
    all_c = ""
    all_js = ""
    for f in png_files:
        basename = os.path.basename(f).replace(".png", "")
        print(f"Converting {f} ({basename})...")
        c, js = convert_png_to_c_array(f, basename)
        all_c += c
        all_js += js
        
    with open("sprites.h", "w") as f:
        f.write("#ifndef SPRITES_H\n#define SPRITES_H\n\n")
        f.write("#include <pgmspace.h>\n\n")
        f.write(all_c)
        f.write("#endif\n")
        
    with open("sprites.js", "w") as f:
        f.write(all_js)
        
    print("Generated sprites.h and sprites.js successfully!")

if __name__ == "__main__":
    main()
