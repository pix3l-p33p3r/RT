/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_camera.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 02:00:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 02:00:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cl_internal.h"

void	gpu_reset_camera(t_vars *vars)
{
	t_gpu		*g;
	t_gpu_cam	cam;
	int			i;

	if (!vars->gpu_ok || !vars->gpu)
		return ;
	g = (t_gpu *)vars->gpu;
	if (!g->scene_uploaded)
		return ;
	cl_pack_cam(&cam, vars);
	i = 0;
	while (i < g->ndevs)
	{
		if (g->devs[i].b_cam)
			clEnqueueWriteBuffer(g->devs[i].queue, g->devs[i].b_cam,
				CL_TRUE, 0, sizeof(t_gpu_cam), &cam, 0, NULL, NULL);
		i++;
	}
}
