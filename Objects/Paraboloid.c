/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Paraboloid.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:55:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:55:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static int	parab_solve(t_ray *br, t_vector v, float *t)
{
	t_vector	p;
	float		q[4];

	p = br->point1;
	q[0] = square(v.x) + square(v.y);
	q[1] = 2.0f * (p.x * v.x + p.y * v.y) - v.z;
	q[2] = square(p.x) + square(p.y) - p.z;
	if (fabsf(q[0]) < EPSILON)
	{
		if (fabsf(q[1]) < EPSILON)
			return (0);
		t[0] = -q[2] / q[1];
		t[1] = -1.0f;
		return (1);
	}
	q[3] = square(q[1]) - (4.0f * q[0] * q[2]);
	if (q[3] < 0.0f)
		return (0);
	q[3] = sqrtf(q[3]);
	t[0] = (-q[1] - q[3]) / (2.0f * q[0]);
	t[1] = (-q[1] + q[3]) / (2.0f * q[0]);
	return (1);
}

static int	parab_pick(t_ray *br, t_vector v, float *t, t_vector *poi)
{
	int	i;

	i = 0;
	while (i < 2)
	{
		if (t[i] > EPSILON)
		{
			*poi = add_vec(br->point1, scale_vec(v, t[i]));
			if (poi->z >= 0.0f && poi->z <= 1.0f)
				return (1);
		}
		i++;
	}
	return (0);
}

static t_vector	parab_world_normal(t_vector poi, t_matrix **gtfm)
{
	t_matrix	*m;
	t_vector	ln;
	t_vector	wn;

	ln = new_vec(2.0f * poi.x, 2.0f * poi.y, -1.0f);
	if (!gtfm || !gtfm[1] || !gtfm[1]->matrix)
		return (normalized(ln));
	m = gtfm[1];
	wn.x = m->matrix[0][0] * ln.x + m->matrix[1][0] * ln.y
		+ m->matrix[2][0] * ln.z;
	wn.y = m->matrix[0][1] * ln.x + m->matrix[1][1] * ln.y
		+ m->matrix[2][1] * ln.z;
	wn.z = m->matrix[0][2] * ln.x + m->matrix[1][2] * ln.y
		+ m->matrix[2][2] * ln.z;
	return (normalized(wn));
}

int	test_paraboloid(t_ray *ray, t_info *info)
{
	t_ray		br;
	t_vector	v;
	t_vector	poi;
	float		t[2];

	br = apply_transform(ray, info->e->gtfm, BACKWARD);
	v = normalized(br.direction);
	if (!parab_solve(&br, v, t))
		return (0);
	if (!parab_pick(&br, v, t, &poi))
		return (0);
	info->hitpoint = apply_transform_vector(&poi, info->e->gtfm, FORWARD);
	info->normal = parab_world_normal(poi, info->e->gtfm);
	if (dot_product(info->normal, ray->direction) > 0.0f)
		info->normal = scale_vec(info->normal, -1.0f);
	return (1);
}
