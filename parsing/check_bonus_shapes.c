/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_bonus_shapes.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 03:58:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 03:58:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

void	append_light(t_vars *var, t_light light)
{
	t_light	*tmp;

	var->lights[var->pos1++] = light;
	if (var->pos1 + 1 == var->len1)
	{
		tmp = ft_calloc(var->len1 * 2 + 1, sizeof(t_light));
		ft_memcpy(tmp, var->lights, var->len1 * sizeof(t_light));
		free(var->lights);
		var->lights = tmp;
		var->len1 = var->len1 * 2;
	}
}

static void	finish_torus(t_object *obj)
{
	obj->has_material = 1;
	obj->shininess = 30;
	obj->reflectivity = 0.8;
	obj->rotation = get_rotation_vector(obj->d_normal);
	obj->scale = (t_vector){1, 1, 1};
	obj->gtfm = set_transform(obj->translation, obj->rotation, obj->scale);
	obj->hit = test_torus;
}

void	check_torus(t_vars *var, char *line, int i)
{
	t_object	obj;

	obj = (t_object){0};
	i += 2;
	obj.type = TORUS;
	obj.translation = parse_vector(line, &i);
	obj.d_normal = parse_vector(line, &i);
	check_numbers(obj.d_normal.x, -1.0, 1.0);
	check_numbers(obj.d_normal.y, -1.0, 1.0);
	check_numbers(obj.d_normal.z, -1.0, 1.0);
	obj.radius = parse_number(line, &i);
	obj.height = parse_number(line, &i);
	obj.base_color = parse_vector(line, &i);
	check_numbers(obj.base_color.x, 0, 255);
	check_numbers(obj.base_color.y, 0, 255);
	check_numbers(obj.base_color.z, 0, 255);
	obj.base_color = scale_vec(obj.base_color, 1.0f / 255.0f);
	parse_optional_mat(&obj, line, &i);
	finish_torus(&obj);
	add_object(var, obj);
	free(line);
}

void	check_neg_sphere(t_vars *var, char *line, int i)
{
	t_object	obj;

	obj = (t_object){0};
	i += 2;
	obj.type = NEG_SPHERE;
	obj.is_neg = 1;
	obj.translation = parse_vector(line, &i);
	obj.radius = parse_number(line, &i) / 2.0f;
	if (line[i])
		unexpected_error(line, i);
	obj.rotation = (t_vector){0, 0, 0};
	obj.scale = (t_vector){1, 1, 1};
	obj.gtfm = set_transform((t_vector){0, 0, 0}, obj.rotation, obj.scale);
	obj.hit = test_sphere;
	add_object(var, obj);
	free(line);
}
