/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Csg.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 03:58:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 03:58:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

int	point_in_neg(t_vector p, t_object *objects)
{
	int	i;

	i = 0;
	while (objects && objects[i].gtfm)
	{
		if (objects[i].is_neg
			&& length(sub_vec(p, objects[i].translation)) < objects[i].radius)
			return (i);
		i++;
	}
	return (-1);
}

static int	point_in_solid(t_vector p, t_object *o)
{
	t_vector	lp;
	float		q;

	if (!o || o->is_neg)
		return (0);
	if (o->type == SPHERE)
		return (length2(sub_vec(p, o->translation)) < square(o->radius));
	if (o->type == CYLINDER)
	{
		lp = apply_transform_vector(&p, o->gtfm, BACKWARD);
		return (fabsf(lp.z) < 1.0f && square(lp.x) + square(lp.y) < 1.0f);
	}
	if (o->type == TORUS)
	{
		lp = apply_transform_vector(&p, o->gtfm, BACKWARD);
		q = sqrtf(square(lp.x) + square(lp.y)) - o->radius;
		return (square(q) + square(lp.z) < square(o->height));
	}
	return (0);
}

static t_object	*find_solid_at(t_vector p, t_object *objects, t_object *pref)
{
	int	i;

	if (point_in_solid(p, pref))
		return (pref);
	i = 0;
	while (objects && objects[i].gtfm)
	{
		if (point_in_solid(p, &objects[i]))
			return (&objects[i]);
		i++;
	}
	return (NULL);
}

static int	csg_exit_neg(t_ray *ray, t_object *objs,
		t_vector *exit_pt, int *idx)
{
	t_vector	rd;
	t_vector	ro;
	t_vector	oc;
	float		q[2];
	int			guard;

	rd = normalized(ray->direction);
	ro = ray->point1;
	guard = 0;
	while (*idx >= 0 && guard++ < 8)
	{
		oc = sub_vec(ro, objs[*idx].translation);
		q[0] = dot_product(oc, rd);
		q[1] = q[0] * q[0] - dot_product(oc, oc) + square(objs[*idx].radius);
		if (q[1] < 0.0f || sqrtf(q[1]) - q[0] <= EPSILON)
			return (0);
		*exit_pt = add_vec(ro, scale_vec(rd, sqrtf(q[1]) - q[0]));
		ro = add_vec(*exit_pt, scale_vec(rd, 0.001f));
		if (point_in_neg(ro, objs) < 0)
			return (1);
		*idx = point_in_neg(ro, objs);
	}
	return (0);
}

int	csg_carve_hit(t_ray *ray, t_object *objects, t_info *info)
{
	int			idx;
	t_vector	exit_pt;
	t_vector	rd;
	t_object	*solid;
	t_ray		cont;

	idx = point_in_neg(info->hitpoint, objects);
	if (idx < 0)
		return (1);
	if (!csg_exit_neg(ray, objects, &exit_pt, &idx))
		return (0);
	solid = find_solid_at(exit_pt, objects, info->e);
	if (solid)
	{
		info->e = solid;
		info->hitpoint = exit_pt;
		info->normal = normalized(sub_vec(objects[idx].translation, exit_pt));
		return (1);
	}
	rd = normalized(ray->direction);
	cont = new_ray(add_vec(exit_pt, scale_vec(rd, 0.001f)),
			add_vec(exit_pt, scale_vec(rd, 1.001f)));
	return (test_intersection(&cont, objects, info));
}
