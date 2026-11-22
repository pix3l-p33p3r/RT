---
title: pthread raytrace + OpenCL GPU path
status: ready
branch: develop
---

# Plan: Threads then GPU (OpenCL)

## Settled

- Threads first, GPU immediately after (`user-directed` / `user-approved`)
- OpenCL not CUDA (AMD Navi + Intel iGPU; no NVIDIA) (`user-approved`)
- Ship each finished slice: `develop → test → main` (`user-directed`)
- Final image still CPU/MLX blit — GPU only for compute (`subject`)

## Part A — Threads

1. Split image rows across `N = nproc` pthreads.
2. Per-thread RNG (`frames * k + tid`) — no shared `rng_state` races.
3. Pixel writes are disjoint → no locks on `image` / `buffer_img`.
4. `raytrace()` joins all workers then returns; `render()` unchanged.
5. Link `-pthread`.

## Part B — OpenCL (right after)

1. Vendor or install OpenCL C headers; link `-lOpenCL`.
2. Runtime device pick (GPU preferred, else CPU ICD / skip).
3. Host uploads scene floats + camera; kernel writes RGB float buffer.
4. v1 kernel: primary rays + sphere/plane/cylinder/cone hits + ambient/diffuse (match host enough to demo).
5. Fallback: if no ICD/device/build, keep pthread path; log once.
6. Toggle: env `RT_USE_GPU=1` or compile `RT_OPENCL=1`.

## Verify

- `make` + `./rt scenes/cone_demo.rt` (CPU threads)
- Valgrind/ helgrind optional later
- With GPU ICD: `RT_USE_GPU=1 ./rt scenes/cone_demo.rt`
- Update TODO checkboxes

## Out of scope this run

Reflection/glass, live editor, CUDA.
