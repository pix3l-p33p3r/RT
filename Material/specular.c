/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   specular.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/17 23:18:20 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:47:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static void	add_light_spec(t_norm6 *norm, t_info *info,
		t_vars *vars, t_ray *camera_ray)
{
	t_vector	atten;
	t_light		*l;

	l = &vars->lights[norm->i];
	atten = shadow_atten(l, vars->objects, info);
	if (atten.x <= 0.0f && atten.y <= 0.0f && atten.z <= 0.0f)
		return ;
	if (l->is_dir)
		norm->lighdir = l->position;
	else
		norm->lighdir = normalized(sub_vec(l->position, info->hitpoint));
	norm->r = reflect(norm->lighdir, info->normal);
	norm->v = normalized(camera_ray->direction);
	norm->prod = dot_product(norm->r, norm->v);
	if (norm->prod <= 0.0f)
		return ;
	norm->intensity = info->e->reflectivity
		* powf(norm->prod, info->e->shininess) * l->intensity;
	norm->red += l->color.x * atten.x * norm->intensity;
	norm->green += l->color.y * atten.y * norm->intensity;
	norm->blue += l->color.z * atten.z * norm->intensity;
}

t_vector	specular_highlight(t_info *info, t_vars *vars, t_ray *camera_ray)
{
	t_norm6	norm;

	norm.i = -1;
	norm.red = 0;
	norm.green = 0;
	norm.blue = 0;
	while (++norm.i < vars->pos1)
		add_light_spec(&norm, info, vars, camera_ray);
	return (new_vec(norm.red, norm.green, norm.blue));
}

t_vector	compute_color(t_vars *vars, t_info *info, t_ray *camera_ray)
{
	t_vector	color;
	t_vector	diff_color;
	t_vector	spc_color;

	diff_color = diffuse(info, vars);
	spc_color = new_vec(0, 0, 0);
	if (info->e->has_material == 1 && info->e->shininess > 0)
		spc_color = specular_highlight(info, vars, camera_ray);
	color = add_vec(diff_color, spc_color);
	color = add_vec(color, vars->ambient->color);
	return (color);
}
