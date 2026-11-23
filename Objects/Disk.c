/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Disk.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:55:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:55:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

int	test_disk(t_ray *ray, t_info *info)
{
	t_vector	axis;
	t_vector	hit;
	float		denom;
	float		t;

	axis = normalized(info->e->d_normal);
	denom = dot_product(axis, ray->direction);
	if (fabsf(denom) < EPSILON)
		return (0);
	t = dot_product(sub_vec(info->e->translation, ray->point1), axis) / denom;
	if (t < EPSILON)
		return (0);
	hit = add_vec(ray->point1, scale_vec(ray->direction, t));
	if (length2(sub_vec(hit, info->e->translation)) > square(info->e->radius))
		return (0);
	info->hitpoint = hit;
	info->normal = axis;
	if (dot_product(info->normal, ray->direction) > 0.0f)
		info->normal = scale_vec(info->normal, -1.0f);
	return (1);
}
