# RT

42 **Ray Tracer** in C — forked from [mini-RT](https://github.com/pix3l-p33p3r/mini-RT).

Executable: `rt`

## Build

```bash
make
./rt scenes/scene1.rt
```

Needs miniLibX (`-lmlx -lXext -lX11`), `-lm`, `-pthread`, and `-lOpenCL`.

### Perf

- **CPU threads (default):** row-parallel pthread raytrace  
- **Dual-GPU OpenCL (AMD RX 6500 + Intel UHD 770):**
  ```bash
  ./rt-gpu scenes/giza.rt
  ```
  (Auto-enters Nix OpenCL env, builds with Nix `ocl-icd`, and splits scanlines across both GPUs weighted by compute units. Press `G` inside to toggle GPU/CPU live.)

## Status

Base: miniRT bonus (sphere / plane / cylinder, multi lights, shadows, specular, `.rt` scenes).

### Mandatory checklist

- [x] Ray tracing + window
- [x] Sphere, plane, cylinder
- [x] Cone (`cn` — same args as `cy`)
- [x] Translation / rotation
- [x] Camera from scene file
- [x] Expose / redraw without full recompute
- [x] Brightness, shadows, multi-spot, shine

### Options (grade lives here)

- [x] Ambient light
- [x] External scene files
- [x] Recursive mirror reflection (`scenes/mirror_demo.rt`, optional trailing `mirror` `[0..1]`)
- [x] Transparency, Snell refraction, dispersion & tinted glass shadows (`scenes/glass_demo.rt`, `scenes/prism_vortex.rt`, `scenes/liquid_snowflake.rt`)
- [x] Extra shapes & parallel light: `tr`, `hs`, `dk`, `pb`, `D` (`scenes/pokeball.rt`, `scenes/giza.rt`, `scenes/logo_42.rt`, `scenes/nixos_logo.rt`, `scenes/shapes_demo.rt`)
- [x] Surfaces: checker/stripe/marble disruption, bump mapping, PPM image textures (`scenes/surfaces_demo.rt`)
- [x] Live controls: `WASD`/`QE` move · arrows look · `G` toggle GPU/CPU · `P` export `render_out.ppm` · `ESC` clean exit (0 leaks)

## Branches

| Branch | Role |
|--------|------|
| `main` | Stable / defence-ready |
| `develop` | Integration — merge features here first |
| `test` | Experiments / WIP checks |

Flow: work on `develop` → promote `test` → promote `main` after **each** finished task.

Roadmap: [TODO.md](./TODO.md)

## Team

- [@pix3l-p33p3r](https://github.com/pix3l-p33p3r)
- [@0xakhii](https://github.com/0xakhii)
