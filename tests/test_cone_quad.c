/* Unit-cone side quadratic: x^2 + y^2 - z^2 = 0 (no MLX). */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define EPS 1e-4f

static int	cone_side_t(float px, float py, float pz,
		float dx, float dy, float dz, float *t0, float *t1)
{
	float	a;
	float	b;
	float	c;
	float	disc;
	float	s;

	a = dx * dx + dy * dy - dz * dz;
	b = 2.0f * (px * dx + py * dy - pz * dz);
	c = px * px + py * py - pz * pz;
	if (fabsf(a) < 1e-6f)
		return (0);
	disc = b * b - 4.0f * a * c;
	if (disc < 0.0f)
		return (0);
	s = sqrtf(disc);
	*t0 = (-b - s) / (2.0f * a);
	*t1 = (-b + s) / (2.0f * a);
	return (1);
}

static int	valid_side(float px, float py, float pz,
		float dx, float dy, float dz, float t)
{
	float	x;
	float	y;
	float	z;

	if (t <= 0.0f)
		return (0);
	x = px + t * dx;
	y = py + t * dy;
	z = pz + t * dz;
	return (fabsf(z) < 1.0f
		&& fabsf(x * x + y * y - z * z) < 1e-3f);
}

static void	expect(int ok, const char *msg)
{
	if (!ok)
	{
		fprintf(stderr, "FAIL: %s\n", msg);
		exit(1);
	}
	printf("ok: %s\n", msg);
}

int	main(void)
{
	float	t0;
	float	t1;

	/* Ray (0,0,-3)->+Z meets apex region t=3 at (0,0,0). */
	expect(cone_side_t(0, 0, -3, 0, 0, 1, &t0, &t1), "disc along +z");
	expect(fabsf(t0 - 3.0f) < EPS || fabsf(t1 - 3.0f) < EPS, "t≈3 apex");

	/* Ray from side should hit mantle with |z|<1. */
	expect(cone_side_t(3, 0, 0.5f, -1, 0, 0, &t0, &t1), "disc from +x");
	expect(valid_side(3, 0, 0.5f, -1, 0, 0, t0)
		|| valid_side(3, 0, 0.5f, -1, 0, 0, t1), "mantle |z|<1");

	/* Miss: ray parallel-ish outside. */
	expect(!cone_side_t(5, 5, 0, 0, 0, 1, &t0, &t1)
		|| (!valid_side(5, 5, 0, 0, 0, 1, t0)
			&& !valid_side(5, 5, 0, 0, 0, 1, t1)), "no false mantle");

	printf("all cone quad tests passed\n");
	return (0);
}
