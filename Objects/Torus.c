/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Torus.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 03:58:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 03:58:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static float	torus_sdf(t_vector p, float maj, float min)
{
	float	q;

	q = sqrtf(square(p.x) + square(p.y)) - maj;
	return (sqrtf(square(q) + square(p.z)) - min);
}

static int	torus_bounds(t_vector ro, t_vector rd, float rb, float *t)
{
	float	b;
	float	c;
	float	disc;
	float	s;

	b = dot_product(ro, rd);
	c = dot_product(ro, ro) - square(rb);
	disc = b * b - c;
	if (disc < 0.0f)
		return (0);
	s = sqrtf(disc);
	t[0] = -b - s;
	t[1] = -b + s;
	if (t[1] < EPSILON)
		return (0);
	if (t[0] < EPSILON)
		t[0] = EPSILON;
	return (1);
}

static int	torus_march(t_ray *br, t_vector rd, t_object *e, t_vector *hit)
{
	float		t[2];
	float		d;
	int			step;
	t_vector	p;

	if (!torus_bounds(br->point1, rd, e->radius + e->height + 0.01f, t))
		return (0);
	step = 0;
	while (step < 96 && t[0] <= t[1])
	{
		p = add_vec(br->point1, scale_vec(rd, t[0]));
		d = torus_sdf(p, e->radius, e->height);
		if (fabsf(d) < 0.0005f)
		{
			*hit = p;
			return (1);
		}
		t[0] += fabsf(d);
		step++;
	}
	return (0);
}

static t_vector	torus_normal(t_vector p, t_object *e, t_vector rdir)
{
	float		q;
	float		factor;
	t_vector	n_loc;
	t_vector	n_w;

	q = sqrtf(square(p.x) + square(p.y));
	if (q < 0.000001f)
		q = 0.000001f;
	factor = 1.0f - e->radius / q;
	n_loc = new_vec(p.x * factor, p.y * factor, p.z);
	n_w = apply_transform_vector(&n_loc, e->gtfm, FORWARD);
	n_w = normalized(sub_vec(n_w, e->translation));
	if (dot_product(n_w, rdir) > 0.0f)
		n_w = scale_vec(n_w, -1.0f);
	return (n_w);
}

int	test_torus(t_ray *ray, t_info *info)
{
	t_ray		br;
	t_vector	rd;
	t_vector	poi;

	br = apply_transform(ray, info->e->gtfm, BACKWARD);
	rd = normalized(br.direction);
	if (!torus_march(&br, rd, info->e, &poi))
		return (0);
	info->hitpoint = apply_transform_vector(&poi, info->e->gtfm, FORWARD);
	info->normal = torus_normal(poi, info->e, ray->direction);
	return (1);
}
