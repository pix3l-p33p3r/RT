/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   raytrace_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/26 23:21:23 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

unsigned int	rand_pcg(unsigned int *rng_state)
{
	unsigned int	state;
	unsigned int	word;

	state = *rng_state;
	*rng_state = state * 747796405u + 2891336453u;
	word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return ((word >> 22u) ^ word);
}

float	random_float(unsigned int *rng_state, float min, float max)
{
	return (min + (rand_pcg(rng_state) * (1.0 / UINT_MAX)
		) * (max - min));
}

void	process_pixel(t_vars *vars, t_norm1 *norm, unsigned int *rng)
{
	norm->xoff = random_float(rng, -0.5, 0.5);
	norm->yoff = random_float(rng, -0.5, 0.5);
	norm->normx = (norm->xfact * (float)(norm->x + norm->xoff)) - 1.0;
	norm->normy = (norm->yfact * (float)(norm->y + norm->yoff)) - 1.0;
	norm->ray = generate_ray(norm->normx, norm->normy, &vars->camera);
	norm->intfound = test_intersection(&norm->ray, vars->objects, &norm->info);
	if (norm->intfound)
	{
		norm->color = compute_color(vars, &norm->info, &norm->ray);
		vars->buffer_img[norm->y * WIDTH + norm->x]
			= add_vec(vars->buffer_img[norm->y * WIDTH + norm->x], norm->color);
		norm->color = scale_vec(vars->buffer_img[
				norm->y * WIDTH + norm->x], 1.0f / vars->frames);
		set_pixel(vars->image, norm->x, norm->y, norm->color);
	}
}

void	raytrace_rows(t_thr_arg *arg)
{
	t_norm1			norm;
	unsigned int	rng;

	norm.xfact = arg->xfact;
	norm.yfact = arg->yfact;
	rng = arg->rng;
	norm.y = arg->y0 - 1;
	while (++norm.y < arg->y1)
	{
		norm.x = -1;
		while (++norm.x < WIDTH)
			process_pixel(arg->vars, &norm, &rng);
	}
}

void	raytrace(t_vars *vars)
{
	if (vars->use_gpu && vars->gpu_ok && gpu_raytrace(vars))
		return ;
	threaded_raytrace(vars);
}
