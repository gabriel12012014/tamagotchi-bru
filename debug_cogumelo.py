from PIL import Image

img = Image.open('Assets/cogumelo.png').convert('RGBA')
w, h = img.size
print(f'Tamanho: {w}x{h}')
pixels = img.load()

has_dark = False
has_light = False
has_transparent = False

for y in range(h):
    for x in range(w):
        r, g, b, a = pixels[x, y]
        if a < 128:
            has_transparent = True
        else:
            brightness = (r + g + b) / 3
            if brightness < 128:
                has_dark = True
            else:
                has_light = True

print(f'has_dark={has_dark}, has_light={has_light}, has_transparent={has_transparent}')
print('Mapa de pixels (T=transparente, W=branco/claro, B=escuro):')

for y in range(h):
    row = []
    for x in range(w):
        r, g, b, a = pixels[x, y]
        if a < 128:
            row.append('.')
        else:
            brightness = (r + g + b) / 3
            row.append('#' if brightness >= 128 else 'X')
    print('  ' + ''.join(row))

print()
print('Detalhes de pixels NAO transparentes:')
for y in range(h):
    for x in range(w):
        r, g, b, a = pixels[x, y]
        if a >= 128:
            brightness = (r + g + b) / 3
            print(f'  ({x},{y}) r={r} g={g} b={b} a={a} brightness={brightness:.0f}')
