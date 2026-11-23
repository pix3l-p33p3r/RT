/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_render.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:50:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:14:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include "../main.h"
#include <string.h>

#define MAX_GPUS 8

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

static int	already_picked(t_gpu *g, cl_uint vendor_id, const char *name)
{
	int	k;

	k = 0;
	while (k < g->ndevs)
	{
		if (g->devs[k].vendor_id == vendor_id
			&& strcmp(g->devs[k].name, name) == 0)
			return (1);
		k++;
	}
	return (0);
}

static void	collect_gpu_devices(t_gpu *g)
{
	cl_platform_id	plats[8];
	cl_device_id	devs[8];
	cl_uint			np;
	cl_uint			nd;
	cl_uint			i;
	cl_uint			j;
	char			name[128];
	cl_uint			vid;
	cl_uint			cu;

	g->ndevs = 0;
	if (clGetPlatformIDs(8, plats, &np) != CL_SUCCESS || np == 0)
		return ;
	i = 0;
	while (i < np && g->ndevs < MAX_GPUS)
	{
		if (clGetDeviceIDs(plats[i], CL_DEVICE_TYPE_GPU, 8, devs, &nd)
			== CL_SUCCESS && nd > 0)
		{
			j = 0;
			while (j < nd && g->ndevs < MAX_GPUS)
			{
				name[0] = 0;
				vid = 0;
				cu = 1;
				clGetDeviceInfo(devs[j], CL_DEVICE_NAME,
					sizeof(name), name, NULL);
				clGetDeviceInfo(devs[j], CL_DEVICE_VENDOR_ID,
					sizeof(vid), &vid, NULL);
				clGetDeviceInfo(devs[j], CL_DEVICE_MAX_COMPUTE_UNITS,
					sizeof(cu), &cu, NULL);
				if (!already_picked(g, vid, name))
				{
					g->devs[g->ndevs].dev = devs[j];
					g->devs[g->ndevs].vendor_id = vid;
					g->devs[g->ndevs].cu = cu ? cu : 1;
					snprintf(g->devs[g->ndevs].name,
						sizeof(g->devs[g->ndevs].name), "%s", name);
					g->ndevs++;
				}
				j++;
			}
		}
		i++;
	}
}

static void	assign_row_bands(t_gpu *g)
{
	unsigned int	total_cu;
	int				i;
	int				y;
	int				rows;

	total_cu = 0;
	i = 0;
	while (i < g->ndevs)
	{
		total_cu += g->devs[i].cu;
		i++;
	}
	if (total_cu == 0)
		total_cu = (unsigned int)g->ndevs;
	y = 0;
	i = 0;
	while (i < g->ndevs)
	{
		g->devs[i].y_start = y;
		if (i == g->ndevs - 1)
			rows = HEIGHT - y;
		else
		{
			rows = (int)(((long)HEIGHT * g->devs[i].cu) / total_cu);
			if (rows < 1)
				rows = 1;
			if (y + rows > HEIGHT - (g->ndevs - 1 - i))
				rows = HEIGHT - (g->ndevs - 1 - i) - y;
		}
		g->devs[i].y_count = rows;
		y += rows;
		i++;
	}
}

static int	init_one_dev(t_gpu_dev *d, const char *src, size_t slen)
{
	cl_int	err;
	char	logbuf[4096];
	size_t	logn;

	d->ctx = clCreateContext(NULL, 1, &d->dev, NULL, NULL, &err);
	if (err != CL_SUCCESS)
		return (0);
	d->queue = clCreateCommandQueue(d->ctx, d->dev, 0, &err);
	if (err != CL_SUCCESS)
		return (0);
	d->prog = clCreateProgramWithSource(d->ctx, 1, &src, &slen, &err);
	if (err != CL_SUCCESS)
		return (0);
	err = clBuildProgram(d->prog, 1, &d->dev, "-cl-std=CL1.2", NULL, NULL);
	if (err != CL_SUCCESS)
	{
		logn = 0;
		clGetProgramBuildInfo(d->prog, d->dev, CL_PROGRAM_BUILD_LOG,
			sizeof(logbuf) - 1, logbuf, &logn);
		logbuf[logn] = 0;
		printf("[rt] OpenCL build failed on %s:\n%s\n", d->name, logbuf);
		return (0);
	}
	d->kern = clCreateKernel(d->prog, "trace_frame", &err);
	return (err == CL_SUCCESS);
}

int	gpu_init(t_vars *vars)
{
	t_gpu	*g;
	char	*src;
	size_t	slen;
	int		i;
	int		good;

	vars->gpu = NULL;
	vars->gpu_ok = 0;
	if (!vars->use_gpu)
		return (0);
	g = ft_calloc(1, sizeof(t_gpu));
	if (!g)
		return (0);
	collect_gpu_devices(g);
	if (g->ndevs == 0)
		return (free(g),
			printf("[rt] OpenCL: no GPU found (check OCL_ICD_VENDORS / RUSTICL_ENABLE)\n"),
			0);
	src = load_file("gpu/kernels/trace.cl", &slen);
	if (!src)
		return (free(g), printf("[rt] OpenCL: missing gpu/kernels/trace.cl\n"), 0);
	good = 0;
	i = 0;
	while (i < g->ndevs)
	{
		if (init_one_dev(&g->devs[i], src, slen))
		{
			if (good != i)
				g->devs[good] = g->devs[i];
			good++;
		}
		i++;
	}
	free(src);
	g->ndevs = good;
	if (g->ndevs == 0)
		return (free(g), 0);
	assign_row_bands(g);
	g->fb = calloc(WIDTH * HEIGHT * 3, sizeof(float));
	if (!g->fb)
		return (gpu_shutdown(vars), free(g), 0);
	g->ok = 1;
	vars->gpu = g;
	vars->gpu_ok = 1;
	printf("[rt] OpenCL multi-GPU ready (%d device%s):\n",
		g->ndevs, g->ndevs > 1 ? "s" : "");
	i = 0;
	while (i < g->ndevs)
	{
		printf("  - GPU #%d: %s (%u CU) -> rows [%d..%d)\n",
			i, g->devs[i].name, g->devs[i].cu,
			g->devs[i].y_start, g->devs[i].y_start + g->devs[i].y_count);
		i++;
	}
	fflush(stdout);
	return (1);
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
	{
		if (g->devs[i].b_out)
			clReleaseMemObject(g->devs[i].b_out);
		if (g->devs[i].b_cam)
			clReleaseMemObject(g->devs[i].b_cam);
		if (g->devs[i].b_objs)
			clReleaseMemObject(g->devs[i].b_objs);
		if (g->devs[i].b_lights)
			clReleaseMemObject(g->devs[i].b_lights);
		if (g->devs[i].kern)
			clReleaseKernel(g->devs[i].kern);
		if (g->devs[i].prog)
			clReleaseProgram(g->devs[i].prog);
		if (g->devs[i].queue)
			clReleaseCommandQueue(g->devs[i].queue);
		if (g->devs[i].ctx)
			clReleaseContext(g->devs[i].ctx);
		i++;
	}
	free(g->fb);
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
	o->shininess = e->has_material ? e->shininess : 0.0f;
	o->reflectivity = e->reflectivity;
	if (e->type == SPHERE)
		o->radius = e->scale.x;
}

static int	upload_scene_once(t_gpu *g, t_vars *vars)
{
	t_gpu_obj	objs[128];
	t_gpu_light	lights[32];
	t_gpu_cam	cam;
	cl_int		err;
	int			i;

	if (g->scene_uploaded)
		return (1);
	g->nobj = vars->pos > 128 ? 128 : vars->pos;
	g->nlight = vars->pos1 > 32 ? 32 : vars->pos1;
	i = -1;
	while (++i < g->nobj)
		pack_obj(&objs[i], &vars->objects[i]);
	i = -1;
	while (++i < g->nlight)
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
	i = 0;
	while (i < g->ndevs)
	{
		t_gpu_dev *d = &g->devs[i];
		d->b_out = clCreateBuffer(d->ctx, CL_MEM_WRITE_ONLY,
				sizeof(float) * WIDTH * d->y_count * 3, NULL, &err);
		d->b_objs = clCreateBuffer(d->ctx,
				CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				sizeof(t_gpu_obj) * (g->nobj ? g->nobj : 1), objs, &err);
		d->b_lights = clCreateBuffer(d->ctx,
				CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				sizeof(t_gpu_light) * (g->nlight ? g->nlight : 1), lights, &err);
		d->b_cam = clCreateBuffer(d->ctx,
				CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				sizeof(t_gpu_cam), &cam, &err);
		if (!d->b_out || !d->b_objs || !d->b_lights || !d->b_cam)
			return (0);
		i++;
	}
	g->scene_uploaded = 1;
	return (1);
}

int	gpu_raytrace(t_vars *vars)
{
	t_gpu	*g;
	float	amb[4];
	int		w;
	int		h;
	int		i;
	cl_int	err;

	g = (t_gpu *)vars->gpu;
	if (!g || !g->ok || !upload_scene_once(g, vars))
		return (0);
	vars->frames++;
	w = WIDTH;
	h = HEIGHT;
	amb[0] = vars->ambient ? vars->ambient->color.x : 0.0f;
	amb[1] = vars->ambient ? vars->ambient->color.y : 0.0f;
	amb[2] = vars->ambient ? vars->ambient->color.z : 0.0f;
	amb[3] = 0.0f;
	i = 0;
	while (i < g->ndevs)
	{
		t_gpu_dev	*d = &g->devs[i];
		size_t		global = (size_t)WIDTH * (size_t)d->y_count;

		clSetKernelArg(d->kern, 0, sizeof(cl_mem), &d->b_out);
		clSetKernelArg(d->kern, 1, sizeof(cl_mem), &d->b_cam);
		clSetKernelArg(d->kern, 2, sizeof(cl_mem), &d->b_objs);
		clSetKernelArg(d->kern, 3, sizeof(int), &g->nobj);
		clSetKernelArg(d->kern, 4, sizeof(cl_mem), &d->b_lights);
		clSetKernelArg(d->kern, 5, sizeof(int), &g->nlight);
		clSetKernelArg(d->kern, 6, sizeof(cl_float3), amb);
		clSetKernelArg(d->kern, 7, sizeof(int), &w);
		clSetKernelArg(d->kern, 8, sizeof(int), &h);
		clSetKernelArg(d->kern, 9, sizeof(int), &d->y_start);
		clSetKernelArg(d->kern, 10, sizeof(int), &d->y_count);
		clSetKernelArg(d->kern, 11, sizeof(int), &vars->frames);
		err = clEnqueueNDRangeKernel(d->queue, d->kern, 1, NULL, &global,
				NULL, 0, NULL, NULL);
		if (err != CL_SUCCESS)
			return (0);
		clFlush(d->queue);
		i++;
	}
	i = 0;
	while (i < g->ndevs)
	{
		t_gpu_dev	*d = &g->devs[i];
		size_t		bytes = sizeof(float) * WIDTH * d->y_count * 3;
		float		*dst = g->fb + ((size_t)d->y_start * WIDTH * 3);

		err = clEnqueueReadBuffer(d->queue, d->b_out, CL_TRUE, 0,
				bytes, dst, 0, NULL, NULL);
		if (err != CL_SUCCESS)
			return (0);
		i++;
	}
	i = -1;
	while (++i < WIDTH * HEIGHT)
	{
		t_vector	c;

		c = new_vec(g->fb[i * 3], g->fb[i * 3 + 1], g->fb[i * 3 + 2]);
		vars->buffer_img[i] = add_vec(vars->buffer_img[i], c);
		c = scale_vec(vars->buffer_img[i], 1.0f / vars->frames);
		set_pixel(vars->image, i % WIDTH, i / WIDTH, c);
	}
	return (1);
}
