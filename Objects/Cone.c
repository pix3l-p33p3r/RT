/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cone.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static int	cone_side_t(t_ray *back_ray, t_vector *vhat,
		float *t, t_vector *poi)
{
	float		q[4];
	t_vector	p;

	p = back_ray->point1;
	q[0] = square(vhat->x) + square(vhat->y) - square(vhat->z);
	q[1] = 2.0f * (p.x * vhat->x + p.y * vhat->y - p.z * vhat->z);
	q[2] = square(p.x) + square(p.y) - square(p.z);
	if (fabsf(q[0]) < EPSILON)
		return (0);
	q[3] = square(q[1]) - (4.0f * q[0] * q[2]);
	if (q[3] < 0.0f)
		return (0);
	q[3] = sqrtf(q[3]);
	t[0] = (-q[1] + q[3]) / (2.0f * q[0]);
	t[1] = (-q[1] - q[3]) / (2.0f * q[0]);
	poi[0] = add_vec(back_ray->point1, scale_vec(*vhat, t[0]));
	poi[1] = add_vec(back_ray->point1, scale_vec(*vhat, t[1]));
	return (1);
}

static void	cone_caps(t_ray *back_ray, t_vector *vhat, float *t, t_vector *poi)
{
	if (fabsf(vhat->z) < EPSILON)
	{
		t[2] = 100e6;
		t[3] = 100e6;
		return ;
	}
	t[2] = (back_ray->point1.z - 1.0f) / -vhat->z;
	t[3] = (back_ray->point1.z + 1.0f) / -vhat->z;
	poi[2] = add_vec(back_ray->point1, scale_vec(*vhat, t[2]));
	poi[3] = add_vec(back_ray->point1, scale_vec(*vhat, t[3]));
	if (!(t[2] > 0.0f && sqrtf(square(poi[2].x) + square(poi[2].y)) < 1.0f))
		t[2] = 100e6;
	if (!(t[3] > 0.0f && sqrtf(square(poi[3].x) + square(poi[3].y)) < 1.0f))
		t[3] = 100e6;
}

static void	cone_get_distance(t_ray *back_ray, t_vector *vhat,
		float *t, t_vector *poi)
{
	if (!cone_side_t(back_ray, vhat, t, poi))
	{
		t[0] = 100e6;
		t[1] = 100e6;
	}
	else
	{
		if (!(t[0] > 0.0f && fabsf(poi[0].z) < 1.0f))
			t[0] = 100e6;
		if (!(t[1] > 0.0f && fabsf(poi[1].z) < 1.0f))
			t[1] = 100e6;
	}
	cone_caps(back_ray, vhat, t, poi);
}

static float	cone_get_min(float *t, int *min_index)
{
	float	min;
	int		i;

	if (t[0] == 100e6 && t[1] == 100e6
		&& t[2] == 100e6 && t[3] == 100e6)
		return (-1);
	min = 10e6;
	i = 0;
	*min_index = 0;
	while (i < 4)
	{
		if (t[i] < min)
		{
			min = t[i];
			*min_index = i;
		}
		i++;
	}
	return (min);
}

int	test_cone(t_ray *ray, t_info *info)
{
	t_ray		back_ray;
	t_vector	vhat;
	float		t[4];
	t_vector	poi[4];
	int			min_index;

	back_ray = apply_transform(ray, info->e->gtfm, BACKWARD);
	vhat = back_ray.direction;
	cone_get_distance(&back_ray, &vhat, t, poi);
	min_index = 0;
	if (cone_get_min(t, &min_index) == -1)
		return (0);
	return (cone_calculate_props(min_index, &poi[min_index], info, &vhat));
}
