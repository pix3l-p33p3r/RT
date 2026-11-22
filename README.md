# RT

42 **Ray Tracer** in C — forked from [mini-RT](https://github.com/pix3l-p33p3r/mini-RT).

Executable: `rt`

## Build

```bash
make
./rt scenes/scene1.rt
```

Needs miniLibX (`-lmlx -lXext -lX11`) and `-lm`.

## Status

Base: miniRT bonus (sphere / plane / cylinder, multi lights, shadows, specular, `.rt` scenes).

### Mandatory checklist

- [x] Ray tracing + window
- [x] Sphere, plane, cylinder
- [x] Cone (`cn` — same args as `cy`)
- [x] Translation / rotation
- [x] Camera from scene file
- [ ] Expose / redraw without full recompute
- [x] Brightness, shadows, multi-spot, shine

### Options (grade lives here)

- [x] Ambient light
- [x] External scene files
- [ ] Mirror reflection, transparency, textures, bump, more shapes, …

## Branches

| Branch | Role |
|--------|------|
| `main` | Stable / defence-ready |
| `develop` | Integration — merge features here first |
| `test` | Experiments / WIP checks |

Flow: `test` → `develop` → `main`. Local and GitHub stay in sync (push after every commit).

Roadmap: [TODO.md](./TODO.md)

## Team

- [@pix3l-p33p3r](https://github.com/pix3l-p33p3r)
- [@0xakhii](https://github.com/0xakhii)
