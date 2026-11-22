/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cone_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/22 23:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static int	cone_props_side(t_vector *poi, t_info *info)
{
	t_vector	orgnormal;

	info->hitpoint = apply_transform_vector(poi, info->e->gtfm, FORWARD);
	orgnormal = new_vec(poi->x, poi->y, -poi->z);
	orgnormal = apply_transform_vector(&orgnormal, info->e->gtfm, FORWARD);
	info->normal = normalized(sub_vec(orgnormal, info->e->translation));
	return (1);
}

static int	cone_props_cap(t_vector *poi, t_info *info, t_vector *vhat)
{
	t_vector	orgnormal;

	if (fabsf(vhat->z) < EPSILON)
		return (0);
	if (sqrtf(square(poi->x) + square(poi->y)) >= 1.0f)
		return (0);
	info->hitpoint = apply_transform_vector(poi, info->e->gtfm, FORWARD);
	orgnormal = new_vec(0, 0, poi->z);
	orgnormal = apply_transform_vector(&orgnormal, info->e->gtfm, FORWARD);
	info->normal = normalized(sub_vec(orgnormal, info->e->translation));
	return (1);
}

int	cone_calculate_props(int min_index, t_vector *poi,
		t_info *info, t_vector *vhat)
{
	if (min_index < 2)
		return (cone_props_side(poi, info));
	return (cone_props_cap(poi, info, vhat));
}
