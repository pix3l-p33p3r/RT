/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stereo.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 04:00:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 04:00:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

t_ray	stereo_eye_ray(t_vars *vars, float nx, float ny, float eye_sign)
{
	t_ray		base;
	t_vector	eye;
	t_vector	focal;

	base = generate_ray(nx, ny, &vars->camera);
	eye = add_vec(vars->camera.origin,
			scale_vec(vars->camera.cam_u, eye_sign * 0.18f));
	focal = add_vec(vars->camera.origin,
			scale_vec(base.direction, 8.0f));
	return (new_ray(eye, focal));
}

static t_vector	trace_sbs_pixel(t_vars *vars, t_norm1 *norm)
{
	t_ray	ray;
	float	nx;

	if (norm->x < WIDTH / 2)
	{
		nx = norm->normx * 2.0f + 1.0f;
		ray = stereo_eye_ray(vars, nx, norm->normy, -1.0f);
		return (trace_ray(vars, &ray, MAX_BOUNCES));
	}
	nx = norm->normx * 2.0f - 1.0f;
	ray = stereo_eye_ray(vars, nx, norm->normy, 1.0f);
	return (trace_ray(vars, &ray, MAX_BOUNCES));
}

t_vector	trace_stereo_pixel(t_vars *vars, t_norm1 *norm)
{
	t_ray		rl;
	t_ray		rr;
	t_vector	cl;
	t_vector	cr;

	if (vars->stereo_mode == 1)
	{
		rl = stereo_eye_ray(vars, norm->normx, norm->normy, -1.0f);
		rr = stereo_eye_ray(vars, norm->normx, norm->normy, 1.0f);
		cl = trace_ray(vars, &rl, MAX_BOUNCES);
		cr = trace_ray(vars, &rr, MAX_BOUNCES);
		return (new_vec(cl.x, cr.y, cr.z));
	}
	if (vars->stereo_mode == 2)
		return (trace_sbs_pixel(vars, norm));
	norm->ray = generate_ray(norm->normx, norm->normy, &vars->camera);
	return (trace_ray(vars, &norm->ray, MAX_BOUNCES));
}

t_vector	jitter_light_pos(t_light *light, unsigned int *rng)
{
	t_vector	off;
	int			tries;

	if (light->is_dir || light->radius <= 0.0f)
		return (light->position);
	tries = 0;
	while (tries < 8)
	{
		off = new_vec(random_float(rng, -1.0f, 1.0f),
				random_float(rng, -1.0f, 1.0f),
				random_float(rng, -1.0f, 1.0f));
		if (length2(off) <= 1.0f)
			return (add_vec(light->position, scale_vec(off, light->radius)));
		tries++;
	}
	return (add_vec(light->position,
			scale_vec(normalized(off), light->radius)));
}

void	cycle_stereo(t_vars *vars)
{
	vars->stereo_mode = (vars->stereo_mode + 1) % 3;
	if (vars->stereo_mode == 0)
		printf("[rt] Stereo: Mono\n");
	else if (vars->stereo_mode == 1)
		printf("[rt] Stereo: Red-Cyan Anaglyph 3D\n");
	else
		printf("[rt] Stereo: Side-by-Side VR\n");
	fflush(stdout);
	reset_accum(vars);
}
