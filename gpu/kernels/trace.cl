/* OpenCL 1.2 multi-GPU ray tracer (sp/pl/cy/cn + shadows + Phong + jitter). */

#define V3(a) ((float3)((a)[0], (a)[1], (a)[2]))

typedef struct {
	int		type;
	float	pos[3];
	float	axis[3];
	float	v2[3];
	float	color[3];
	float	radius;
	float	height;
	float	shininess;
	float	reflectivity;
	float	mirror;
	float	transp;
	float	ior;
} GpuObj;

typedef struct {
	float	pos[3];
	float	color[3];
	float	intensity;
} GpuLight;

typedef struct {
	float	origin[3];
	float	center[3];
	float	u[3];
	float	v[3];
	float	horizontal;
	float	aspect;
} GpuCam;

static uint	pcg_hash(uint v)
{
	uint	state;
	uint	word;

	state = v * 747796405u + 2891336453u;
	word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return ((word >> 22u) ^ word);
}

static float	rand_u(uint *seed)
{
	*seed = pcg_hash(*seed);
	return ((float)(*seed) * (1.0f / 4294967295.0f));
}

static float	dot3(float3 a, float3 b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

static float3	norm3(float3 a)
{
	float	l;

	l = sqrt(dot3(a, a));
	if (l < 1e-8f)
		return (a);
	return (a / l);
}

static int	hit_sphere(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	pos;
	float3	oc;
	float	b;
	float	c;
	float	disc;
	float	t0;

	pos = V3(o.pos);
	oc = ro - pos;
	b = 2.0f * dot3(oc, rd);
	c = dot3(oc, oc) - o.radius * o.radius;
	disc = b * b - 4.0f * c;
	if (disc < 0.0f)
		return (0);
	t0 = (-b - sqrt(disc)) * 0.5f;
	if (t0 < 1e-3f)
		t0 = (-b + sqrt(disc)) * 0.5f;
	if (t0 < 1e-3f)
		return (0);
	*t = t0;
	*n = norm3((ro + rd * t0) - pos);
	return (1);
}

static int	hit_plane(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	axis;
	float	denom;
	float	d;
	float	tt;

	axis = norm3(V3(o.axis));
	denom = dot3(axis, rd);
	if (fabs(denom) < 1e-6f)
		return (0);
	d = -dot3(axis, V3(o.pos));
	tt = (-d - dot3(ro, axis)) / denom;
	if (tt < 1e-3f)
		return (0);
	*t = tt;
	*n = axis;
	if (dot3(*n, rd) > 0.0f)
		*n = -*n;
	return (1);
}

static int	hit_cylinder(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	pos;
	float3	ca;
	float3	oc;
	float	card;
	float	caoc;
	float	a;
	float	b;
	float	c;
	float	disc;
	float	s;
	float	roots[2];
	float	best;
	float3	best_n;
	int		found;
	int		k;

	pos = V3(o.pos);
	ca = norm3(V3(o.axis));
	oc = ro - pos;
	card = dot3(ca, rd);
	caoc = dot3(ca, oc);
	best = 1e30f;
	found = 0;
	a = dot3(rd, rd) - card * card;
	b = 2.0f * (dot3(rd, oc) - card * caoc);
	c = dot3(oc, oc) - caoc * caoc - o.radius * o.radius;
	disc = b * b - 4.0f * a * c;
	if (disc >= 0.0f && fabs(a) > 1e-8f)
	{
		s = sqrt(disc);
		roots[0] = (-b - s) / (2.0f * a);
		roots[1] = (-b + s) / (2.0f * a);
		for (k = 0; k < 2; k++)
		{
			if (roots[k] > 1e-3f && roots[k] < best)
			{
				float3 p = ro + rd * roots[k];
				float h = dot3(p - pos, ca);
				if (fabs(h) <= o.height)
				{
					best = roots[k];
					best_n = norm3(p - pos - ca * h);
					found = 1;
				}
			}
		}
	}
	if (fabs(card) > 1e-6f)
	{
		float cap_h[2] = { o.height, -o.height };
		for (k = 0; k < 2; k++)
		{
			float3 cc = pos + ca * cap_h[k];
			float tc = dot3(cc - ro, ca) / card;
			if (tc > 1e-3f && tc < best)
			{
				float3 p = ro + rd * tc;
				float3 d = p - cc;
				if (dot3(d, d) <= o.radius * o.radius)
				{
					best = tc;
					best_n = (cap_h[k] > 0.0f) ? ca : -ca;
					found = 1;
				}
			}
		}
	}
	if (!found)
		return (0);
	*t = best;
	*n = best_n;
	return (1);
}

static int	hit_cone(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	pos;
	float3	ca;
	float3	oc;
	float	ratio;
	float	k2;
	float	card;
	float	caoc;
	float	a;
	float	b;
	float	c;
	float	disc;
	float	s;
	float	roots[2];
	float	best;
	float3	best_n;
	int		found;
	int		k;

	pos = V3(o.pos);
	ca = norm3(V3(o.axis));
	oc = ro - pos;
	ratio = o.radius / fmax(o.height, 1e-4f);
	k2 = ratio * ratio;
	card = dot3(ca, rd);
	caoc = dot3(ca, oc);
	best = 1e30f;
	found = 0;
	a = dot3(rd, rd) - (1.0f + k2) * card * card;
	b = 2.0f * (dot3(rd, oc) - (1.0f + k2) * card * caoc);
	c = dot3(oc, oc) - (1.0f + k2) * caoc * caoc;
	disc = b * b - 4.0f * a * c;
	if (disc >= 0.0f && fabs(a) > 1e-8f)
	{
		s = sqrt(disc);
		roots[0] = (-b - s) / (2.0f * a);
		roots[1] = (-b + s) / (2.0f * a);
		for (k = 0; k < 2; k++)
		{
			if (roots[k] > 1e-3f && roots[k] < best)
			{
				float3 p = ro + rd * roots[k];
				float h = dot3(p - pos, ca);
				if (fabs(h) <= o.height)
				{
					best = roots[k];
					best_n = norm3(p - pos - ca * h * (1.0f + k2));
					found = 1;
				}
			}
		}
	}
	if (fabs(card) > 1e-6f)
	{
		float cap_h[2] = { o.height, -o.height };
		for (k = 0; k < 2; k++)
		{
			float3 cc = pos + ca * cap_h[k];
			float tc = dot3(cc - ro, ca) / card;
			if (tc > 1e-3f && tc < best)
			{
				float3 p = ro + rd * tc;
				float3 d = p - cc;
				if (dot3(d, d) <= o.radius * o.radius)
				{
					best = tc;
					best_n = (cap_h[k] > 0.0f) ? ca : -ca;
					found = 1;
				}
			}
		}
	}
	if (!found)
		return (0);
	*t = best;
	*n = best_n;
	return (1);
}

static float3	glass_tint3(float3 base_c)
{
	return ((float3)(0.35f + 0.65f * base_c.x,
			0.35f + 0.65f * base_c.y,
			0.35f + 0.65f * base_c.z));
}

static int	hit_triangle(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	v0 = V3(o.pos);
	float3	v1 = V3(o.axis);
	float3	v2 = V3(o.v2);
	float3	e1 = v1 - v0;
	float3	e2 = v2 - v0;
	float3	h = cross(rd, e2);
	float	a = dot3(e1, h);
	if (fabs(a) < 1e-6f)
		return (0);
	float	f = 1.0f / a;
	float3	s = ro - v0;
	float	u = f * dot3(s, h);
	if (u < 0.0f || u > 1.0f)
		return (0);
	float3	q = cross(s, e1);
	float	v = f * dot3(rd, q);
	if (v < 0.0f || u + v > 1.0f)
		return (0);
	float	tt = f * dot3(e2, q);
	if (tt < 1e-3f)
		return (0);
	*t = tt;
	*n = norm3(cross(e1, e2));
	if (dot3(*n, rd) > 0.0f)
		*n = -*n;
	return (1);
}

static int	hit_half_sphere(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	pos = V3(o.pos);
	float3	axis = norm3(V3(o.axis));
	float3	oc = ro - pos;
	float	b = 2.0f * dot3(oc, rd);
	float	c = dot3(oc, oc) - o.radius * o.radius;
	float	disc = b * b - 4.0f * c;
	if (disc < 0.0f)
		return (0);
	float	sd = sqrt(disc);
	float	roots[2] = { (-b - sd) * 0.5f, (-b + sd) * 0.5f };
	for (int k = 0; k < 2; k++)
	{
		if (roots[k] > 1e-3f)
		{
			float3 p = ro + rd * roots[k];
			if (dot3(p - pos, axis) >= -1e-4f)
			{
				*t = roots[k];
				*n = norm3(p - pos);
				return (1);
			}
		}
	}
	return (0);
}

static int	intersect(float3 ro, float3 rd, __global const GpuObj *objs,
		int nobj, float *tmin, float3 *n, float3 *col,
		float *shininess, float *reflectivity, float *mirror,
		float *transp, float *ior, int *oid)
{
	int		i;
	float	t;
	float3	nn;
	int		hit;

	*tmin = 1e30f;
	*oid = -1;
	*mirror = 0.0f;
	*transp = 0.0f;
	*ior = 1.0f;
	hit = 0;
	for (i = 0; i < nobj; i++)
	{
		GpuObj o = objs[i];
		t = 1e30f;
		if (o.type == 1 && hit_sphere(ro, rd, o, &t, &nn))
			;
		else if (o.type == 2 && hit_plane(ro, rd, o, &t, &nn))
			;
		else if (o.type == 3 && hit_cylinder(ro, rd, o, &t, &nn))
			;
		else if (o.type == 4 && hit_cone(ro, rd, o, &t, &nn))
			;
		else if (o.type == 5 && hit_triangle(ro, rd, o, &t, &nn))
			;
		else if (o.type == 6 && hit_half_sphere(ro, rd, o, &t, &nn))
			;
		else
			continue ;
		if (t < *tmin)
		{
			*tmin = t;
			*n = nn;
			*col = V3(o.color);
			*shininess = o.shininess;
			*reflectivity = o.reflectivity;
			*mirror = o.mirror;
			*transp = o.transp;
			*ior = o.ior;
			*oid = i;
			hit = 1;
		}
	}
	return (hit);
}

static float3	gpu_shadow_atten(float3 p, float3 ldir, float ldist,
		__global const GpuObj *objs, int nobj, int self_id)
{
	float3	atten = (float3)(1.0f, 1.0f, 1.0f);
	float3	ro = p + ldir * 1e-3f;

	for (int i = 0; i < nobj; i++)
	{
		if (i == self_id)
			continue ;
		GpuObj o = objs[i];
		float t = 1e30f;
		float3 nn;
		int h = 0;
		if (o.type == 1)
			h = hit_sphere(ro, ldir, o, &t, &nn);
		else if (o.type == 2)
			h = hit_plane(ro, ldir, o, &t, &nn);
		else if (o.type == 3)
			h = hit_cylinder(ro, ldir, o, &t, &nn);
		else if (o.type == 4)
			h = hit_cone(ro, ldir, o, &t, &nn);
		else if (o.type == 5)
			h = hit_triangle(ro, ldir, o, &t, &nn);
		else if (o.type == 6)
			h = hit_half_sphere(ro, ldir, o, &t, &nn);
		if (!h || t >= ldist)
			continue ;
		if (o.transp <= 0.0f)
			return ((float3)(0.0f, 0.0f, 0.0f));
		atten *= glass_tint3(V3(o.color)) * o.transp;
	}
	return (atten);
}

static float3	shade_direct(float3 p, float3 n, float3 rd, float3 col,
		float shininess, float reflectivity, int oid,
		__global const GpuObj *objs, int nobj,
		__global const GpuLight *lights, int nlight, float3 ambient)
{
	float3	lit = ambient;

	for (int li = 0; li < nlight; li++)
	{
		float3 lpos = V3(lights[li].pos);
		float3 lcol = V3(lights[li].color);
		float3 ldir = lpos - p;
		float ldist = sqrt(dot3(ldir, ldir));
		ldir = ldir / fmax(ldist, 1e-6f);
		float3 atten = gpu_shadow_atten(p, ldir, ldist, objs, nobj, oid);
		if (atten.x <= 0.0f && atten.y <= 0.0f && atten.z <= 0.0f)
			continue ;
		float ndl = fmax(dot3(n, ldir), 0.0f);
		float diff = ndl * lights[li].intensity;
		if (diff > 0.0f)
			lit += col * lcol * atten * diff;
		if (shininess > 0.0f)
		{
			float3 r = ldir - n * (2.0f * dot3(ldir, n));
			float prod = dot3(norm3(r), rd);
			if (prod > 0.0f)
			{
				float spec = reflectivity * pow(prod, shininess)
					* lights[li].intensity;
				lit += lcol * atten * spec;
			}
		}
	}
	return (lit);
}

__kernel void	trace_frame(__global float *out,
		__global const GpuCam *cam,
		__global const GpuObj *objs,
		int nobj,
		__global const GpuLight *lights,
		int nlight,
		float3 ambient,
		int width,
		int height,
		int y_start,
		int y_count,
		int frame)
{
	int		id;
	int		x;
	int		ly;
	int		y;
	uint	seed;
	float	xoff;
	float	yoff;
	float	nx;
	float	ny;
	float3	ro;
	float3	rd;
	float	tmin;
	float3	n;
	float3	col;
	float	shininess;
	float	reflectivity;
	float	mirror;
	float	transp;
	float	ior;
	int		oid;
	float3	rgb;
	float3	wmask;

	id = (int)get_global_id(0);
	if (id >= width * y_count)
		return ;
	x = id % width;
	ly = id / width;
	y = y_start + ly;
	seed = (uint)(y * width + x) ^ ((uint)frame * 2654435761u);
	xoff = rand_u(&seed) - 0.5f;
	yoff = rand_u(&seed) - 0.5f;
	nx = (((float)x + xoff) * (2.0f / (float)width)) - 1.0f;
	ny = (((float)y + yoff) * (2.0f / (float)height)) - 1.0f;
	ro = V3(cam->origin);
	rd = norm3(V3(cam->center) + V3(cam->u) * nx + V3(cam->v) * ny - ro);
	rgb = (float3)(0.0f, 0.0f, 0.0f);
	wmask = (float3)(1.0f, 1.0f, 1.0f);
	for (int bounce = 0; bounce <= 5; bounce++)
	{
		if (wmask.x + wmask.y + wmask.z < 1e-3f)
			break ;
		if (!intersect(ro, rd, objs, nobj, &tmin, &n, &col,
				&shininess, &reflectivity, &mirror, &transp, &ior, &oid))
			break ;
		float3 p = ro + rd * tmin;
		float3 lit = shade_direct(p, n, rd, col, shininess, reflectivity, oid,
				objs, nobj, lights, nlight, ambient);
		if ((mirror <= 0.0f && transp <= 0.0f) || bounce == 5)
		{
			rgb += lit * wmask;
			break ;
		}
		float opaque = fmax(0.0f, 1.0f - mirror - transp);
		rgb += lit * (wmask * opaque);
		float3 nn = (dot3(rd, n) > 0.0f) ? -n : n;
		if (transp > 0.0f)
		{
			if (mirror > 0.0f)
			{
				float3 rdir = norm3(rd - nn * (2.0f * dot3(rd, nn)));
				float3 rorg = p + nn * 1e-3f;
				float rt;
				float3 rn, rc;
				float rsh, rrf, rmr, rtr, rior;
				int roid;
				if (intersect(rorg, rdir, objs, nobj, &rt, &rn, &rc,
						&rsh, &rrf, &rmr, &rtr, &rior, &roid))
				{
					float3 rlit = shade_direct(rorg + rdir * rt, rn, rdir, rc,
							rsh, rrf, roid, objs, nobj, lights, nlight, ambient);
					rgb += rlit * (wmask * mirror);
				}
			}
			float cosi = dot3(rd, n);
			float eta = 1.0f / fmax(ior, 1.0f);
			if (cosi < 0.0f)
				cosi = -cosi;
			else
				eta = fmax(ior, 1.0f);
			float kt = 1.0f - eta * eta * (1.0f - cosi * cosi);
			wmask *= glass_tint3(col) * transp;
			if (kt < 0.0f)
			{
				ro = p + nn * 1e-3f;
				rd = norm3(rd - nn * (2.0f * dot3(rd, nn)));
			}
			else
			{
				ro = p - nn * 1e-3f;
				rd = norm3(rd * eta + nn * (eta * cosi - sqrt(kt)));
			}
		}
		else
		{
			wmask *= mirror;
			ro = p + nn * 1e-3f;
			rd = norm3(rd - nn * (2.0f * dot3(rd, nn)));
		}
	}
	out[id * 3 + 0] = rgb.x;
	out[id * 3 + 1] = rgb.y;
	out[id * 3 + 2] = rgb.z;
}
