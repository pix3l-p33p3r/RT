---
title: Fix 4 bugs, leak-free exit, and per-GPU + CPU pthreads
status: ready
branch: develop
---

# Plan: Bug fixes, Valgrind clean exit, and GPU+CPU threads

## Fixes

1. `main.c` / `main.h` — add `cleanup_vars()` + `close_hook(t_vars *)` (ESC & X button exit `0` cleanly, zero leaks); support `RT_MAX_FRAMES` for automated tests.
2. `Material/Diffuse.c` — fix double `norm.i++` in shadow `cast_ray`.
3. `parsing/check_elements.c` — remove duplicate `/255` on ambient color.
4. `gpu/kernels/trace.cl` — match CPU Lambert diffuse (`ndl * intensity`).
5. `Parallel/threads.c` + `gpu/cl_render.c` — `RT_THREADS` env + startup log; spawn one pthread per GPU to enqueue, read back, and accumulate each GPU's row band in parallel.

## Verify

- `make re && make test_cone`
- `RT_MAX_FRAMES=2 valgrind --leak-check=full ./rt scenes/cone_demo.rt` (exit `0`, 0 definite leaks)
- `nix-enter nix develop`: `RT_USE_GPU=1 RT_MAX_FRAMES=5 ./rt scenes/cone_demo.rt` (exit `0`, both GPUs active)
- Push `develop → test → main`
