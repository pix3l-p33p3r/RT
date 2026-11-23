/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_setup.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cl_internal.h"

static void	print_gpu_summary(t_gpu *g)
{
	int	i;

	printf("[rt] OpenCL multi-GPU ready (%d device(s)):\n", g->ndevs);
	i = 0;
	while (i < g->ndevs)
	{
		printf("  - GPU #%d: %s (%u CU) -> rows [%d..%d)\n",
			i, g->devs[i].name, g->devs[i].cu,
			g->devs[i].y_start, g->devs[i].y_start + g->devs[i].y_count);
		i++;
	}
	fflush(stdout);
}

static int	build_all_devices(t_gpu *g, const char *src, size_t slen)
{
	int	i;
	int	good;

	good = 0;
	i = 0;
	while (i < g->ndevs)
	{
		if (cl_init_device(&g->devs[i], src, slen))
		{
			if (good != i)
				g->devs[good] = g->devs[i];
			good++;
		}
		i++;
	}
	g->ndevs = good;
	return (good);
}

int	gpu_init(t_vars *vars)
{
	t_gpu	*g;
	char	*src;
	size_t	slen;

	if (!vars->use_gpu)
		return (0);
	g = ft_calloc(1, sizeof(t_gpu));
	if (!g)
		return (0);
	cl_collect_devices(g);
	src = cl_load_file("gpu/kernels/trace.cl", &slen);
	if (g->ndevs == 0 || !src)
		return (free(src), free(g), printf("[rt] OpenCL: unavailable\n"), 0);
	build_all_devices(g, src, slen);
	free(src);
	g->fb = calloc(WIDTH * HEIGHT * 3, sizeof(float));
	if (g->ndevs == 0 || !g->fb)
		return (free(g->fb), free(g), 0);
	cl_assign_bands(g);
	g->ok = 1;
	vars->gpu = g;
	vars->gpu_ok = 1;
	print_gpu_summary(g);
	return (1);
}

static void	release_device(t_gpu_dev *d)
{
	if (d->b_out)
		clReleaseMemObject(d->b_out);
	if (d->b_cam)
		clReleaseMemObject(d->b_cam);
	if (d->b_objs)
		clReleaseMemObject(d->b_objs);
	if (d->b_lights)
		clReleaseMemObject(d->b_lights);
	if (d->kern)
		clReleaseKernel(d->kern);
	if (d->prog)
		clReleaseProgram(d->prog);
	if (d->queue)
		clReleaseCommandQueue(d->queue);
	if (d->ctx)
		clReleaseContext(d->ctx);
}

void	gpu_shutdown(t_vars *vars)
{
	t_gpu	*g;
	int		i;

	g = (t_gpu *)vars->gpu;
	if (!g)
		return ;
	i = 0;
	while (i < g->ndevs)
		release_device(&g->devs[i++]);
	free(g->fb);
	free(g);
	vars->gpu = NULL;
	vars->gpu_ok = 0;
}
