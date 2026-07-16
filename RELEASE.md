## What's new in 0.4.0

- **VGA Mode 13h graphics driver** (`gfx.c`) — the biggest visual leap yet!
  - 320x200 true pixel graphics with 256 colors, replacing the old 80x25 text-mode "art"
  - Pixel primitives: `gfx_putpixel`, `gfx_fill`, `gfx_rect`, `gfx_line` (Bresenham)
  - Built-in 8x8 bitmap font (ASCII 32–126) with `gfx_drawchar` / `gfx_drawstr`
  - Clean mode switching: enter 13h on game start, restore text mode 3 on exit

- **Pong** — fully pixel-rendered
  - 320x200 playfield with pixel-precise border, dashed center net
  - 4x4 ball at 2px/tick, 4x24 colored paddles (cyan vs magenta)
  - Score rendered in proportional font, yellow on black

- **Snake** — fully pixel-rendered
  - 60x35 tile grid with 5x5 pixel tiles, giving a crisp retro look
  - Green snake body with lighter green head, yellow food, cyan border

## What's inside

| Game  | Description                              | Size   |
|-------|------------------------------------------|--------|
| pong  | Classic Pong - 2 Player (pixel graphics) | 3.3 KB |
| snake | Snake - Eat, grow, don't bite yourself   | 3.0 KB |

## Build

```
python build.py img
python build.py run
```
