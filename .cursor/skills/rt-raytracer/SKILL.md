---
name: rt-raytracer
description: Build, extend, debug, render, and validate the 42 RT C ray tracer (CPU pthreads + dual-GPU OpenCL, 42 Norminette, Valgrind 0-leak, Nix OpenCL devShell, and .rt scenes). Use when working on RT primitives, materials, OpenCL kernels, .rt scenes, or 42 RT defence prep.
---

# 42 RT Ray Tracer Workflow

Distilled engineering workflow, invariants, and gotchas for the `RT` C11 ray tracer (`pix3l-p33p3r/RT`).

## Architecture Snapshot

- **CPU path (`./rt`):** POSIX pthreads split scanlines across `nproc` workers (`Parallel/threads.c`) with zero-malloc 4×4 `gtfm` transforms (`Transformation/transfor_matrix.c`) and progressive jitter AA.
- **Dual-GPU path (`./rt-gpu`):** OpenCL 1.2 compute kernel (`gpu/kernels/trace.cl`) + per-GPU pthreads (`gpu/cl_{init,setup,scene,camera,render}.c`) splitting scanlines proportionally by Compute Units inside a Nix devShell (`flake.nix`, building separate `./rt_ocl` so `./rt` is never overwritten).
- **Display & Controls:** Persistent MLX frame buffer (`expose_hook` blits without re-tracing); live `WASD`/`QE`/Arrows camera, `V` Stereo/VR cycle, `G` GPU↔CPU toggle, `P` PPM export, `ESC`/`X` leak-free exit (`cleanup.c`).

## Hard Invariants (Never Violate)

1. **Coordinate frame:** `+X` right, `+Y` forward, **`-Z` UP**. Put floors at `z = 0`, objects at `z < 0`, cameras at `y < 0, z < 0`, and keep `C` `lookat` components in `[-1, 1]`.
2. **`gtfm` sentinel:** Intersection loops iterate `while (objects[++i].gtfm)`. Every object (including planes, triangles, disks, negative CSG spheres) MUST zero-init `obj = (t_object){0};` and assign a valid `gtfm`.
3. **Zero per-ray heap mallocs:** Never call `create_matrix`/`malloc` inside hit or shading loops (`apply_transform_vector` computes directly on `gtfm[0/1]->matrix`).
4. **42 Norminette on all `.c`/`.h`** (excluding `third_party/` and `tests/`): max 25 lines/func, 5 funcs/file, 4 args, 5 vars, 80 cols, tabs, no `for`, no ternary, no decl+assign, no inline comments inside functions/structs, `#include` at file start (`-D` flags belong in `Makefile`).
5. **OpenCL C (`gpu/kernels/trace.cl`):** Never name variables `local` or `surf` (reserved/macro conflicts under Mesa rusticl). Keep ray tracing iterative (no recursion in OpenCL 1.2) and call `trace_one_ray` from a single site so LLVM JIT compiles fast.
6. **Zero leaks & exit `0`:** Always exit via `close_hook(vars)` → `cleanup_vars(vars)`. Never wire `(void *)exit` directly to MLX event 17.

## Workflow: Adding a Feature (Shape / Material / Light)

```
Task Progress:
- [ ] 1. Write standalone C math test in tests/test_<feature>.c
- [ ] 2. Implement CPU code (Norminette-clean: <=5 funcs/file, <=25 lines/func)
- [ ] 3. Mirror feature in gpu/cl_internal.h, gpu/cl_scene.c, gpu/kernels/trace.cl
- [ ] 4. Wire parser (backwards-compatible optional trailing fields) + Makefile
- [ ] 5. Add showcase scene in scenes/<feature>_demo.rt (-Z up!)
- [ ] 6. Run validation gates (Norminette + unit tests + 19+ scenes + Valgrind + Dual-GPU)
- [ ] 7. Commit on develop, fast-forward promote test & main, push all three
```

## Validation Gates (Run Before Every Commit)

```bash
# 1. Norminette + build + all unit tests + all .rt scenes (CPU 20 threads)
./scripts/verify_all_scenes.sh

# 2. Valgrind leak audit (must report: 0 bytes in 0 blocks, 0 errors)
RT_MAX_FRAMES=1 RT_THREADS=4 valgrind --leak-check=full \
  --show-leak-kinds=definite,indirect ./rt scenes/<scene>.rt

# 3. Dual-GPU OpenCL smoke test (rebuilds rt_ocl if deleted)
rm -f rt_ocl && RT_MAX_FRAMES=3 ./rt-gpu scenes/<scene>.rt

# 4. Headless render export (for gallery / visual verification)
RT_MAX_FRAMES=8 RT_EXPORT_PATH=/tmp/out.ppm ./rt scenes/<scene>.rt
```

## Git Sync Protocol

Work on `develop`; after every verified task, push and fast-forward promote:

```bash
git push origin develop && \
git checkout test && git merge develop && git push origin test && \
git checkout main && git merge develop && git push origin main && \
git checkout develop
```

## Detailed Reference

For the full `.rt` grammar, primitive/material registry, Nix/OpenCL kernel-5.15 setup, and known debugging playbooks, see [reference.md](reference.md).
