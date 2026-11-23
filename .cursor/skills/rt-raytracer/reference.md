# RT Ray Tracer — Technical Reference

## 1. Scene Grammar (`.rt`)

All shapes accept optional trailing material parameters (preserving backwards compatibility with legacy scenes):

```text
[mirror [transp [ior [pattern [bump [ppm_path]]]]]]
```

| Field | Range | Behavior |
|-------|-------|----------|
| `mirror` | `0.0–1.0` | Recursive mirror reflection weight (`MAX_BOUNCES = 5`) |
| `transp` | `0.0–1.0` | Refractive transparency weight + RGB tinted shadows |
| `ior` | `1.0–3.0` | Snell's law index; `ior >= 1.7` triggers RGB chromatic dispersion (`±0.08`) |
| `pattern` | `0–4` | `0` solid · `1` checker · `2` stripe · `3` marble · `4` PPM image texture |
| `bump` | `0.0–2.0` | Tangent-frame sin/cos normal perturbation |
| `ppm_path` | path | Binary P6 or ASCII P3 PPM file (only when `pattern == 4`) |

### Elements & Primitives

| Token | Enum | File (CPU) | Syntax |
|-------|------|-----------|--------|
| `A` | — | `parsing/check_elements.c` | `A ratio R,G,B` |
| `C` | — | `parsing/check_elements.c` | `C ox,oy,oz lx,ly,lz fov` (`l*` in `[-1,1]`) |
| `L` | — | `parsing/check_elements.c` | `L x,y,z intensity R,G,B [radius]` (`radius > 0` = soft area shadow) |
| `D` | — | `parsing/check_shapes2.c` | `D dx,dy,dz intensity R,G,B` (parallel sun light, `is_dir = 1`) |
| `sp` | `1` | `Objects/Sphere.c` | `sp cx,cy,cz diam R,G,B [mat...]` |
| `pl` | `2` | `Objects/Plane.c` | `pl px,py,pz nx,ny,nz R,G,B [mat...]` |
| `cy` | `3` | `Objects/Cylinder.c` | `cy cx,cy,cz ax,ay,az diam height R,G,B [mat...]` |
| `cn` | `4` | `Objects/Cone.c` | `cn cx,cy,cz ax,ay,az diam height R,G,B [mat...]` |
| `tr` | `5` | `Objects/Triangle.c` | `tr x1,y1,z1 x2,y2,z2 x3,y3,z3 R,G,B [mat...]` |
| `hs` | `6` | `Objects/HalfSphere.c` | `hs cx,cy,cz nx,ny,nz diam R,G,B [mat...]` |
| `dk` | `7` | `Objects/Disk.c` | `dk cx,cy,cz nx,ny,nz diam R,G,B [mat...]` |
| `pb` | `8` | `Objects/Paraboloid.c` | `pb cx,cy,cz ax,ay,az diam height R,G,B [mat...]` |
| `to` | `9` | `Objects/Torus.c` | `to cx,cy,cz nx,ny,nz major_r minor_r R,G,B [mat...]` |
| `ns` | `10` | `Objects/Csg.c` | `ns cx,cy,cz diam` (negative CSG sphere carving solid hits) |

## 2. Environment Variables & Controls

| Env Var | Effect |
|---------|--------|
| `RT_THREADS=N` | CPU pthread count (`1..64`, default `nproc`) |
| `RT_USE_GPU=1` | Enable multi-GPU OpenCL (auto-set by `./rt-gpu`) |
| `RT_STEREO=0\|1\|2` | `0` Mono · `1` Red-Cyan Anaglyph 3D · `2` Side-by-Side VR |
| `RT_MAX_FRAMES=N` | Stop after `N` progressive frames and exit `0` via `close_hook` |
| `RT_EXPORT_PATH=file.ppm` | Export final frame to binary PPM before exiting on `RT_MAX_FRAMES` |

In-window keys (`controls.c`): `W/A/S/D` + `Q/E` move · Arrows yaw/pitch · `V` stereo cycle · `G` GPU/CPU toggle · `P` save `render_out.ppm` · `ESC`/`X` clean exit.

## 3. Nix + Dual-GPU OpenCL Playbook (Linux 5.15 Cluster)

- **Why `nixos-24.05` is pinned in `flake.nix`:** Unstable Mesa 26 `rusticl` requires kernel DRM `>= 3.59` (Linux 6.11+) and refuses to initialize AMD RX 6500 on 5.15 (`DRM 3.42`). Pinning `pkgs2405.mesa.opencl` exposes BOTH **AMD Radeon RX 6500 (16 CU)** and **Intel UHD Graphics 770 (2 CU)** under `RUSTICL_ENABLE=radeonsi,iris`.
- **Why `./rt-gpu` builds `./rt_ocl` separately:** Ubuntu host glibc 2.35 cannot `dlopen` Nix OpenCL ICDs (glibc 2.39+), and Nix-built ELFs reference `/nix/store/.../ld-linux-x86-64.so.2` which only exists inside `nix-user-chroot` (`nix-enter`). Keeping `./rt` native and `./rt_ocl` Nix-linked prevents clashes.
- **Makefile `-std=gnu11` & `-Wno-unused-result`:** Required when building under Nix GCC 16 (defaults to C23 strict function pointers that reject MLX's K&R `int (*)()` prototypes, and `_FORTIFY_SOURCE` warns on `libft` `write()`).
- **Never run `make re -j4`:** `re: fclean all` runs `fclean` and `all` concurrently under `-j`, deleting freshly compiled outputs. Always run `make re` sequentially or `make fclean && make -j4`.

## 4. Known Pitfalls Already Solved (Do Not Regress)

1. **Sphere interior exit:** `test_sphere` must accept the smallest positive root (`t > EPSILON`), never `if (t1 < 0 || t2 < 0) return 0`, or rays inside glass spheres miss the exit wall.
2. **Ambient scaling:** Scale ambient RGB by `1.0f / 255.0f` **once**, never twice.
3. **Shadow loop self-skip:** Inside `while (objects[++i].gtfm)`, use plain `continue ;` (never an extra `i++`), or the next object is skipped and the NULL sentinel can be overrun.
4. **Triangle / 3D logo extrusion:** Since `-Z` is up, map 2D `(x, up_y)` to `(x, depth_y, -up_y)`. Keep total objects `<= MAX_GPU_OBJS` (`256`).
5. **OpenCL `surf`/`local` keywords:** Avoid `local` and `surf` as identifiers in `.cl` files; use `lit` / `hp`.
6. **Shader JIT speed:** Keep `trace_one_ray` called from a single loop in `trace_frame` and cap torus SDF marching to 48 steps so Mesa's shader cache warms fast and subsequent runs execute in `< 1s`.
