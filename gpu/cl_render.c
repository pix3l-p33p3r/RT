/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_render.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:50:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cl_internal.h"

static void	set_kernel_args(t_gpu_job *j)
{
	t_gpu_dev	*d;
	int			w;
	int			h;

	d = j->d;
	w = WIDTH;
	h = HEIGHT;
	clSetKernelArg(d->kern, 0, sizeof(cl_mem), &d->b_out);
	clSetKernelArg(d->kern, 1, sizeof(cl_mem), &d->b_cam);
	clSetKernelArg(d->kern, 2, sizeof(cl_mem), &d->b_objs);
	clSetKernelArg(d->kern, 3, sizeof(int), &j->g->nobj);
	clSetKernelArg(d->kern, 4, sizeof(cl_mem), &d->b_lights);
	clSetKernelArg(d->kern, 5, sizeof(int), &j->g->nlight);
	clSetKernelArg(d->kern, 6, sizeof(cl_float3), j->amb);
	clSetKernelArg(d->kern, 7, sizeof(int), &w);
	clSetKernelArg(d->kern, 8, sizeof(int), &h);
	clSetKernelArg(d->kern, 9, sizeof(int), &d->y_start);
	clSetKernelArg(d->kern, 10, sizeof(int), &d->y_count);
	clSetKernelArg(d->kern, 11, sizeof(int), &j->vars->frames);
}

static void	accum_band(t_gpu_job *j)
{
	int			pix;
	int			end;
	t_vector	c;

	pix = j->d->y_start * WIDTH;
	end = (j->d->y_start + j->d->y_count) * WIDTH;
	while (pix < end)
	{
		c = new_vec(j->g->fb[pix * 3], j->g->fb[pix * 3 + 1],
				j->g->fb[pix * 3 + 2]);
		j->vars->buffer_img[pix] = add_vec(j->vars->buffer_img[pix], c);
		c = scale_vec(j->vars->buffer_img[pix], 1.0f / j->vars->frames);
		set_pixel(j->vars->image, pix % WIDTH, pix / WIDTH, c);
		pix++;
	}
}

static void	*gpu_worker(void *p)
{
	t_gpu_job	*j;
	size_t		global;
	size_t		bytes;
	float		*dst;
	cl_int		err;

	j = (t_gpu_job *)p;
	global = (size_t)WIDTH * (size_t)j->d->y_count;
	set_kernel_args(j);
	err = clEnqueueNDRangeKernel(j->d->queue, j->d->kern, 1, NULL, &global,
			NULL, 0, NULL, NULL);
	if (err != CL_SUCCESS)
		return (j->ok = 0, NULL);
	bytes = sizeof(float) * WIDTH * j->d->y_count * 3;
	dst = j->g->fb + ((size_t)j->d->y_start * WIDTH * 3);
	err = clEnqueueReadBuffer(j->d->queue, j->d->b_out, CL_TRUE, 0,
			bytes, dst, 0, NULL, NULL);
	if (err != CL_SUCCESS)
		return (j->ok = 0, NULL);
	accum_band(j);
	j->ok = 1;
	return (NULL);
}

static void	init_gpu_job(t_gpu_job *j, t_gpu *g, t_vars *vars, int idx)
{
	j->g = g;
	j->d = &g->devs[idx];
	j->vars = vars;
	j->amb[0] = 0.0f;
	j->amb[1] = 0.0f;
	j->amb[2] = 0.0f;
	j->amb[3] = 0.0f;
	if (vars->ambient)
	{
		j->amb[0] = vars->ambient->color.x;
		j->amb[1] = vars->ambient->color.y;
		j->amb[2] = vars->ambient->color.z;
	}
	j->ok = 0;
}

int	gpu_raytrace(t_vars *vars)
{
	t_gpu		*g;
	pthread_t	tids[MAX_GPUS];
	t_gpu_job	jobs[MAX_GPUS];
	int			i;
	int			all_ok;

	g = (t_gpu *)vars->gpu;
	if (!g || !g->ok || !cl_upload_scene(g, vars))
		return (0);
	vars->frames++;
	i = -1;
	while (++i < g->ndevs)
	{
		init_gpu_job(&jobs[i], g, vars, i);
		if (pthread_create(&tids[i], NULL, gpu_worker, &jobs[i]) != 0)
			tids[i] = 0;
	}
	all_ok = 1;
	while (--i >= 0)
	{
		if (tids[i])
			pthread_join(tids[i], NULL);
		all_ok &= jobs[i].ok;
	}
	return (all_ok);
}
