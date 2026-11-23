/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   reflect.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:35:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:47:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static t_ray	build_reflect_ray(t_info *info, t_ray *in_ray)
{
	t_vector	dir;
	t_vector	refl;
	t_vector	orig;
	t_vector	nn;

	dir = normalized(in_ray->direction);
	nn = info->normal;
	if (dot_product(dir, nn) > 0.0f)
		nn = scale_vec(nn, -1.0f);
	refl = reflect(dir, nn);
	orig = add_vec(info->hitpoint, scale_vec(nn, 0.001f));
	return (new_ray(orig, add_vec(orig, refl)));
}

static t_vector	tint_mul(t_vector c, t_vector tint)
{
	return (new_vec(c.x * tint.x, c.y * tint.y, c.z * tint.z));
}

static t_vector	add_secondary(t_vars *vars, t_info *info,
		t_ray *ray, int depth)
{
	t_vector	out;
	t_ray		sec;
	t_vector	c;

	out = new_vec(0.0f, 0.0f, 0.0f);
	if (info->e->mirror > 0.0f)
	{
		sec = build_reflect_ray(info, ray);
		c = trace_ray(vars, &sec, depth - 1);
		out = add_vec(out, scale_vec(c, info->e->mirror));
	}
	if (info->e->transp > 0.0f)
	{
		sec = build_refract_ray(info, ray);
		c = tint_mul(trace_ray(vars, &sec, depth - 1),
				glass_tint(info->e->base_color));
		out = add_vec(out, scale_vec(c, info->e->transp));
	}
	return (out);
}

t_vector	trace_ray(t_vars *vars, t_ray *ray, int depth)
{
	t_info		info;
	t_vector	local;
	float		opaque;

	if (!test_intersection(ray, vars->objects, &info))
		return (new_vec(0.0f, 0.0f, 0.0f));
	local = compute_color(vars, &info, ray);
	if (depth <= 0 || (info.e->mirror <= 0.0f && info.e->transp <= 0.0f))
		return (local);
	opaque = 1.0f - info.e->mirror - info.e->transp;
	if (opaque < 0.0f)
		opaque = 0.0f;
	return (add_vec(scale_vec(local, opaque),
			add_secondary(vars, &info, ray, depth)));
}
