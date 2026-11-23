/* OpenCL 1.2 multi-GPU ray tracer (sp/pl/cy/cn + shadows + Phong + jitter). */

#define V3(a) ((float3)((a)[0], (a)[1], (a)[2]))

typedef struct {
	int		type;
	float	pos[3];
	float	axis[3];
	float	color[3];
	float	radius;
	float	height;
	float	shininess;
	float	reflectivity;
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

static int	intersect(float3 ro, float3 rd, __global const GpuObj *objs,
		int nobj, float *tmin, float3 *n, float3 *col,
		float *shininess, float *reflectivity, int *oid)
{
	int		i;
	float	t;
	float3	nn;
	int		hit;

	*tmin = 1e30f;
	*oid = -1;
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
		else
			continue ;
		if (t < *tmin)
		{
			*tmin = t;
			*n = nn;
			*col = V3(o.color);
			*shininess = o.shininess;
			*reflectivity = o.reflectivity;
			*oid = i;
			hit = 1;
		}
	}
	return (hit);
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
	int		oid;
	float3	rgb;

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
	if (intersect(ro, rd, objs, nobj, &tmin, &n, &col,
			&shininess, &reflectivity, &oid))
	{
		float3 p = ro + rd * tmin;
		rgb = ambient;
		for (int li = 0; li < nlight; li++)
		{
			float3 lpos = V3(lights[li].pos);
			float3 lcol = V3(lights[li].color);
			float3 ldir = lpos - p;
			float ldist = sqrt(dot3(ldir, ldir));
			ldir = ldir / fmax(ldist, 1e-6f);
			float t2;
			float3 n2, c2;
			float sh2, rf2;
			int oid2;
			int blocked = intersect(p + ldir * 1e-3f, ldir, objs, nobj,
					&t2, &n2, &c2, &sh2, &rf2, &oid2) && (t2 < ldist);
			if (!blocked)
			{
				float ndl = fmax(dot3(n, ldir), 0.0f);
				float diff = (1.0f - (acos(clamp(ndl, -1.0f, 1.0f))
						/ 1.5707963f)) * lights[li].intensity;
				if (diff > 0.0f)
					rgb += col * lcol * diff;
				if (shininess > 0.0f)
				{
					float3 r = ldir - n * (2.0f * dot3(ldir, n));
					float prod = dot3(norm3(r), rd);
					if (prod > 0.0f)
					{
						float spec = reflectivity * pow(prod, shininess)
							* lights[li].intensity;
						rgb += lcol * spec;
					}
				}
			}
		}
	}
	out[id * 3 + 0] = rgb.x;
	out[id * 3 + 1] = rgb.y;
	out[id * 3 + 2] = rgb.z;
}
