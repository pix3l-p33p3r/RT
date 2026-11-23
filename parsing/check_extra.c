/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_extra.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:32:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:32:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	check_triangle(t_vars *var, char *line, int i)
{
	t_object	obj;

	obj = (t_object){0};
	i += 2;
	obj.type = TRIANGLE;
	obj.translation = parse_vector(line, &i);
	obj.d_normal = parse_vector(line, &i);
	obj.scale = parse_vector(line, &i);
	obj.base_color = parse_vector(line, &i);
	check_numbers(obj.base_color.x, 0, 255);
	check_numbers(obj.base_color.y, 0, 255);
	check_numbers(obj.base_color.z, 0, 255);
	obj.base_color = scale_vec(obj.base_color, 1.0f / 255.0f);
	parse_optional_mat(&obj, line, &i);
	obj.has_material = 1;
	obj.shininess = 15;
	obj.reflectivity = 0.5;
	obj.gtfm = set_transform((t_vector){0, 0, 0},
			(t_vector){0, 0, 0}, (t_vector){1, 1, 1});
	obj.hit = test_triangle;
	add_object(var, obj);
	free(line);
}

static void	finish_half_sphere(t_vars *var, t_object *obj, char *line, int i)
{
	parse_optional_mat(obj, line, &i);
	obj->has_material = 1;
	obj->shininess = 25;
	obj->reflectivity = 0.75;
	obj->rotation = (t_vector){0, 0, 0};
	obj->scale = (t_vector){obj->radius, obj->radius, obj->radius};
	obj->gtfm = set_transform(obj->translation, obj->rotation, obj->scale);
	obj->hit = test_half_sphere;
	add_object(var, *obj);
	free(line);
}

void	check_half_sphere(t_vars *var, char *line, int i)
{
	t_object	obj;

	obj = (t_object){0};
	i += 2;
	obj.type = HALF_SPHERE;
	obj.translation = parse_vector(line, &i);
	obj.d_normal = parse_vector(line, &i);
	check_numbers(obj.d_normal.x, -1.0, 1.0);
	check_numbers(obj.d_normal.y, -1.0, 1.0);
	check_numbers(obj.d_normal.z, -1.0, 1.0);
	obj.radius = parse_number(line, &i) / 2;
	obj.base_color = parse_vector(line, &i);
	check_numbers(obj.base_color.x, 0, 255);
	check_numbers(obj.base_color.y, 0, 255);
	check_numbers(obj.base_color.z, 0, 255);
	obj.base_color = scale_vec(obj.base_color, 1.0f / 255.0f);
	finish_half_sphere(var, &obj, line, i);
}
