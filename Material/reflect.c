/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reflect.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:35:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:35:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static t_ray	build_reflect_ray(t_info *info, t_ray *in_ray)
{
	t_vector	dir;
	t_vector	refl;
	t_vector	orig;

	dir = normalized(in_ray->direction);
	refl = reflect(dir, info->normal);
	orig = add_vec(info->hitpoint, scale_vec(info->normal, 0.001f));
	return (new_ray(orig, add_vec(orig, refl)));
}

static t_vector	blend_colors(t_vector local, t_vector refl, float m)
{
	return (add_vec(scale_vec(local, 1.0f - m), scale_vec(refl, m)));
}

t_vector	trace_ray(t_vars *vars, t_ray *ray, int depth)
{
	t_info		info;
	t_vector	local;
	t_vector	refl;
	t_ray		r_ray;
	float		m;

	if (!test_intersection(ray, vars->objects, &info))
		return (new_vec(0.0f, 0.0f, 0.0f));
	local = compute_color(vars, &info, ray);
	m = info.e->mirror;
	if (m <= 0.0f || depth <= 0)
		return (local);
	r_ray = build_reflect_ray(&info, ray);
	refl = trace_ray(vars, &r_ray, depth - 1);
	return (blend_colors(local, refl, m));
}
