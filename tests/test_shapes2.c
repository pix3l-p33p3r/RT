/* Standalone math test for Disk + Paraboloid primitives (cc -lm). */
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

static t_v3	vnorm(t_v3 a)
{
	float	l;

	l = sqrtf(vdot(a, a));
	if (l > 0.0f)
		return (vscale(a, 1.0f / l));
	return (a);
}

static int	disk_hit(t_v3 ro, t_v3 rd, t_v3 center, t_v3 nrm, float radius,
		t_v3 *hit_out, t_v3 *n_out)
{
	t_v3	axis;
	t_v3	hit;
	float	denom;
	float	t;

	axis = vnorm(nrm);
	denom = vdot(axis, rd);
	if (fabsf(denom) < EPS)
		return (0);
	t = vdot(vsub(center, ro), axis) / denom;
	if (t < EPS)
		return (0);
	hit = vadd(ro, vscale(rd, t));
	if (vdot(vsub(hit, center), vsub(hit, center)) > radius * radius)
		return (0);
	*hit_out = hit;
	*n_out = axis;
	if (vdot(*n_out, rd) > 0.0f)
		*n_out = vscale(*n_out, -1.0f);
	return (1);
}

static int	parab_hit(t_v3 ro, t_v3 rd, t_v3 *poi_out, t_v3 *n_out)
{
	t_v3	v;
	float	q[4];
	float	t[2];
	int		i;

	v = vnorm(rd);
	q[0] = v.x * v.x + v.y * v.y;
	q[1] = 2.0f * (ro.x * v.x + ro.y * v.y) - v.z;
	q[2] = ro.x * ro.x + ro.y * ro.y - ro.z;
	if (fabsf(q[0]) < EPS)
	{
		if (fabsf(q[1]) < EPS)
			return (0);
		t[0] = -q[2] / q[1];
		t[1] = -1.0f;
	}
	else
	{
		q[3] = q[1] * q[1] - 4.0f * q[0] * q[2];
		if (q[3] < 0.0f)
			return (0);
		q[3] = sqrtf(q[3]);
		t[0] = (-q[1] - q[3]) / (2.0f * q[0]);
		t[1] = (-q[1] + q[3]) / (2.0f * q[0]);
	}
	i = 0;
	while (i < 2)
	{
		if (t[i] > EPS)
		{
			*poi_out = vadd(ro, vscale(v, t[i]));
			if (poi_out->z >= 0.0f && poi_out->z <= 1.0f)
			{
				*n_out = vnorm(v3(2.0f * poi_out->x, 2.0f * poi_out->y, -1.0f));
				if (vdot(*n_out, v) > 0.0f)
					*n_out = vscale(*n_out, -1.0f);
				return (1);
			}
		}
		i++;
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

static void	test_disks(void)
{
	t_v3	h;
	t_v3	n;

	expect(disk_hit(v3(0, 0, -3), v3(0, 0, 1), v3(0, 0, 0), v3(0, 0, -1),
			1.0f, &h, &n), "disk center hit");
	expect(fabsf(h.z) < EPS && n.z < -0.99f, "disk center hitpoint + normal");
	expect(disk_hit(v3(0.9f, 0, -3), v3(0, 0, 1), v3(0, 0, 0), v3(0, 0, -1),
			1.0f, &h, &n), "disk inside rim hit");
	expect(!disk_hit(v3(1.1f, 0, -3), v3(0, 0, 1), v3(0, 0, 0), v3(0, 0, -1),
			1.0f, &h, &n), "disk outside radius miss");
	expect(disk_hit(v3(0, 0, 3), v3(0, 0, -1), v3(0, 0, 0), v3(0, 0, -1),
			1.0f, &h, &n), "disk back-side hit");
	expect(n.z > 0.99f, "disk double-sided normal flip");
	expect(!disk_hit(v3(0, 0, -3), v3(1, 0, 0), v3(0, 0, 0), v3(0, 0, -1),
			1.0f, &h, &n), "disk parallel ray miss");
	expect(!disk_hit(v3(0, 0, -3), v3(0, 0, -1), v3(0, 0, 0), v3(0, 0, -1),
			1.0f, &h, &n), "disk behind ray miss");
}

static void	test_paraboloids(void)
{
	t_v3	p;
	t_v3	n;
	t_v3	wn;
	t_v3	wt;

	expect(parab_hit(v3(-3, 0, 0.25f), v3(1, 0, 0), &p, &n),
		"paraboloid side hit");
	expect(fabsf(p.x + 0.5f) < EPS && fabsf(p.z - 0.25f) < EPS,
		"paraboloid closest root x=-0.5, z=0.25");
	expect(vdot(n, v3(1, 0, 0)) < 0.0f, "paraboloid outward normal faces ray");
	expect(parab_hit(v3(0, 0, 0.5f), v3(1, 0, 0), &p, &n),
		"paraboloid inside bowl hit");
	expect(vdot(n, v3(1, 0, 0)) < 0.0f && n.x < 0.0f,
		"paraboloid inside normal flipped inward");
	expect(parab_hit(v3(-1.2f, 0, 2.0f), v3(1.0f, 0, -1.0f), &p, &n),
		"paraboloid first root clipped, second root inside bowl");
	expect(p.z >= 0.0f && p.z <= 1.0f
		&& fabsf(p.x * p.x + p.y * p.y - p.z) < 1e-3f,
		"paraboloid clipped root satisfies surface equation");
	expect(parab_hit(v3(0.5f, 0, -2.0f), v3(0, 0, 1), &p, &n),
		"paraboloid linear (a==0) hit from below");
	expect(fabsf(p.z - 0.25f) < EPS && n.z < 0.0f,
		"paraboloid linear hitpoint z=0.25");
	expect(parab_hit(v3(0.5f, 0, 2.0f), v3(0, 0, -1), &p, &n),
		"paraboloid linear (a==0) hit from inside bowl");
	expect(fabsf(p.z - 0.25f) < EPS && n.z > 0.0f,
		"paraboloid linear inside normal flipped");
	expect(!parab_hit(v3(1.5f, 0, -2.0f), v3(0, 0, 1), &p, &n),
		"paraboloid linear z>1 clipped miss");
	expect(!parab_hit(v3(-3, 0, 1.5f), v3(1, 0, 0), &p, &n),
		"paraboloid above z=1 rim miss");
	expect(!parab_hit(v3(-3, 0, -1.0f), v3(1, 0, 0), &p, &n),
		"paraboloid below z=0 vertex miss");
	wn = vnorm(v3(2.0f * 0.5f / 2.0f, 0.0f, -1.0f / 1.0f));
	wt = v3(2.0f * 1.0f, 0.0f, 1.0f * 2.0f * 0.5f);
	expect(fabsf(vdot(wn, wt)) < EPS,
		"paraboloid inverse-transpose normal orthogonal to scaled tangent");
}

int	main(void)
{
	test_disks();
	test_paraboloids();
	printf("all disk + paraboloid tests passed\n");
	return (0);
}
