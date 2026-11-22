/* OpenCL 1.2 primary-ray tracer (sp/pl/cy/cn + ambient/diffuse/shadows). */

typedef struct {
	int		type;
	float	pos[3];
	float	axis[3];
	float	color[3];
	float	radius;
	float	height;
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

float3	vload3f(const float *a)
{
	return ((float3)(a[0], a[1], a[2]));
}

float	dot3(float3 a, float3 b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

float3	norm3(float3 a)
{
	float	l;

	l = sqrt(dot3(a, a));
	if (l < 1e-8f)
		return (a);
	return (a / l);
}

int	hit_sphere(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	oc;
	float	b;
	float	c;
	float	disc;
	float	t0;

	oc = ro - vload3f(o.pos);
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
	*n = norm3((ro + rd * t0) - vload3f(o.pos));
	return (1);
}

int	hit_plane(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	axis;
	float	denom;
	float	d;
	float	tt;

	axis = norm3(vload3f(o.axis));
	denom = dot3(axis, rd);
	if (fabs(denom) < 1e-6f)
		return (0);
	d = -dot3(axis, vload3f(o.pos));
	tt = (-d - dot3(ro, axis)) / denom;
	if (tt < 1e-3f)
		return (0);
	*t = tt;
	*n = axis;
	if (dot3(*n, rd) > 0.0f)
		*n = -*n;
	return (1);
}

int	hit_cylinder(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	ca;
	float3	oc;
	float	card;
	float	caoc;
	float	a;
	float	b;
	float	c;
	float	disc;
	float	t0;
	float3	p;
	float	h;
	float	halfh;

	ca = norm3(vload3f(o.axis));
	oc = ro - vload3f(o.pos);
	card = dot3(ca, rd);
	caoc = dot3(ca, oc);
	a = dot3(rd, rd) - card * card;
	b = 2.0f * (dot3(rd, oc) - card * caoc);
	c = dot3(oc, oc) - caoc * caoc - o.radius * o.radius;
	disc = b * b - 4.0f * a * c;
	if (disc < 0.0f || fabs(a) < 1e-8f)
		return (0);
	t0 = (-b - sqrt(disc)) / (2.0f * a);
	if (t0 < 1e-3f)
		t0 = (-b + sqrt(disc)) / (2.0f * a);
	if (t0 < 1e-3f)
		return (0);
	p = ro + rd * t0;
	h = dot3(p - vload3f(o.pos), ca);
	halfh = o.height * 0.5f;
	if (h < -halfh || h > halfh)
		return (0);
	*t = t0;
	*n = norm3(p - vload3f(o.pos) - ca * h);
	return (1);
}

int	hit_cone(float3 ro, float3 rd, GpuObj o, float *t, float3 *n)
{
	float3	ca;
	float3	oc;
	float	k;
	float	card;
	float	caoc;
	float	a;
	float	b;
	float	c;
	float	disc;
	float	t0;
	float3	p;
	float	h;
	float	halfh;

	ca = norm3(vload3f(o.axis));
	oc = ro - vload3f(o.pos);
	k = (o.radius / fmax(o.height, 1e-4f));
	k = k * k;
	card = dot3(ca, rd);
	caoc = dot3(ca, oc);
	a = dot3(rd, rd) - (1.0f + k) * card * card;
	b = 2.0f * (dot3(rd, oc) - (1.0f + k) * card * caoc);
	c = dot3(oc, oc) - (1.0f + k) * caoc * caoc;
	disc = b * b - 4.0f * a * c;
	if (disc < 0.0f || fabs(a) < 1e-8f)
		return (0);
	t0 = (-b - sqrt(disc)) / (2.0f * a);
	if (t0 < 1e-3f)
		t0 = (-b + sqrt(disc)) / (2.0f * a);
	if (t0 < 1e-3f)
		return (0);
	p = ro + rd * t0;
	h = dot3(p - vload3f(o.pos), ca);
	halfh = o.height * 0.5f;
	if (h < -halfh || h > halfh)
		return (0);
	*t = t0;
	*n = norm3(p - vload3f(o.pos) - ca * h * (1.0f + k));
	return (1);
}

int	intersect(float3 ro, float3 rd, __global const GpuObj *objs, int nobj,
		float *tmin, float3 *n, float3 *col, int *oid)
{
	int		i;
	float	t;
	float3	nn;
	int		hit;

	*tmin = 1e30f;
	*oid = -1;
	hit = 0;
	i = 0;
	while (i < nobj)
	{
		t = 1e30f;
		if (objs[i].type == 1 && hit_sphere(ro, rd, objs[i], &t, &nn))
			;
		else if (objs[i].type == 2 && hit_plane(ro, rd, objs[i], &t, &nn))
			;
		else if (objs[i].type == 3 && hit_cylinder(ro, rd, objs[i], &t, &nn))
			;
		else if (objs[i].type == 4 && hit_cone(ro, rd, objs[i], &t, &nn))
			;
		else
		{
			i++;
			continue ;
		}
		if (t < *tmin)
		{
			*tmin = t;
			*n = nn;
			*col = vload3f(objs[i].color);
			*oid = i;
			hit = 1;
		}
		i++;
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
		int frame)
{
	int		id;
	int		x;
	int		y;
	float	nx;
	float	ny;
	float3	ro;
	float3	rd;
	float	tmin;
	float3	n;
	float3	col;
	float3	p;
	float3	rgb;
	int		oid;
	int		li;
	float3	ldir;
	float	ldist;
	float	t2;
	float3	n2;
	float3	c2;
	int		oid2;
	float	diff;

	id = get_global_id(0);
	if (id >= width * height)
		return ;
	x = id % width;
	y = id / width;
	nx = ((float)x / (float)(width / 2)) - 1.0f;
	ny = ((float)y / (float)(height / 2)) - 1.0f;
	ro = vload3f(cam->origin);
	rd = norm3(vload3f(cam->center) + vload3f(cam->u) * nx
			+ vload3f(cam->v) * ny - ro);
	rgb = ambient;
	if (intersect(ro, rd, objs, nobj, &tmin, &n, &col, &oid))
	{
		p = ro + rd * tmin;
		li = 0;
		while (li < nlight)
		{
			ldir = vload3f(lights[li].pos) - p;
			ldist = sqrt(dot3(ldir, ldir));
			ldir = ldir / ldist;
			if (!intersect(p + ldir * 1e-3f, ldir, objs, nobj, &t2, &n2, &c2,
					&oid2) || t2 > ldist)
			{
				diff = fmax(dot3(n, ldir), 0.0f);
				rgb += col * vload3f(lights[li].color) * lights[li].intensity
					* diff;
			}
			li++;
		}
	}
	out[id * 3 + 0] = rgb.x;
	out[id * 3 + 1] = rgb.y;
	out[id * 3 + 2] = rgb.z;
	(void)frame;
}
