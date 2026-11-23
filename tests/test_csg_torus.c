/* Standalone math test for Torus SDF + Negative CSG Sphere Carving (cc -lm). */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define EPS 1e-4f

typedef struct s_v3
{
	float	x;
	float	y;
	float	z;
}	t_v3;

typedef struct s_sph
{
	t_v3	c;
	float	r;
	int		is_neg;
}	t_sph;

static t_v3	v3(float x, float y, float z)
{
	t_v3	r;

	r.x = x;
	r.y = y;
	r.z = z;
	return (r);
}

static t_v3	vsub(t_v3 a, t_v3 b)
{
	return (v3(a.x - b.x, a.y - b.y, a.z - b.z));
}

static t_v3	vadd(t_v3 a, t_v3 b)
{
	return (v3(a.x + b.x, a.y + b.y, a.z + b.z));
}

static t_v3	vscale(t_v3 a, float s)
{
	return (v3(a.x * s, a.y * s, a.z * s));
}

static float	vdot(t_v3 a, t_v3 b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

static float	vlen(t_v3 a)
{
	return (sqrtf(vdot(a, a)));
}

static t_v3	vnorm(t_v3 a)
{
	float	l;

	l = vlen(a);
	if (l > 0.0f)
		return (vscale(a, 1.0f / l));
	return (a);
}

static float	torus_sdf(t_v3 p, float maj, float min)
{
	float	q;

	q = sqrtf(p.x * p.x + p.y * p.y) - maj;
	return (sqrtf(q * q + p.z * p.z) - min);
}

static int	torus_hit(t_v3 ro, t_v3 rd, float maj, float min,
		t_v3 *hit_out, t_v3 *n_out)
{
	t_v3	d;
	float	rb;
	float	b;
	float	disc;
	float	t0;
	float	t1;
	int		step;

	d = vnorm(rd);
	rb = maj + min + 0.01f;
	b = vdot(ro, d);
	disc = b * b - (vdot(ro, ro) - rb * rb);
	if (disc < 0.0f)
		return (0);
	t0 = -b - sqrtf(disc);
	t1 = -b + sqrtf(disc);
	if (t1 < EPS)
		return (0);
	if (t0 < EPS)
		t0 = EPS;
	step = 0;
	while (step < 96 && t0 <= t1)
	{
		t_v3	p;
		float	sd;

		p = vadd(ro, vscale(d, t0));
		sd = torus_sdf(p, maj, min);
		if (fabsf(sd) < 0.0005f)
		{
			float	q;
			float	f;

			q = sqrtf(p.x * p.x + p.y * p.y);
			if (q < 1e-6f)
				q = 1e-6f;
			f = 1.0f - maj / q;
			*hit_out = p;
			*n_out = vnorm(v3(p.x * f, p.y * f, p.z));
			if (vdot(*n_out, d) > 0.0f)
				*n_out = vscale(*n_out, -1.0f);
			return (1);
		}
		t0 += fabsf(sd);
		step++;
	}
	return (0);
}

static int	sphere_hit(t_v3 ro, t_v3 rd, t_sph s, t_v3 *hit_out, t_v3 *n_out)
{
	t_v3	d;
	t_v3	oc;
	float	b;
	float	disc;
	float	t;

	d = vnorm(rd);
	oc = vsub(ro, s.c);
	b = vdot(oc, d);
	disc = b * b - (vdot(oc, oc) - s.r * s.r);
	if (disc < 0.0f)
		return (0);
	t = -b - sqrtf(disc);
	if (t <= EPS)
		t = -b + sqrtf(disc);
	if (t <= EPS)
		return (0);
	*hit_out = vadd(ro, vscale(d, t));
	*n_out = vnorm(vsub(*hit_out, s.c));
	return (1);
}

static int	csg_carve(t_v3 ro, t_v3 rd, t_sph solid, t_sph neg,
		t_v3 *hit, t_v3 *nrm)
{
	t_v3	d;
	t_v3	oc;
	float	b;
	float	disc;
	float	t_exit;
	t_v3	ex;

	if (vlen(vsub(*hit, neg.c)) >= neg.r)
		return (1);
	d = vnorm(rd);
	oc = vsub(ro, neg.c);
	b = vdot(oc, d);
	disc = b * b - (vdot(oc, oc) - neg.r * neg.r);
	if (disc < 0.0f)
		return (0);
	t_exit = -b + sqrtf(disc);
	if (t_exit <= EPS)
		return (0);
	ex = vadd(ro, vscale(d, t_exit));
	if (vlen(vsub(ex, solid.c)) < solid.r)
	{
		*hit = ex;
		*nrm = vnorm(vsub(neg.c, ex));
		return (1);
	}
	return (0);
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

static void	test_torus_cases(void)
{
	t_v3	h;
	t_v3	n;

	expect(torus_hit(v3(-5, 0, 0), v3(1, 0, 0), 2.0f, 0.5f, &h, &n),
		"torus outer equator hit");
	expect(fabsf(h.x + 2.5f) < 1e-3f && fabsf(h.y) < 1e-3f
		&& fabsf(h.z) < 1e-3f, "torus outer equator hitpoint (-2.5,0,0)");
	expect(n.x < -0.99f && fabsf(n.y) < 1e-3f && fabsf(n.z) < 1e-3f,
		"torus outer equator outward normal (-1,0,0)");
	expect(!torus_hit(v3(0, 0, -5), v3(0, 0, 1), 2.0f, 0.5f, &h, &n),
		"torus central hole miss");
	expect(torus_hit(v3(2.0f, 0, -5), v3(0, 0, 1), 2.0f, 0.5f, &h, &n),
		"torus top tube hit along Z");
	expect(fabsf(h.z + 0.5f) < 1e-3f && n.z < -0.99f,
		"torus top tube hitpoint z=-0.5 and normal (0,0,-1)");
	expect(torus_hit(v3(0, 0, 0), v3(1, 0, 0), 2.0f, 0.5f, &h, &n),
		"torus inner hole wall hit from origin");
	expect(fabsf(h.x - 1.5f) < 1e-3f && n.x < -0.99f,
		"torus inner hole hitpoint x=1.5 and normal (-1,0,0)");
	expect(!torus_hit(v3(0, 5, 0), v3(1, 0, 0), 2.0f, 0.5f, &h, &n),
		"torus bounding sphere miss");
	expect(!torus_hit(v3(-5, 0, 0.8f), v3(1, 0, 0), 2.0f, 0.5f, &h, &n),
		"torus grazing above tube miss");
	expect(!torus_hit(v3(5, 0, 0), v3(1, 0, 0), 2.0f, 0.5f, &h, &n),
		"torus behind ray miss");
	expect(torus_hit(v3(2.0f, -4.0f, -1.0f), v3(0.0f, 4.0f, 1.0f),
			2.0f, 0.5f, &h, &n), "torus oblique ray hit");
	expect(fabsf(torus_sdf(h, 2.0f, 0.5f)) < 1e-3f
		&& fabsf(vlen(n) - 1.0f) < 1e-3f
		&& vdot(n, vnorm(v3(0.0f, 4.0f, 1.0f))) < 0.0f,
		"torus oblique hit on surface with unit facing normal");
}

static void	test_csg_cases(void)
{
	t_sph	solid;
	t_sph	neg;
	t_sph	big_neg;
	t_v3	h;
	t_v3	n;

	solid = (t_sph){v3(0, 0, 0), 2.0f, 0};
	neg = (t_sph){v3(0, -1.8f, 0), 1.0f, 1};
	expect(sphere_hit(v3(0, -5, 1.5f), v3(0, 1, 0), solid, &h, &n),
		"solid sphere uncarved region hit");
	expect(csg_carve(v3(0, -5, 1.5f), v3(0, 1, 0), solid, neg, &h, &n),
		"csg preserves hit outside negative sphere");
	expect(h.y < -1.2f && n.y < 0.0f,
		"uncarved hitpoint and outward normal unchanged");
	expect(sphere_hit(v3(0, -5, 0), v3(0, 1, 0), solid, &h, &n),
		"solid sphere front hit inside negative sphere");
	expect(csg_carve(v3(0, -5, 0), v3(0, 1, 0), solid, neg, &h, &n),
		"csg death-star crater returns concave wall hit");
	expect(fabsf(h.y + 0.8f) < 1e-3f && fabsf(h.x) < 1e-3f,
		"csg exit point matches neg sphere far wall y=-0.8");
	expect(n.y < -0.99f && fabsf(n.x) < 1e-3f && fabsf(n.z) < 1e-3f,
		"csg concave normal points inward toward neg center (0,-1,0)");
	expect(sphere_hit(v3(0.5f, -5, 0), v3(0, 1, 0), solid, &h, &n),
		"off-axis crater front hit");
	expect(csg_carve(v3(0.5f, -5, 0), v3(0, 1, 0), solid, neg, &h, &n),
		"off-axis crater concave wall hit");
	expect(fabsf(h.y - (-1.8f + sqrtf(0.75f))) < 1e-3f
		&& fabsf(vlen(vsub(h, neg.c)) - neg.r) < 1e-3f
		&& vdot(n, v3(0, 1, 0)) < 0.0f
		&& n.x < -0.49f,
		"off-axis concave exit point and inward normal exact");
	big_neg = (t_sph){v3(0, -0.5f, 0), 3.0f, 1};
	expect(sphere_hit(v3(0, -5, 0), v3(0, 1, 0), solid, &h, &n),
		"through-hole initial solid hit");
	expect(!csg_carve(v3(0, -5, 0), v3(0, 1, 0), solid, big_neg, &h, &n),
		"csg through-hole sees empty space when exit is outside solid");
}

int	main(void)
{
	test_torus_cases();
	test_csg_cases();
	printf("all torus + csg carving tests passed\n");
	return (0);
}
