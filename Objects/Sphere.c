/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sphere.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/14 13:15:13 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:46:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static float	pick_pos_t(float t1, float t2)
{
	if (t1 > EPSILON && t2 > EPSILON)
	{
		if (t1 < t2)
			return (t1);
		return (t2);
	}
	if (t1 > EPSILON)
		return (t1);
	if (t2 > EPSILON)
		return (t2);
	return (-1.0f);
}

int	test_sphere(t_ray *ray, t_info *info)
{
	t_norm	norm;
	float	t;

	norm.back_ray = apply_transform(ray, info->e->gtfm, BACKWARD);
	norm.ray_dir = normalized(norm.back_ray.direction);
	norm.b = 2 * dot_product(norm.ray_dir, norm.back_ray.point1);
	norm.c = dot_product(norm.back_ray.point1, norm.back_ray.point1) - 1.0f;
	norm.delta = square(norm.b) - (4 * norm.c);
	if (norm.delta < 0)
		return (0);
	norm.t1 = (-norm.b - sqrtf(norm.delta)) / 2.0f;
	norm.t2 = (-norm.b + sqrtf(norm.delta)) / 2.0f;
	t = pick_pos_t(norm.t1, norm.t2);
	if (t < 0.0f)
		return (0);
	norm.localpoi = add_vec(norm.back_ray.point1, scale_vec(norm.ray_dir, t));
	info->hitpoint = apply_transform_vector(&norm.localpoi,
			info->e->gtfm, FORWARD);
	info->normal = normalized(sub_vec(info->hitpoint, info->e->translation));
	return (1);
}
