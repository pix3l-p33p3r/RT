---
title: Transparency, Snell refraction, and tinted glass shadows (CPU + OpenCL GPU)
status: ready
branch: develop
---

# Plan: Glass optics & transparent shadows (RT options)

## Design

1. Scene syntax (backwards-compatible): optional trailing `mirror [transp [ior]]` on `sp`, `pl`, `cy`, `cn`.
   - `transp` in `[0.0..1.0]` (default `0.0`)
   - `ior` in `[1.0..3.0]` (default `1.5` if `transp > 0`, else `1.0`)
2. `Objects/Sphere.c`: accept smallest positive root (`t > EPSILON`) so refracted rays exiting a sphere hit the back wall.
3. `Material/refract.c`: Snell's law `refract_vec()` with inside/outside normal flip + Total Internal Reflection (TIR) fallback + tinted transmission helper.
4. `Material/reflect.c`: combine opaque (`1 - m - tr`), mirror (`m`), and refracted (`tr`) rays up to `MAX_BOUNCES`.
5. `Material/Diffuse.c` + `Material/specular.c`: RGB shadow attenuation through transparent objects (colored glass casts tinted translucent shadows; opaque objects still block 100%).
6. OpenCL GPU (`gpu/cl_internal.h`, `gpu/cl_scene.c`, `gpu/kernels/trace.cl`): mirror + Snell refraction + TIR + tinted transparent shadows.
7. Unit test (`tests/test_refract.c`) + demo scene (`scenes/glass_demo.rt`).

## Verify

- `norminette` → 0 errors
- `make re && make test_cone && make test_reflect && make test_refract`
- Valgrind (`0 bytes in 0 blocks`, `0 errors`) on `scenes/glass_demo.rt`
- Nix dual-GPU (`RT_USE_GPU=1 RT_MAX_FRAMES=5 ./rt scenes/glass_demo.rt`, `GPU_EXIT:0`)
- Promote `develop → test → main`
