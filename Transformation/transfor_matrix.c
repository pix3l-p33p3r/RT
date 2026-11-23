/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   transfor_matrix.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/08 06:32:54 by elel-yak          #+#    #+#             */
/*   Updated: 2023/12/26 20:14:02 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

t_vector	apply_transform_vector(t_vector *input,
	t_matrix **gtfm, int dirflag)
{
	t_matrix	*m;
	t_vector	output;

	if (!gtfm)
		return (*input);
	if (dirflag)
		m = gtfm[0];
	else
		m = gtfm[1];
	if (!m || !m->matrix)
		return ((t_vector){0, 0, 0});
	output.x = m->matrix[0][0] * input->x + m->matrix[0][1] * input->y
		+ m->matrix[0][2] * input->z + m->matrix[0][3];
	output.y = m->matrix[1][0] * input->x + m->matrix[1][1] * input->y
		+ m->matrix[1][2] * input->z + m->matrix[1][3];
	output.z = m->matrix[2][0] * input->x + m->matrix[2][1] * input->y
		+ m->matrix[2][2] * input->z + m->matrix[2][3];
	return (output);
}

t_ray	apply_transform(t_ray *ray, t_matrix **gtfm, int dirflag)
{
	t_ray	new_ray;

	new_ray.point1 = apply_transform_vector(&ray->point1, gtfm, dirflag);
	new_ray.point2 = apply_transform_vector(&ray->point2, gtfm, dirflag);
	new_ray.direction = sub_vec(new_ray.point2, new_ray.point1);
	return (new_ray);
}

t_matrix	**set_transform(t_vector t, t_vector r, t_vector s)
{
	t_matrix	**transform;
	t_matrix	*forward;
	t_matrix	*backward;

	transform = malloc(sizeof(t_matrix *) * 2);
	forward = safe_matrix_multy(translation_matrix(&t), rotation_matrix_x(r.x));
	forward = safe_matrix_multy(forward, rotation_matrix_y(r.y));
	forward = safe_matrix_multy(forward, rotation_matrix_z(r.z));
	forward = safe_matrix_multy(forward, scaling_matrix(&s));
	backward = inverse(forward);
	transform[0] = forward;
	transform[1] = backward;
	return (transform);
}
