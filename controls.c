/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   controls.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:54:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:54:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

void	reset_accum(t_vars *vars)
{
	vars->frames = 0;
	ft_bzero(vars->buffer_img, sizeof(t_vector) * WIDTH * HEIGHT);
	gpu_reset_camera(vars);
}

void	rebuild_camera(t_camera *cam)
{
	t_vector	up;

	cam->up_vector = new_vec(0.0f, 0.0f, 1.0f);
	cam->cam_w = normalized(cam->cam_w);
	up = cam->up_vector;
	if (fabsf(cam->cam_w.z) > 0.999f)
		up = new_vec(0.0f, 1.0f, 0.0f);
	cam->cam_u = normalized(cross_product(cam->cam_w, up));
	cam->cam_v = normalized(cross_product(cam->cam_u, cam->cam_w));
	cam->lookat = add_vec(cam->origin, cam->cam_w);
	cam->screen_center = add_vec(cam->origin,
			scale_vec(cam->cam_w, cam->focal_length));
	cam->screen_u = scale_vec(cam->cam_u, cam->horizontal);
	cam->screen_v = scale_vec(cam->cam_v, cam->horizontal / cam->aspect_ratio);
}

static int	move_camera(int keycode, t_camera *cam)
{
	t_vector	delta;

	if (keycode == 119)
		delta = scale_vec(cam->cam_w, 0.5f);
	else if (keycode == 115)
		delta = scale_vec(cam->cam_w, -0.5f);
	else if (keycode == 97)
		delta = scale_vec(cam->cam_u, -0.5f);
	else if (keycode == 100)
		delta = scale_vec(cam->cam_u, 0.5f);
	else if (keycode == 113)
		delta = scale_vec(cam->up_vector, -0.5f);
	else if (keycode == 101)
		delta = scale_vec(cam->up_vector, 0.5f);
	else
		return (0);
	cam->origin = add_vec(cam->origin, delta);
	return (1);
}

static int	rotate_camera(int keycode, t_camera *cam)
{
	float		a;
	t_vector	next;

	a = 0.08f;
	if (keycode == 65361 || keycode == 65363)
	{
		if (keycode == 65363)
			a = -0.08f;
		next.x = cam->cam_w.x * cosf(a) - cam->cam_w.y * sinf(a);
		next.y = cam->cam_w.x * sinf(a) + cam->cam_w.y * cosf(a);
		next.z = cam->cam_w.z;
		cam->cam_w = normalized(next);
		return (1);
	}
	if (keycode != 65362 && keycode != 65364)
		return (0);
	if (keycode == 65362)
		a = -0.08f;
	next = normalized(add_vec(scale_vec(cam->cam_w, cosf(a)),
				scale_vec(cam->cam_v, sinf(a))));
	if (fabsf(next.z) >= 0.99f)
		return (0);
	cam->cam_w = next;
	return (1);
}

int	handle_keypress(int keycode, t_vars *vars)
{
	if (keycode == 65307)
		close_hook(vars);
	else if (keycode == 112)
		export_ppm(vars, "render_out.ppm");
	else if (keycode == 118)
		cycle_stereo(vars);
	else if (keycode == 103)
	{
		vars->use_gpu = !vars->use_gpu;
		if (vars->use_gpu && !vars->gpu_ok && !gpu_init(vars))
			vars->use_gpu = 0;
		if (vars->use_gpu)
			printf("[rt] Mode: GPU\n");
		else
			printf("[rt] Mode: CPU\n");
		fflush(stdout);
		reset_accum(vars);
	}
	else if (move_camera(keycode, &vars->camera)
		|| rotate_camera(keycode, &vars->camera))
	{
		rebuild_camera(&vars->camera);
		reset_accum(vars);
	}
	return (0);
}
