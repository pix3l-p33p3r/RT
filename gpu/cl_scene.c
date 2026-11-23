/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cl_scene.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "cl_internal.h"

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
	o->shininess = 0.0f;
	if (e->has_material)
		o->shininess = e->shininess;
	o->reflectivity = e->reflectivity;
	if (e->type == SPHERE)
		o->radius = e->scale.x;
}

static void	pack_scene(t_gpu *g, t_vars *vars, t_gpu_scene *sc)
{
	int	i;

	g->nobj = vars->pos;
	if (g->nobj > 128)
		g->nobj = 128;
	g->nlight = vars->pos1;
	if (g->nlight > 32)
		g->nlight = 32;
	i = -1;
	while (++i < g->nobj)
		pack_obj(&sc->objs[i], &vars->objects[i]);
	i = -1;
	while (++i < g->nlight)
	{
		sc->lights[i].pos[0] = vars->lights[i].position.x;
		sc->lights[i].pos[1] = vars->lights[i].position.y;
		sc->lights[i].pos[2] = vars->lights[i].position.z;
		sc->lights[i].color[0] = vars->lights[i].color.x;
		sc->lights[i].color[1] = vars->lights[i].color.y;
		sc->lights[i].color[2] = vars->lights[i].color.z;
		sc->lights[i].intensity = vars->lights[i].intensity;
	}
	pack_cam(&sc->cam, &vars->camera);
}

static int	upload_to_dev(t_gpu *g, t_gpu_dev *d, t_gpu_scene *sc)
{
	cl_int	err;
	int		no;
	int		nl;

	no = g->nobj;
	if (no == 0)
		no = 1;
	nl = g->nlight;
	if (nl == 0)
		nl = 1;
	d->b_out = clCreateBuffer(d->ctx, CL_MEM_WRITE_ONLY,
			sizeof(float) * WIDTH * d->y_count * 3, NULL, &err);
	d->b_objs = clCreateBuffer(d->ctx,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(t_gpu_obj) * no, sc->objs, &err);
	d->b_lights = clCreateBuffer(d->ctx,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(t_gpu_light) * nl, sc->lights, &err);
	d->b_cam = clCreateBuffer(d->ctx,
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(t_gpu_cam), &sc->cam, &err);
	return (d->b_out && d->b_objs && d->b_lights && d->b_cam);
}

int	cl_upload_scene(t_gpu *g, t_vars *vars)
{
	t_gpu_scene	sc;
	int			i;

	if (g->scene_uploaded)
		return (1);
	ft_bzero(&sc, sizeof(sc));
	pack_scene(g, vars, &sc);
	i = 0;
	while (i < g->ndevs)
	{
		if (!upload_to_dev(g, &g->devs[i], &sc))
			return (0);
		i++;
	}
	g->scene_uploaded = 1;
	return (1);
}
