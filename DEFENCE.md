# RT — Live Defence Guide (125/125)

## Quick commands

```bash
make re                          # build native CPU (20 pthreads)
./rt scenes/<scene>.rt           # run on 20 CPU threads
./rt-gpu scenes/<scene>.rt       # run on Dual-GPU OpenCL (Intel UHD 770 + AMD RX 6500)
./scripts/verify_all_scenes.sh   # check norminette + 7 unit tests + all 19 scenes
```

## Live in-window controls (for evaluator manipulation)

| Key | Action |
|-----|--------|
| `W` `A` `S` `D` | Move camera forward / left / back / right |
| `Q` / `E` | Move camera up / down |
| `←` `→` `↑` `↓` | Yaw / pitch camera direction |
| `V` | Cycle **Mono → Red-Cyan Anaglyph 3D → Side-by-Side VR** |
| `G` | Live toggle **Dual-GPU OpenCL ↔ 20-thread CPU** |
| `P` | Export current render to `render_out.ppm` |
| `ESC` / window `X` | Clean exit (`0` leaks verified via Valgrind) |

## Feature → Live Scene Map

### Mandatory (0 pts, unlocks everything)
| Requirement | Live scene |
|-------------|------------|
| 4 basic equation shapes (`pl`, `sp`, `cy`, `cn`) + 2 spots + shadows + shine | `scenes/vi1_four_shapes.rt` |
| Same scene, second camera viewpoint | `scenes/vi2_other_view.rt` |
| Shadow mixing (multi-spot RGB overlapping shadows) | `scenes/vi3_shadow_mix.rt` |
| Expose redraw without full recompute | Cover/uncover any window (`expose_hook` blits persistent `mlx_img`) |
| 0 memory leaks | `RT_MAX_FRAMES=1 valgrind --leak-check=full ./rt scenes/cone_demo.rt` → `0 bytes in 0 blocks` |
| 42 Norminette | `norminette ...` → `0 errors` |

### Options & Bonuses (up to 125)
| Option / Bonus | Live scene |
|----------------|------------|
| **Ambient + `.rt` scene parser + progressive AA** | Any scene |
| **Multi-spot RGB lights** | `scenes/multi_spots_grid.rt` |
| **Recursive mirror reflection (5 bounces)** | `scenes/mirror_demo.rt` |
| **Transparency + Snell refraction + TIR + tinted glass shadows** | `scenes/glass_demo.rt` |
| **Chromatic dispersion (RGB IOR split)** | `scenes/prism_vortex.rt`, `scenes/liquid_snowflake.rt` |
| **Textures (PPM P6/P3), bump mapping, checker/stripe/marble** | `scenes/surfaces_demo.rt` |
| **Parallel / directional sun light (`D`)** | `scenes/shapes_demo.rt` |
| **Limited objects (`hs` half-sphere, `dk` disk)** | `scenes/pokeball.rt`, `scenes/shapes_demo.rt` |
| **Composed elements (`tr` pyramids, 3D 42 & NixOS logos)** | `scenes/giza.rt`, `scenes/logo_42.rt`, `scenes/nixos_logo.rt` |
| **More natives (`pb` paraboloid, `to` torus)** | `scenes/shapes_demo.rt`, `scenes/csg_torus_demo.rt` |
| **Negative CSG subtraction (`ns` carving)** | `scenes/csg_torus_demo.rt` |
| **Soft area shadows (`L ... radius`) + Stereoscopic 3D / VR (`V` key)** | `scenes/stereo_soft_demo.rt` |
| **20-thread CPU pthreads + Dual-GPU OpenCL (CU-weighted split)** | `./rt-gpu scenes/liquid_snowflake.rt` |

## Architecture talking points (be ready to explain)
1. **Object transforms (`gtfm`):** forward/inverse 4×4 matrices precomputed at load; zero heap mallocs in `apply_transform_vector()` for lock-free 20-thread scaling (~27 FPS CPU).
2. **GPU compute vs forbidden GPU rasterization:** OpenCL compute kernel (`gpu/kernels/trace.cl`) computes per-pixel ray intersections into a float buffer; host still blits pixels via MiniLibX (100% compliant with Ch.IV).
3. **Dual-GPU load balancing:** discovers & deduplicates Intel UHD 770 (2 CU) + AMD RX 6500 (16 CU), assigns row bands proportional to CU count (`[0..66)` + `[66..600)`), and drives each GPU on its own pthread.
4. **AI disclosure:** be transparent with peers about AI assistance and make sure both teammates can walk through every math equation (quadratic/paraboloid, Möller–Trumbore, Snell's law, torus SDF, CSG exit).
