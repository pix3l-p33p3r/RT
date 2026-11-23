/* Standalone math test for UV mapping, checker alternation & bump normals. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

typedef struct s_vec
{
	float	x;
	float	y;
	float	z;
}	t_vec;

static void	expect(int ok, const char *msg)
{
	if (!ok)
	{
		fprintf(stderr, "FAIL: %s\n", msg);
		exit(1);
	}
	printf("ok: %s\n", msg);
}

static t_vec	v_norm(t_vec a)
{
	float	l = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);

	return ((t_vec){a.x / l, a.y / l, a.z / l});
}

static t_vec	v_cross(t_vec a, t_vec b)
{
	return ((t_vec){
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	});
}

static void	sphere_uv(t_vec p, float *u, float *v)
{
	t_vec	d = v_norm(p);

	*u = 0.5f + atan2f(d.y, d.x) / (2.0f * (float)M_PI);
	*v = 0.5f + asinf(fmaxf(-1.0f, fminf(1.0f, d.z))) / (float)M_PI;
}

static void	plane_uv(t_vec p, float *u, float *v)
{
	*u = p.x * 0.125f;
	*v = p.y * 0.125f;
	*u = *u - floorf(*u);
	*v = *v - floorf(*v);
}

static int	checker_parity(float u, float v)
{
	int	ix = (int)floorf(u * 8.0f);
	int	iy = (int)floorf(v * 8.0f);

	return ((ix + iy) & 1);
}

static t_vec	bump_normal(t_vec n, float u, float v, float bump)
{
	t_vec	ref = (fabsf(n.z) < 0.9f)
		? (t_vec){0.0f, 0.0f, 1.0f} : (t_vec){1.0f, 0.0f, 0.0f};
	t_vec	t = v_norm(v_cross(ref, n));
	t_vec	b = v_cross(n, t);
	float	f = 2.0f * (float)M_PI * 10.0f;
	float	du = cosf(u * f) * cosf(v * f) * bump;
	float	dv = -sinf(u * f) * sinf(v * f) * bump;
	t_vec	out = {
		n.x + t.x * du + b.x * dv,
		n.y + t.y * du + b.y * dv,
		n.z + t.z * du + b.z * dv
	};

	return (v_norm(out));
}

int	main(void)
{
	t_vec	pts[5] = {
		{1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, -0.7f, 0.3f}
	};
	float	u;
	float	v;
	int		i;

	for (i = 0; i < 5; i++)
	{
		sphere_uv(pts[i], &u, &v);
		expect(u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f,
			"sphere UV within [0,1]");
	}
	plane_uv((t_vec){-3.7f, 11.2f, 0.0f}, &u, &v);
	expect(u >= 0.0f && u < 1.0f && v >= 0.0f && v < 1.0f,
		"plane UV wrapped to [0,1)");
	expect(checker_parity(0.05f, 0.05f) != checker_parity(0.18f, 0.05f),
		"checker alternates across U");
	expect(checker_parity(0.05f, 0.05f) != checker_parity(0.05f, 0.18f),
		"checker alternates across V");
	expect(checker_parity(0.05f, 0.05f) == checker_parity(0.18f, 0.18f),
		"checker matches diagonally");
	t_vec	n0 = v_norm((t_vec){0.2f, -0.4f, 0.89f});
	t_vec	nb = bump_normal(n0, 0.12f, 0.34f, 0.25f);
	float	len = sqrtf(nb.x * nb.x + nb.y * nb.y + nb.z * nb.z);
	expect(fabsf(len - 1.0f) < 1e-5f, "bumped normal is unit length");
	expect(fabsf(nb.x - n0.x) + fabsf(nb.y - n0.y) + fabsf(nb.z - n0.z) > 1e-3f,
		"bump perturbs normal");
	t_vec	np = bump_normal((t_vec){0.0f, 0.0f, 1.0f}, 0.25f, 0.25f, 0.25f);
	float	lenp = sqrtf(np.x * np.x + np.y * np.y + np.z * np.z);
	expect(fabsf(lenp - 1.0f) < 1e-5f, "polar bumped normal is unit length");
	printf("all surface tests passed\n");
	return (0);
}
