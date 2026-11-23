/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_shapes2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:55:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:55:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

static void	finish_disk(t_vars *var, t_object *obj, char *line, int i)
{
	parse_optional_mat(obj, line, &i);
	obj->has_material = 1;
	obj->shininess = 20;
	obj->reflectivity = 0.75;
	obj->rotation = (t_vector){0, 0, 0};
	obj->scale = (t_vector){1, 1, 1};
	obj->gtfm = set_transform(obj->translation, obj->rotation, obj->scale);
	obj->hit = test_disk;
	add_object(var, *obj);
	free(line);
}

void	check_disk(t_vars *var, char *line, int i)
{
	t_object	obj;

	obj = (t_object){0};
	i += 2;
	obj.type = DISK;
	obj.translation = parse_vector(line, &i);
	obj.d_normal = parse_vector(line, &i);
	check_numbers(obj.d_normal.x, -1.0, 1.0);
	check_numbers(obj.d_normal.y, -1.0, 1.0);
	check_numbers(obj.d_normal.z, -1.0, 1.0);
	obj.radius = parse_number(line, &i) / 2.0f;
	obj.base_color = parse_vector(line, &i);
	check_numbers(obj.base_color.x, 0, 255);
	check_numbers(obj.base_color.y, 0, 255);
	check_numbers(obj.base_color.z, 0, 255);
	obj.base_color = scale_vec(obj.base_color, 1.0f / 255.0f);
	finish_disk(var, &obj, line, i);
}

static void	finish_paraboloid(t_object *obj)
{
	obj->has_material = 1;
	obj->shininess = 25;
	obj->reflectivity = 0.75;
	obj->rotation = get_rotation_vector(obj->d_normal);
	obj->scale = (t_vector){obj->radius, obj->radius, obj->height};
	obj->gtfm = set_transform(obj->translation, obj->rotation, obj->scale);
	obj->hit = test_paraboloid;
}

void	check_paraboloid(t_vars *var, char *line, int i)
{
	t_object	obj;

	obj = (t_object){0};
	i += 2;
	obj.type = PARABOLOID;
	obj.translation = parse_vector(line, &i);
	obj.d_normal = parse_vector(line, &i);
	check_numbers(obj.d_normal.x, -1.0, 1.0);
	check_numbers(obj.d_normal.y, -1.0, 1.0);
	check_numbers(obj.d_normal.z, -1.0, 1.0);
	obj.radius = parse_number(line, &i) / 2.0f;
	obj.height = parse_number(line, &i);
	obj.base_color = parse_vector(line, &i);
	check_numbers(obj.base_color.x, 0, 255);
	check_numbers(obj.base_color.y, 0, 255);
	check_numbers(obj.base_color.z, 0, 255);
	obj.base_color = scale_vec(obj.base_color, 1.0f / 255.0f);
	parse_optional_mat(&obj, line, &i);
	finish_paraboloid(&obj);
	add_object(var, obj);
	free(line);
}

void	check_dir_light(t_vars *var, char *line, int i)
{
	t_light	light;
	t_light	*tmp;

	i++;
	light.position = normalized(parse_vector(line, &i));
	light.intensity = parse_number(line, &i);
	light.color = parse_vector(line, &i);
	check_numbers(light.intensity, 0.0, 1.0);
	check_numbers(light.color.x, 0, 255);
	check_numbers(light.color.y, 0, 255);
	check_numbers(light.color.z, 0, 255);
	light.color = scale_vec(light.color, 1.0f / 255.0f);
	light.is_dir = 1;
	if (line[i])
		unexpected_error(line, i);
	var->lights[var->pos1++] = light;
	if (var->pos1 + 1 == var->len1)
	{
		tmp = ft_calloc(var->len1 * 2 + 1, sizeof(t_light));
		ft_memcpy(tmp, var->lights, var->len1 * sizeof(t_light));
		free(var->lights);
		var->lights = tmp;
		var->len1 = var->len1 * 2;
	}
	free(line);
}
