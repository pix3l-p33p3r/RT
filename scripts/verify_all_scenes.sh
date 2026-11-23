#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

echo "=== 1. Norminette ==="
if norminette Image Material Matrix Objects Parallel Transformation gpu libft parsing \
  camera.c cleanup.c controls.c main.c main.h raytrace.c raytrace_utils.c \
  vector.c vector1.c vector2.c | grep -v ': OK!'; then
  exit 1
fi
echo "Norminette: 100% OK"

echo "=== 2. Build + 7 Unit Tests ==="
make re >/dev/null
make test_cone test_reflect test_refract test_surface test_shapes2 test_csg_torus test_stereo >/dev/null
echo "All 7 unit tests: OK"

echo "=== 3. Smoke-test all .rt scenes (CPU 20 threads) ==="
count=0
for f in scenes/*.rt; do
  RT_MAX_FRAMES=1 ./rt "$f" >/dev/null
  count=$((count + 1))
done
echo "All $count scenes rendered cleanly on CPU (exit 0)"
