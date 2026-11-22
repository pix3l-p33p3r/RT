---
title: MLX Expose redraw without full re-trace
status: ready
branch: develop
---

# Plan: Expose redraw (RT mandatory)

## Outcome

Window expose/uncover redraws the last computed frame by blitting the existing MLX image — **no** `raytrace()` call.

## Settled (session)

- C fork of mini-RT (`user-directed`)
- After each task: commit on `develop`, push, fast-forward `test` then `main` (`user-directed`)
- Cone done; next mandatory unlock is expose (`user-approved`)

## Design

1. Keep a **persistent** `t_data` MLX image on `t_vars` (created once after window init).
2. `render()` writes pixels into that image and `mlx_put_image_to_window` — do not destroy the image each frame.
3. Add `expose_hook` → only `mlx_put_image_to_window` (guard if image missing / zero frames).
4. Register: `mlx_hook(win, 12, 1L << 15, expose_hook, &vars)` (Expose / ExposureMask).
5. Update `.cursor/rules/git-sync.mdc` + README: after each task `develop → test → main`.
6. Tick TODO.md expose row.

## Files

- `main.h` — `t_data *mlx_img` (or reuse existing); declare `expose_hook`
- `main.c` — create image once; hooks; expose handler
- `Image/image_utils.c` — `render` uses persistent image
- `TODO.md`, `README.md`, `.cursor/rules/git-sync.mdc`

## Verify

- `make` succeeds
- `./rt scenes/cone_demo.rt` runs
- Manual: cover/uncover window — image returns without hitching a full re-trace (expose path has no `raytrace`)
- Push: `develop` then FF `test` + `main`

## Out of scope

Demo scenes VI.1–3, Valgrind pass, options.
