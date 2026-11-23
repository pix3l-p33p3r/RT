/* Standalone math unit test for stereoscopic camera rays, anaglyph mixing,
 * side-by-side VR remapping, and soft area-light spherical jitter bounds.
 * Compile: cc -Wall -Wextra -Werror tests/test_stereo.c -lm -o test_stereo
 */
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH 800
#define EYE_OFFSET 0.18f
#define FOCAL_DIST 8.0f

typedef struct s_vec3
{
	float	x;
	float	y;
	float	z;
}	t_vec3;

typedef struct s_ray
{
	t_vec3	p1;
	t_vec3	p2;
	t_vec3	dir;
}	t_ray;

typedef struct s_cam
{
	t_vec3	origin;
	t_vec3	cam_u;
	t_vec3	cam_v;
	t_vec3	cam_w;
	t_vec3	screen_center;
	t_vec3	screen_u;
	t_vec3	screen_v;
}	t_cam;

typedef struct s_light
{
	t_vec3	position;
	int		is_dir;
	float	radius;
}	t_light;

static t_vec3	v3(float x, float y, float z)
{
	t_vec3	v = {x, y, z};
	return (v);
}

static t_vec3	v_add(t_vec3 a, t_vec3 b)
{
	return (v3(a.x + b.x, a.y + b.y, a.z + b.z));
}

static t_vec3	v_sub(t_vec3 a, t_vec3 b)
{
	return (v3(a.x - b.x, a.y - b.y, a.z - b.z));
}

static t_vec3	v_scale(t_vec3 a, float s)
{
	return (v3(a.x * s, a.y * s, a.z * s));
}

static float	v_len2(t_vec3 a)
{
	return (a.x * a.x + a.y * a.y + a.z * a.z);
}

static float	v_len(t_vec3 a)
{
	return (sqrtf(v_len2(a)));
}

static t_vec3	v_norm(t_vec3 a)
{
	float	l = v_len(a);
	if (l < 1e-8f)
		return (a);
	return (v_scale(a, 1.0f / l));
}

static unsigned int	rand_pcg(unsigned int *rng_state)
{
	unsigned int	state = *rng_state;
	unsigned int	word;

	*rng_state = state * 747796405u + 2891336453u;
	word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return ((word >> 22u) ^ word);
}

static float	random_float(unsigned int *rng, float min, float max)
{
	return (min + (float)(rand_pcg(rng) * (1.0 / UINT_MAX)) * (max - min));
}

static t_ray	generate_ray(float nx, float ny, const t_cam *cam)
{
	t_vec3	p2 = v_add(v_add(cam->screen_center, v_scale(cam->screen_u, nx)),
			v_scale(cam->screen_v, ny));
	t_ray	r = {cam->origin, p2, v_sub(p2, cam->origin)};
	return (r);
}

static t_ray	stereo_eye_ray(const t_cam *cam, float nx, float ny,
		float eye_sign)
{
	t_ray	base = generate_ray(nx, ny, cam);
	t_vec3	eye = v_add(cam->origin, v_scale(cam->cam_u, eye_sign * EYE_OFFSET));
	t_vec3	focal = v_add(cam->origin, v_scale(base.dir, FOCAL_DIST));
	t_ray	r = {eye, focal, v_sub(focal, eye)};
	return (r);
}

static t_vec3	anaglyph_mix(t_vec3 cl, t_vec3 cr)
{
	return (v3(cl.x, cr.y, cr.z));
}

static float	sbs_remap_nx(int x, float normx)
{
	if (x < WIDTH / 2)
		return (normx * 2.0f + 1.0f);
	return (normx * 2.0f - 1.0f);
}

static t_vec3	jitter_light_pos(const t_light *light, unsigned int *rng)
{
	t_vec3	off = v3(0.0f, 0.0f, 0.0f);
	int		tries = 0;

	if (light->is_dir || light->radius <= 0.0f)
		return (light->position);
	while (tries < 8)
	{
		off = v3(random_float(rng, -1.0f, 1.0f),
				random_float(rng, -1.0f, 1.0f),
				random_float(rng, -1.0f, 1.0f));
		if (v_len2(off) <= 1.0f)
			return (v_add(light->position, v_scale(off, light->radius)));
		tries++;
	}
	return (v_add(light->position, v_scale(v_norm(off), light->radius)));
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
	t_cam			cam;
	t_ray			rl;
	t_ray			rr;
	t_vec3			mid;
	t_vec3			ana;
	t_light			area = {{5.5f, -5.5f, -8.5f}, 0, 0.9f};
	t_light			pt = {{5.5f, -5.5f, -8.5f}, 0, 0.0f};
	t_light			dir = {{0.0f, 0.0f, -1.0f}, 1, 0.9f};
	unsigned int	rng = 1337u;
	t_vec3			sum = {0.0f, 0.0f, 0.0f};
	float			max_d = 0.0f;
	float			min_d = 1e9f;
	int				i;

	cam.origin = v3(0.0f, -11.0f, -3.0f);
	cam.cam_u = v3(1.0f, 0.0f, 0.0f);
	cam.cam_v = v3(0.0f, 0.0f, 1.0f);
	cam.cam_w = v3(0.0f, 1.0f, 0.0f);
	cam.screen_center = v_add(cam.origin, cam.cam_w);
	cam.screen_u = v_scale(cam.cam_u, 0.89f);
	cam.screen_v = v_scale(cam.cam_v, 0.89f * 0.75f);

	rl = stereo_eye_ray(&cam, 0.0f, 0.0f, -1.0f);
	rr = stereo_eye_ray(&cam, 0.0f, 0.0f, 1.0f);
	mid = v_scale(v_add(rl.p1, rr.p1), 0.5f);
	expect(v_len(v_sub(mid, cam.origin)) < 1e-5f,
		"left/right eye origins symmetric around camera origin");
	expect(fabsf((rr.p1.x - rl.p1.x) - (2.0f * EYE_OFFSET)) < 1e-5f,
		"inter-pupillary separation equals 2 * 0.18 along cam_u");
	expect(v_len(v_sub(rl.p2, rr.p2)) < 1e-5f
		&& fabsf(rl.p2.y - (cam.origin.y + FOCAL_DIST)) < 1e-5f,
		"left/right rays converge at focal distance 8.0");
	expect(fabsf(rl.dir.x + rr.dir.x) < 1e-5f && rl.dir.x > 0.0f
		&& rr.dir.x < 0.0f,
		"center rays toe-in symmetrically (left +X, right -X)");

	expect(fabsf(sbs_remap_nx(0, -1.0f) + 1.0f) < 1e-5f
		&& fabsf(sbs_remap_nx(200, -0.5f)) < 1e-5f
		&& fabsf(sbs_remap_nx(400, 0.0f) + 1.0f) < 1e-5f
		&& fabsf(sbs_remap_nx(600, 0.5f)) < 1e-5f,
		"VR side-by-side nx remap maps both halves onto [-1, 1]");

	ana = anaglyph_mix(v3(0.9f, 0.2f, 0.1f), v3(0.15f, 0.75f, 0.85f));
	expect(fabsf(ana.x - 0.9f) < 1e-6f && fabsf(ana.y - 0.75f) < 1e-6f
		&& fabsf(ana.z - 0.85f) < 1e-6f,
		"anaglyph mixes left R with right G and B channels");

	expect(v_len(v_sub(jitter_light_pos(&pt, &rng), pt.position)) == 0.0f,
		"zero-radius point light returns exact position");
	expect(v_len(v_sub(jitter_light_pos(&dir, &rng), dir.position)) == 0.0f,
		"directional light ignores radius and returns exact direction");

	i = 0;
	while (i < 10000)
	{
		t_vec3	j = jitter_light_pos(&area, &rng);
		float	d = v_len(v_sub(j, area.position));
		if (d > max_d)
			max_d = d;
		if (d < min_d)
			min_d = d;
		sum = v_add(sum, v_sub(j, area.position));
		i++;
	}
	expect(max_d <= area.radius + 1e-5f,
		"all 10000 soft-light samples lie within sphere of radius 0.9");
	expect(max_d > area.radius * 0.85f && min_d < area.radius * 0.25f,
		"soft-light samples fill interior and outer volume of sphere");
	expect(v_len(v_scale(sum, 1.0f / 10000.0f)) < 0.05f,
		"soft-light sample mean is centered at light position");

	printf("all stereo & soft-light tests passed\n");
	return (0);
}
