---
title: Recursive mirror reflection (CPU + OpenCL GPU)
status: ready
branch: develop
---

# Plan: Recursive mirror reflection (RT option)

## Design

1. Scene syntax (backwards-compatible): optional trailing `mirror` float `[0.0..1.0]` on `sp`, `pl`, `cy`, `cn` (defaults to `0.0`).
2. CPU (`Material/reflect.c`): `trace_ray(vars, ray, depth)` up to `MAX_BOUNCES = 5`.
   - On hit: compute local Phong (`compute_color`).
   - If `mirror > 0 && depth > 0`: spawn reflected ray from `hitpoint + n * 0.001f` along `reflect(dir, n)`, recurse `depth - 1`, blend `(1 - m) * local + m * refl`.
3. OpenCL GPU (`gpu/kernels/trace.cl`): iterative bounce loop (`MAX_BOUNCES = 5`, weight attenuation `weight * (1 - m)` then `weight *= m`) — OpenCL 1.2 compliant, matches CPU output.
4. Demo scene: `scenes/mirror_demo.rt` (reflective floor + chrome sphere + matte sphere + reflective cylinder + cone + 2 lights).

## Verify

- `norminette` → 0 errors
- `make re && make test_cone && make test_reflect`
- CPU 20 threads + Valgrind (`0 bytes in 0 blocks`)
- Nix dual-GPU (`RT_USE_GPU=1 RT_MAX_FRAMES=5 ./rt scenes/mirror_demo.rt`, `GPU_EXIT:0`)
- Promote `develop → test → main`
