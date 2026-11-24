/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_init.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cl_internal.h"

char	*cl_load_file(const char *path, size_t *out_len)
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

/* NEO reports Intel EUs. UHD 770 is 32 EUs; /16 keeps the old 2-vs-16 split. */
static void	try_add_device(t_gpu *g, cl_device_id dev)
{
	char	name[128];
	cl_uint	vid;
	cl_uint	cu;
	int		k;

	name[0] = 0;
	vid = 0;
	cu = 1;
	clGetDeviceInfo(dev, CL_DEVICE_NAME, sizeof(name), name, NULL);
	clGetDeviceInfo(dev, CL_DEVICE_VENDOR_ID, sizeof(vid), &vid, NULL);
	clGetDeviceInfo(dev, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cu), &cu, NULL);
	k = -1;
	while (++k < g->ndevs)
		if (g->devs[k].vendor_id == vid && strcmp(g->devs[k].name, name) == 0)
			return ;
	if (vid == 0x8086 && cu > 16)
		cu = cu / 16;
	if (cu == 0)
		cu = 1;
	g->devs[g->ndevs].dev = dev;
	g->devs[g->ndevs].vendor_id = vid;
	g->devs[g->ndevs].cu = cu;
	snprintf(g->devs[g->ndevs].name, sizeof(g->devs[g->ndevs].name),
		"%s", name);
	g->ndevs++;
}

void	cl_collect_devices(t_gpu *g)
{
	cl_platform_id	plats[8];
	cl_device_id	devs[8];
	cl_uint			cnt[2];
	cl_uint			i;
	cl_uint			j;

	g->ndevs = 0;
	if (clGetPlatformIDs(8, plats, &cnt[0]) != CL_SUCCESS || cnt[0] == 0)
		return ;
	i = 0;
	while (i < cnt[0] && g->ndevs < MAX_GPUS)
	{
		if (clGetDeviceIDs(plats[i], CL_DEVICE_TYPE_GPU, 8, devs, &cnt[1])
			== CL_SUCCESS)
		{
			j = 0;
			while (j < cnt[1] && g->ndevs < MAX_GPUS)
				try_add_device(g, devs[j++]);
		}
		i++;
	}
}

void	cl_assign_bands(t_gpu *g)
{
	unsigned int	total_cu;
	int				i;
	int				y;
	int				rows;

	total_cu = 0;
	i = -1;
	while (++i < g->ndevs)
		total_cu += g->devs[i].cu;
	if (total_cu == 0)
		total_cu = (unsigned int)g->ndevs;
	y = 0;
	i = -1;
	while (++i < g->ndevs)
	{
		g->devs[i].y_start = y;
		rows = (int)(((long)HEIGHT * g->devs[i].cu) / total_cu);
		if (rows < 1)
			rows = 1;
		if (i == g->ndevs - 1 || y + rows > HEIGHT - (g->ndevs - 1 - i))
			rows = HEIGHT - (g->ndevs - 1 - i) - y;
		g->devs[i].y_count = rows;
		y += rows;
	}
}

int	cl_init_device(t_gpu_dev *d, const char *src, size_t slen)
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
