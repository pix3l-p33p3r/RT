/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HalfSphere.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:32:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:32:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static int	check_hemi_hit(t_norm *norm, t_info *info, float t)
{
	t_vector	world;
	t_vector	axis;

	if (t <= EPSILON)
		return (0);
	norm->localpoi = add_vec(norm->back_ray.point1,
			scale_vec(norm->ray_dir, t));
	world = apply_transform_vector(&norm->localpoi, info->e->gtfm, FORWARD);
	axis = normalized(info->e->d_normal);
	if (dot_product(sub_vec(world, info->e->translation), axis) < -EPSILON)
		return (0);
	info->hitpoint = world;
	info->normal = normalized(sub_vec(world, info->e->translation));
	return (1);
}

int	test_half_sphere(t_ray *ray, t_info *info)
{
	t_norm	norm;
	float	first;
	float	second;

	norm.back_ray = apply_transform(ray, info->e->gtfm, BACKWARD);
	norm.ray_dir = normalized(norm.back_ray.direction);
	norm.b = 2 * dot_product(norm.ray_dir, norm.back_ray.point1);
	norm.c = dot_product(norm.back_ray.point1, norm.back_ray.point1) - 1.0f;
	norm.delta = square(norm.b) - (4 * norm.c);
	if (norm.delta < 0.0f)
		return (0);
	first = (-norm.b - sqrtf(norm.delta)) / 2.0f;
	second = (-norm.b + sqrtf(norm.delta)) / 2.0f;
	if (check_hemi_hit(&norm, info, first))
		return (1);
	if (check_hemi_hit(&norm, info, second))
		return (1);
	return (0);
}
