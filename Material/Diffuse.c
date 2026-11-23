/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Diffuse.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/17 22:07:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:47:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static void	add_light_diff(t_light *light, t_vars *vars,
		t_info *info, t_norm5 *norm)
{
	t_vector	atten;
	t_vector	lighdir;
	float		ndl;

	atten = shadow_atten(light, vars->objects, info);
	if (atten.x <= 0.0f && atten.y <= 0.0f && atten.z <= 0.0f)
		return ;
	lighdir = normalized(sub_vec(light->position, info->hitpoint));
	ndl = fmaxf(dot_product(info->normal, lighdir), 0.0f) * light->intensity;
	norm->red += light->color.x * atten.x * ndl;
	norm->green += light->color.y * atten.y * ndl;
	norm->blue += light->color.z * atten.z * ndl;
}

t_vector	diffuse(t_info *info, t_vars *vars)
{
	t_norm5	norm;

	norm.i = -1;
	norm.red = 0;
	norm.green = 0;
	norm.blue = 0;
	while (++norm.i < vars->pos1)
		add_light_diff(&vars->lights[norm.i], vars, info, &norm);
	norm.diff_color = new_vec(norm.red * info->e->base_color.x,
			norm.green * info->e->base_color.y,
			norm.blue * info->e->base_color.z);
	return (norm.diff_color);
}
