# RT — path to 125

Base: **mini-RT bonus** fork → binary `rt`.  
Rule: **mandatory = 0 pts**. Options/bonuses only count if mandatory is **perfect**. Cap **125**.

Legend: `[x]` already in mini-RT base · `[ ]` still to build · `[~]` partial / needs hardening for RT defence

---

## 0. Unlock rule (do this first)

- [ ] Mandatory 100% green (no crashes, no leaks, live demos work)
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
| No memory leaks | [~] | re-check on RT (Valgrind) |

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

- [ ] Parallel / directional light
- [ ] Soft / area lights (exotic but strong)
- [ ] Colored lights polish + demo scenes

### Materials / optics

- [ ] **Reflection** (true recursive mirror rays — not only Phong)
- [ ] **Transparency** / refraction
- [ ] Shadow modified by transparent objects
- [ ] Limit reflection / transparency / disruption by texture

### Surfaces

- [ ] Textures (image maps)
- [ ] Bump mapping
- [ ] Colour disruption (procedural noise / checker / etc.)

### Shapes

- [ ] Limited objects: disk, parallelogram, half-sphere, tube, …
- [ ] Composed elements: cube, pyramid, tetrahedron, …
- [ ] Negative elements (CSG subtract)
- [ ] More natives: paraboloid, hyperboloid, torus, …

### Exotic / 125 fuel (subject + “unlisted” eval points)

- [x] **Multithreading (pthread)** — row bands across `nproc` threads (default path)
- [x] **GPU compute (OpenCL)** — `RT_USE_GPU=1`; needs ICD (`mesa-opencl-icd` / `intel-opencl-icd`). Falls back to threads if no device.
- [ ] Distributed render over multiple machines
- [ ] Quadrics / quartics
- [ ] Video clips from your frames
- [ ] Stereoscopic / VR pair
- [ ] POV / 3DS import (**but** mandatory shapes must stay equation-based, not mesh)
- [ ] Live in-program scene editor (camera, lights, move objects)
- [ ] Export PNG/JPEG of renders (nice UX; still must live-render at defence)

---

## 3. Bonuses (subject Ch.VII → up to 125)

Eval sheet bonus slots for “outstanding” extras. Treat as: **depth + polish + wow demos** on top of a large options set.

Suggested 125 stack (pick and finish hard, don’t half-do 20):

1. [ ] Perfect mandatory + 3 subject scenes  
2. [ ] Reflection + transparency + transparent shadows  
3. [ ] Textures + bump  
4. [ ] ≥3 extra shapes (disk, torus, cube/pyramid, …)  
5. [ ] Parallel light + negative/CSG or composed solids  
6. [ ] Threads + GPU + live camera/object controls  
7. [ ] Defence pack: 8–12 polished `.rt` scenes, one feature each  

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
3. [~] Scenes VI.1 / VI.2 / VI.3 added — Valgrind pass still open  
4. [x] **pthread row workers**  
5. [x] **OpenCL GPU path** (enable with `RT_USE_GPU=1` + ICD)  
6. [ ] Recursive reflection  
7. [ ] Transparency + shadow through glass  
8. [ ] Textures + bump  
9. [ ] Extra shapes + parallel light  
10. [ ] Bonus polish until 125 demos feel undeniable  

---

## Quick gap vs mini-RT (one glance)

**Must build for mandatory unlock:** cone · expose redraw · 3 demo scenes · leak audit  

**Already have as options seed:** ambient · `.rt` files · multi-spot · shadows · shine · progressive frames  

**Must build for a real mark / 125:** mirror · glass · textures/bump · more shapes · parallel light · CSG/composed · perf · live edit · defence scenes  
