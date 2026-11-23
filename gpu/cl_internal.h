/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_internal.h                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CL_INTERNAL_H
# define CL_INTERNAL_H

# include <CL/cl.h>
# include "../main.h"
# include <string.h>

# define MAX_GPUS 8

typedef struct s_gpu_obj
{
	int		type;
	float	pos[3];
	float	axis[3];
	float	color[3];
	float	radius;
	float	height;
	float	shininess;
	float	reflectivity;
	float	mirror;
	float	transp;
	float	ior;
}	t_gpu_obj;

typedef struct s_gpu_light
{
	float	pos[3];
	float	color[3];
	float	intensity;
}	t_gpu_light;

typedef struct s_gpu_cam
{
	float	origin[3];
	float	center[3];
	float	u[3];
	float	v[3];
	float	horizontal;
	float	aspect;
}	t_gpu_cam;

typedef struct s_gpu_dev
{
	cl_device_id		dev;
	cl_context			ctx;
	cl_command_queue	queue;
	cl_program			prog;
	cl_kernel			kern;
	cl_mem				b_out;
	cl_mem				b_cam;
	cl_mem				b_objs;
	cl_mem				b_lights;
	cl_uint				cu;
	cl_uint				vendor_id;
	char				name[128];
	int					y_start;
	int					y_count;
}	t_gpu_dev;

typedef struct s_gpu
{
	t_gpu_dev	devs[MAX_GPUS];
	int			ndevs;
	int			scene_uploaded;
	int			nobj;
	int			nlight;
	float		*fb;
	int			ok;
}	t_gpu;

typedef struct s_gpu_job
{
	t_gpu		*g;
	t_gpu_dev	*d;
	t_vars		*vars;
	float		amb[4];
	int			ok;
}	t_gpu_job;

typedef struct s_gpu_scene
{
	t_gpu_obj	objs[128];
	t_gpu_light	lights[32];
	t_gpu_cam	cam;
}	t_gpu_scene;

char	*cl_load_file(const char *path, size_t *out_len);
void	cl_collect_devices(t_gpu *g);
void	cl_assign_bands(t_gpu *g);
int		cl_init_device(t_gpu_dev *d, const char *src, size_t slen);
int		cl_upload_scene(t_gpu *g, t_vars *vars);

#endif
