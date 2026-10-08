"""Generate the code-drawn hover halo; the supplied logo is never modified.

Run with Python + Pillow when changing the motion design. The checked-in GIF
is embedded by qmake, so Python is not needed to build or run ANIWORLD.
"""
from math import cos, sin, pi
from pathlib import Path
from PIL import Image, ImageDraw

SIZE = 228  # 3x the 76-pixel widget, for crisp high-DPI playback.
COLORS = [(0, 0, 0), (226, 240, 243), (55, 126, 142), (233, 164, 61),
          (160, 200, 208), (248, 225, 185)]
palette = [channel for color in COLORS for channel in color]
palette += [0] * (768 - len(palette))
frames = []
for frame in range(60):
    t = frame / 60
    canvas = Image.new("P", (SIZE, SIZE), 0)
    canvas.putpalette(palette)
    draw = ImageDraw.Draw(canvas)
    draw.ellipse((12, 12, 216, 216), outline=1, width=3)
    angle = 360 * t - 90
    draw.arc((12, 12, 216, 216), angle, angle + 78, fill=2, width=4)
    draw.arc((12, 12, 216, 216), angle + 180, angle + 223, fill=4, width=3)
    # Small golden paw follows a gentle sway, leaving the brand readable.
    theta = (-44 + 9 * sin(2*pi*t)) * pi / 180
    x, y = 114 + 100*cos(theta), 114 + 100*sin(theta)
    draw.ellipse((x-17, y-17, x+17, y+17), fill=5)
    pulse = 1 + .08*sin(2*pi*t)
    def oval(dx, dy, rx, ry):
        cx, cy = x+dx*pulse, y+dy*pulse
        draw.ellipse((cx-rx*pulse, cy-ry*pulse, cx+rx*pulse, cy+ry*pulse), fill=3)
    oval(0, 5, 8, 6)
    for dx, dy in [(-10, -2), (-4, -9), (4, -9), (10, -2)]:
        oval(dx, dy, 3, 4)
    # A second accent balances the paw without flashing.
    dot = (angle + 202) * pi / 180
    x, y = 114+102*cos(dot), 114+102*sin(dot)
    draw.ellipse((x-4,y-4,x+4,y+4), fill=3)
    frames.append(canvas)

target = Path(__file__).resolve().parents[1] / "resources/brand/logo-hover.gif"
frames[0].save(target, save_all=True, append_images=frames[1:], duration=40,
               loop=0, transparency=0, disposal=2, optimize=False)
print(f"Saved {target.name}: {len(frames)} frames, 2.4-second loop")
