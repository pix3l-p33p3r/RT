/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   raytrace.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchaguer <hchaguer@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/14 14:19:37 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 04:05:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

int	test_intersection(t_ray *ray, t_object *objects, t_info *info)
{
	t_norm2	norm;

	norm.intfound = 0;
	norm.mindist = 10e6;
	norm.i = -1;
	while (objects[++norm.i].gtfm)
	{
		if (objects[norm.i].is_neg)
			continue ;
		norm.test.e = &objects[norm.i];
		norm.validint = objects[norm.i].hit(ray, &norm.test);
		if (norm.validint == 1)
		{
			norm.dist = length(sub_vec(norm.test.hitpoint, ray->point1));
			if (norm.dist < norm.mindist)
			{
				norm.intfound = 1;
				norm.mindist = norm.dist;
				*info = norm.test;
			}
		}
	}
	if (norm.intfound)
		return (csg_carve_hit(ray, objects, info));
	return (0);
}
