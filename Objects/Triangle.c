/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Triangle.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:32:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:32:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static void	set_tri_hit(t_ray *ray, t_info *info, float t, t_vector n)
{
	info->hitpoint = add_vec(ray->point1, scale_vec(ray->direction, t));
	info->normal = normalized(n);
	if (dot_product(info->normal, ray->direction) > 0.0f)
		info->normal = scale_vec(info->normal, -1.0f);
}

int	test_triangle(t_ray *ray, t_info *info)
{
	t_vector	e[2];
	t_vector	h;
	t_vector	s;
	t_vector	q;
	float		f[4];

	e[0] = sub_vec(info->e->d_normal, info->e->translation);
	e[1] = sub_vec(info->e->scale, info->e->translation);
	h = cross_product(ray->direction, e[1]);
	f[0] = dot_product(e[0], h);
	if (fabsf(f[0]) < EPSILON)
		return (0);
	f[1] = 1.0f / f[0];
	s = sub_vec(ray->point1, info->e->translation);
	f[2] = f[1] * dot_product(s, h);
	if (f[2] < 0.0f || f[2] > 1.0f)
		return (0);
	q = cross_product(s, e[0]);
	f[3] = f[1] * dot_product(ray->direction, q);
	if (f[3] < 0.0f || f[2] + f[3] > 1.0f)
		return (0);
	f[0] = f[1] * dot_product(e[1], q);
	if (f[0] <= EPSILON)
		return (0);
	return (set_tri_hit(ray, info, f[0], cross_product(e[0], e[1])), 1);
}
