# RT — path to 125

Base: **mini-RT bonus** fork → binary `rt`.  
Rule: **mandatory = 0 pts**. Options/bonuses only count if mandatory is **perfect**. Cap **125**.

Legend: `[x]` already in mini-RT base · `[ ]` still to build · `[~]` partial / needs hardening for RT defence

---

## 0. Unlock rule (do this first)

- [x] Mandatory 100% green (no crashes, no leaks, live demos work)
- [ ] Enough options ready to **show live** (PNG/JPEG alone do **not** count)
- [ ] Own scene tools: `.rt` files and/or in-app edits (no public third-party scene tools for defence)

---

## 1. Mandatory (0 pts, but blocks everything)

| Item | Status | Notes |
|------|--------|--------|
| C / C++ / Rust | [x] | C fork |
| Ray tracing image | [x] | |
| **Plane** | [x] | |
| **Sphere** | [x] | |
| **Cylinder** | [x] | |
| **Cone** | [x] | unit cone `x²+y²=z²`, parse `cn` (same layout as `cy`) |
| Translation | [x] | GTFM / `set_transform` |
| Rotation | [x] | |
| Camera pos + direction easy to change | [x] | via `.rt` `C` |
| Redraw **without** full recalculate (e.g. Expose) | [x] | persistent `mlx_img` + Expose hook blits only |
| Light brightness | [x] | |
| Shadows | [x] | |
| Multi-spot | [x] | light array + bonus RGB lights |
| Shine effect | [x] | Phong specular (not mirror) |
| Executable named `rt` | [x] | |
| No memory leaks | [x] | Valgrind verified: `0 bytes in 0 blocks`, `0 errors` on clean exit |

### Mandatory demo scenes (subject strongly advises)

- [x] **Fig VI.1** — `scenes/vi1_four_shapes.rt`
- [x] **Fig VI.2** — `scenes/vi2_other_view.rt`
- [x] **Fig VI.3** — `scenes/vi3_shadow_mix.rt`

---

## 2. Options (real grade — subject list)

Bankable from mini-RT already:

- [x] Ambiance light (`A`)
- [x] External scene description files (`.rt`)
- [~] Direct / point lights (`L`) — present; name + show clearly in defence
- [~] Progressive AA / multi-frame accumulation — present in loop; count as extra if you defend it

Still to implement for a solid options pack:

### Lighting

- [x] Parallel / directional light (`D dx,dy,dz intensity R,G,B` — `scenes/shapes_demo.rt`)
- [ ] Soft / area lights (exotic but strong)
- [x] Colored lights polish + demo scenes (`scenes/vi3_shadow_mix.rt`, `scenes/prism_vortex.rt`)

### Materials / optics

- [x] **Reflection** (true recursive mirror rays — CPU `trace_ray` + GPU 5-bounce loop, optional `mirror` `[0..1]`, `scenes/mirror_demo.rt`)
- [x] **Transparency** / refraction (Snell's law + TIR + inside-sphere exit, optional `transp ior`, `scenes/glass_demo.rt`)
- [x] **Shadow modification according to transparency** (tinted glass shadows on CPU + dual-GPU)
- [ ] Limit reflection / transparency / disruption by texture

### Surfaces

- [x] Textures (PPM P6/P3 image maps — `Material/ppm.c`, `assets/earth_grid.ppm`)
- [x] Bump mapping (tangent-frame normal ripple perturbation — `Material/surface.c` + GPU)
- [x] Colour disruption (procedural checker, stripe, marble — `scenes/surfaces_demo.rt`)

### Shapes

- [x] Limited objects: half-sphere (`hs` — `scenes/pokeball.rt`), disk (`dk` — `scenes/shapes_demo.rt`)
- [x] Composed elements: triangles (`tr`), pyramids (`scenes/giza.rt`), 3D 42 logo (`scenes/logo_42.rt`), 3D NixOS logo (`scenes/nixos_logo.rt`)
- [x] Chromatic dispersion (RGB IOR split for `ior >= 1.7` — `scenes/prism_vortex.rt`, `scenes/liquid_snowflake.rt`)
- [ ] Negative elements (CSG subtract)
- [x] More natives: paraboloid (`pb` — `Objects/Paraboloid.c` + GPU, `scenes/shapes_demo.rt`)

### Exotic / 125 fuel (subject + “unlisted” eval points)

- [x] **Multithreading (pthread)** — row bands across `nproc` threads (default path)
- [x] **GPU compute (OpenCL dual-GPU)** — `RT_USE_GPU=1` inside `nix-enter nix develop`; splits rows across AMD RX 6500 + Intel UHD 770 weighted by CUs. Falls back to threads if no device.
- [ ] Distributed render over multiple machines
- [ ] Quadrics / quartics
- [ ] Video clips from your frames
- [ ] Stereoscopic / VR pair
- [ ] POV / 3DS import (**but** mandatory shapes must stay equation-based, not mesh)
- [x] Live in-program scene editor / controls (`controls.c` — WASD/QE move, arrows yaw/pitch, `G` GPU/CPU toggle, `P` PPM export)
- [x] Export PPM of renders (`Image/export_ppm.c` — press `P` → `render_out.ppm`)

---

## 3. Bonuses (subject Ch.VII → up to 125)

Eval sheet bonus slots for “outstanding” extras. Treat as: **depth + polish + wow demos** on top of a large options set.

Suggested 125 stack (pick and finish hard, don’t half-do 20):

1. [x] Perfect mandatory + 3 subject scenes  
2. [x] Reflection + transparency + transparent shadows  
3. [x] Textures + bump + procedural disruption  
4. [x] ≥3 extra shapes (`tr`, `hs`, `dk`, `pb` + pyramids/3D logos)  
5. [x] Parallel light (`D`) + composed solids + dispersion  
6. [x] Threads + dual-GPU + live camera controls + PPM export  
7. [x] Defence pack: 14 polished `.rt` scenes ready to live-render  

---

## 4. Defence / submission checklist

- [ ] Only work in this Git repo is evaluated  
- [ ] Demo **every** claimed option live  
- [ ] Multiple preconfigured scenes ready to calculate  
- [ ] Team can explain design (AI used = disclose)  
- [ ] Invite accepted: `@0xakhii` collaborator  

---

## 5. Suggested build order

1. [x] Cone + `cn` parsing + scene with 4 shapes  
2. [x] Expose / buffer blit (no full re-trace)  
3. [x] Scenes VI.1 / VI.2 / VI.3 + Valgrind pass (`0 bytes in 0 blocks`)  
4. [x] **pthread row workers**  
5. [x] **OpenCL GPU path** (enable with `RT_USE_GPU=1` + ICD)  
6. [x] Recursive reflection (`scenes/mirror_demo.rt`)  
7. [x] Transparency + refraction + glass shadows (`scenes/glass_demo.rt`)  
8. [x] Textures + bump + color disruption (`scenes/surfaces_demo.rt`)  
9. [x] Extra shapes (`dk`, `pb`) + parallel light (`D`) (`scenes/shapes_demo.rt`) + live controls (`controls.c`)  
10. [ ] Final defence rehearsal / optional extra exotic bonus (CSG/torus)  

---

## Quick gap vs mini-RT (one glance)

**Mandatory unlock:** 100% green (cone · expose · VI.1–3 scenes · 0-byte Valgrind pass)  

**Already have as options seed:** ambient · `.rt` files · multi-spot · shadows · shine · progressive frames  

**Must build for a real mark / 125:** mirror · glass · textures/bump · more shapes · parallel light · CSG/composed · perf · live edit · defence scenes  
