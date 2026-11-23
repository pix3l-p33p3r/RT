/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_cone.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:36:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	parse_optional_mat(t_object *obj, char *line, int *i)
{
	*i = skip_space(line, *i);
	if (line[*i])
	{
		obj->mirror = parse_number(line, i);
		check_numbers(obj->mirror, 0.0f, 1.0f);
	}
	if (line[*i])
		unexpected_error(line, *i);
}

static void	check_cn_continue(t_object *obj)
{
	obj->has_material = 1;
	obj->shininess = 10;
	obj->reflectivity = 0.75;
	obj->rotation = get_rotation_vector(obj->d_normal);
	obj->scale = (t_vector){obj->radius, obj->radius, obj->height};
	obj->gtfm = set_transform(obj->translation, obj->rotation, obj->scale);
	obj->hit = test_cone;
}

void	check_cone(t_vars *var, char *line, int i)
{
	t_object	obj;

	obj = (t_object){0};
	i += 2;
	obj.type = CONE;
	obj.translation = parse_vector(line, &i);
	obj.d_normal = parse_vector(line, &i);
	check_numbers(obj.d_normal.x, -1.0, 1.0);
	check_numbers(obj.d_normal.y, -1.0, 1.0);
	check_numbers(obj.d_normal.z, -1.0, 1.0);
	obj.radius = parse_number(line, &i) / 2;
	obj.height = parse_number(line, &i);
	obj.base_color = parse_vector(line, &i);
	check_numbers(obj.base_color.x, 0, 255);
	check_numbers(obj.base_color.y, 0, 255);
	check_numbers(obj.base_color.z, 0, 255);
	obj.base_color = scale_vec(obj.base_color, 1.0f / 255.0f);
	parse_optional_mat(&obj, line, &i);
	check_cn_continue(&obj);
	add_object(var, obj);
	free(line);
}
