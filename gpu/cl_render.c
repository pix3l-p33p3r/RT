/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_render.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:50:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/22 23:50:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define CL_TARGET_OPENCL_VERSION 120
#include "../third_party/OpenCL/CL/cl.h"
#include "../main.h"
#include <string.h>

typedef struct s_gpu
{
	cl_context			ctx;
	cl_command_queue	queue;
	cl_program			prog;
	cl_kernel			kern;
	cl_device_id			dev;
	int					ok;
}	t_gpu;

typedef struct s_gpu_obj
{
	int		type;
	float	pos[3];
	float	axis[3];
	float	color[3];
	float	radius;
	float	height;
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

static char	*load_file(const char *path, size_t *out_len)
{
	FILE	*f;
	char	*buf;
	long	n;

	f = fopen(path, "rb");
	if (!f)
		return (NULL);
	fseek(f, 0, SEEK_END);
	n = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (n <= 0)
		return (fclose(f), NULL);
	buf = malloc((size_t)n + 1);
	if (!buf)
		return (fclose(f), NULL);
	if (fread(buf, 1, (size_t)n, f) != (size_t)n)
		return (free(buf), fclose(f), NULL);
	buf[n] = 0;
	fclose(f);
	if (out_len)
		*out_len = (size_t)n;
	return (buf);
}

static int	pick_device(cl_device_id *dev)
{
	cl_uint			np;
	cl_uint			nd;
	cl_platform_id	plats[8];
	cl_device_id		devs[8];
	cl_uint			i;
	cl_uint			j;

	if (clGetPlatformIDs(8, plats, &np) != CL_SUCCESS || np == 0)
		return (0);
	i = 0;
	while (i < np)
	{
		if (clGetDeviceIDs(plats[i], CL_DEVICE_TYPE_GPU, 8, devs, &nd)
			== CL_SUCCESS && nd > 0)
		{
			*dev = devs[0];
			return (1);
		}
		i++;
	}
	i = 0;
	while (i < np)
	{
		if (clGetDeviceIDs(plats[i], CL_DEVICE_TYPE_ALL, 8, devs, &nd)
			== CL_SUCCESS && nd > 0)
		{
			j = 0;
			while (j < nd)
			{
				*dev = devs[j];
				return (1);
			}
		}
		i++;
	}
	return (0);
}

int	gpu_init(t_vars *vars)
{
	t_gpu		*g;
	char		*src;
	size_t		slen;
	cl_int		err;
	char		logbuf[2048];
	size_t		logn;

	vars->gpu = NULL;
	vars->gpu_ok = 0;
	if (!vars->use_gpu)
		return (0);
	g = ft_calloc(1, sizeof(t_gpu));
	if (!g || !pick_device(&g->dev))
		return (free(g), printf("[rt] OpenCL: no device (install mesa/intel ICD)\n"), 0);
	g->ctx = clCreateContext(NULL, 1, &g->dev, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		return (free(g), 0);
	g->queue = clCreateCommandQueue(g->ctx, g->dev, 0, &err);
	src = load_file("gpu/kernels/trace.cl", &slen);
	if (!src)
		return (printf("[rt] OpenCL: missing gpu/kernels/trace.cl\n"), 0);
	g->prog = clCreateProgramWithSource(g->ctx, 1, (const char **)&src,
			&slen, &err);
	free(src);
	err = clBuildProgram(g->prog, 1, &g->dev, "-cl-std=CL1.2", NULL, NULL);
	if (err != CL_SUCCESS)
	{
		clGetProgramBuildInfo(g->prog, g->dev, CL_PROGRAM_BUILD_LOG,
			sizeof(logbuf), logbuf, &logn);
		printf("[rt] OpenCL build failed:\n%s\n", logbuf);
		return (0);
	}
	g->kern = clCreateKernel(g->prog, "trace_frame", &err);
	if (err != CL_SUCCESS)
		return (0);
	g->ok = 1;
	vars->gpu = g;
	vars->gpu_ok = 1;
	printf("[rt] OpenCL GPU path ready (RT_USE_GPU=1)\n");
	return (1);
}

void	gpu_shutdown(t_vars *vars)
{
	t_gpu	*g;

	g = (t_gpu *)vars->gpu;
	if (!g)
		return ;
	if (g->kern)
		clReleaseKernel(g->kern);
	if (g->prog)
		clReleaseProgram(g->prog);
	if (g->queue)
		clReleaseCommandQueue(g->queue);
	if (g->ctx)
		clReleaseContext(g->ctx);
	free(g);
	vars->gpu = NULL;
	vars->gpu_ok = 0;
}

static void	pack_cam(t_gpu_cam *c, t_camera *cam)
{
	c->origin[0] = cam->origin.x;
	c->origin[1] = cam->origin.y;
	c->origin[2] = cam->origin.z;
	c->center[0] = cam->screen_center.x;
	c->center[1] = cam->screen_center.y;
	c->center[2] = cam->screen_center.z;
	c->u[0] = cam->screen_u.x;
	c->u[1] = cam->screen_u.y;
	c->u[2] = cam->screen_u.z;
	c->v[0] = cam->screen_v.x;
	c->v[1] = cam->screen_v.y;
	c->v[2] = cam->screen_v.z;
	c->horizontal = cam->horizontal;
	c->aspect = cam->aspect_ratio;
}

static void	pack_obj(t_gpu_obj *o, t_object *e)
{
	o->type = e->type;
	o->pos[0] = e->translation.x;
	o->pos[1] = e->translation.y;
	o->pos[2] = e->translation.z;
	o->axis[0] = e->d_normal.x;
	o->axis[1] = e->d_normal.y;
	o->axis[2] = e->d_normal.z;
	o->color[0] = e->base_color.x;
	o->color[1] = e->base_color.y;
	o->color[2] = e->base_color.z;
	o->radius = e->radius;
	o->height = e->height;
	if (e->type == SPHERE)
		o->radius = e->scale.x;
}

int	gpu_raytrace(t_vars *vars)
{
	t_gpu			*g;
	t_gpu_obj		objs[128];
	t_gpu_light		lights[32];
	t_gpu_cam		cam;
	cl_mem			out;
	cl_mem			b_objs;
	cl_mem			b_lights;
	cl_mem			b_cam;
	cl_int			err;
	float			*fb;
	size_t			global;
	int				i;
	float			amb[4];
	int				nobj;
	int				nlight;
	int				w;
	int				h;

	g = (t_gpu *)vars->gpu;
	if (!g || !g->ok)
		return (0);
	vars->frames++;
	nobj = vars->pos;
	if (nobj > 128)
		nobj = 128;
	nlight = vars->pos1;
	if (nlight > 32)
		nlight = 32;
	w = WIDTH;
	h = HEIGHT;
	i = -1;
	while (++i < nobj)
		pack_obj(&objs[i], &vars->objects[i]);
	i = -1;
	while (++i < nlight)
	{
		lights[i].pos[0] = vars->lights[i].position.x;
		lights[i].pos[1] = vars->lights[i].position.y;
		lights[i].pos[2] = vars->lights[i].position.z;
		lights[i].color[0] = vars->lights[i].color.x;
		lights[i].color[1] = vars->lights[i].color.y;
		lights[i].color[2] = vars->lights[i].color.z;
		lights[i].intensity = vars->lights[i].intensity;
	}
	pack_cam(&cam, &vars->camera);
	amb[0] = vars->ambient ? vars->ambient->color.x : 0;
	amb[1] = vars->ambient ? vars->ambient->color.y : 0;
	amb[2] = vars->ambient ? vars->ambient->color.z : 0;
	amb[3] = 0;
	fb = calloc(WIDTH * HEIGHT * 3, sizeof(float));
	out = clCreateBuffer(g->ctx, CL_MEM_WRITE_ONLY,
			sizeof(float) * WIDTH * HEIGHT * 3, NULL, &err);
	b_objs = clCreateBuffer(g->ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(t_gpu_obj) * (nobj ? nobj : 1), objs, &err);
	b_lights = clCreateBuffer(g->ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(t_gpu_light) * (nlight ? nlight : 1), lights, &err);
	b_cam = clCreateBuffer(g->ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(t_gpu_cam), &cam, &err);
	clSetKernelArg(g->kern, 0, sizeof(cl_mem), &out);
	clSetKernelArg(g->kern, 1, sizeof(cl_mem), &b_cam);
	clSetKernelArg(g->kern, 2, sizeof(cl_mem), &b_objs);
	clSetKernelArg(g->kern, 3, sizeof(int), &nobj);
	clSetKernelArg(g->kern, 4, sizeof(cl_mem), &b_lights);
	clSetKernelArg(g->kern, 5, sizeof(int), &nlight);
	clSetKernelArg(g->kern, 6, sizeof(cl_float3), amb);
	clSetKernelArg(g->kern, 7, sizeof(int), &w);
	clSetKernelArg(g->kern, 8, sizeof(int), &h);
	clSetKernelArg(g->kern, 9, sizeof(int), &vars->frames);
	global = (size_t)WIDTH * (size_t)HEIGHT;
	err = clEnqueueNDRangeKernel(g->queue, g->kern, 1, NULL, &global,
			NULL, 0, NULL, NULL);
	if (err != CL_SUCCESS)
		return (free(fb), 0);
	clEnqueueReadBuffer(g->queue, out, CL_TRUE, 0,
			sizeof(float) * WIDTH * HEIGHT * 3, fb, 0, NULL, NULL);
	i = -1;
	while (++i < WIDTH * HEIGHT)
	{
		t_vector	c;
		int			y;
		int			x;

		y = i / WIDTH;
		x = i % WIDTH;
		c = new_vec(fb[i * 3], fb[i * 3 + 1], fb[i * 3 + 2]);
		vars->buffer_img[i] = add_vec(vars->buffer_img[i], c);
		c = scale_vec(vars->buffer_img[i], 1.0f / vars->frames);
		set_pixel(vars->image, x, y, c);
	}
	clReleaseMemObject(out);
	clReleaseMemObject(b_objs);
	clReleaseMemObject(b_lights);
	clReleaseMemObject(b_cam);
	free(fb);
	return (1);
}
