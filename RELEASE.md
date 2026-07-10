## Fixes

- **Game return path** — `game_entry.asm` now properly saves and restores DS, SS, and SP across the far call. Without this, returning from a game to the shell would land on corrupted kernel state (wrong data segment → wrong return addresses → splash screen freeze).

## Build

```
python build.py img
python build.py run
```
