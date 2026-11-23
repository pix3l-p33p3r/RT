/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   surface.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:55:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:55:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

void	object_uv(t_info *info, float *u, float *v)
{
	t_vector	p;
	int			type;

	type = info->e->type;
	if (type == SPHERE || type == HALF_SPHERE)
	{
		p = normalized(sub_vec(info->hitpoint, info->e->translation));
		*u = 0.5f + atan2f(p.y, p.x) / (2.0f * (float)M_PI);
		*v = 0.5f + asinf(fmaxf(-1.0f, fminf(1.0f, p.z))) / (float)M_PI;
		return ;
	}
	if (type == CYLINDER || type == CONE)
	{
		p = apply_transform_vector(&info->hitpoint, info->e->gtfm, BACKWARD);
		*u = 0.5f + atan2f(p.y, p.x) / (2.0f * (float)M_PI);
		*v = (p.z + 1.0f) * 0.5f;
		*v = *v - floorf(*v);
		return ;
	}
	*u = info->hitpoint.x * 0.125f;
	*v = info->hitpoint.y * 0.125f;
	*u = *u - floorf(*u);
	*v = *v - floorf(*v);
}

static t_vector	checker_color(t_vector base, float u, float v)
{
	int	ix;
	int	iy;

	ix = (int)floorf(u * 8.0f);
	iy = (int)floorf(v * 8.0f);
	if (((ix + iy) & 1) == 0)
		return (base);
	if (base.x + base.y + base.z > 1.2f)
		return (scale_vec(base, 0.22f));
	return (new_vec(fminf(1.0f, base.x + 0.55f),
			fminf(1.0f, base.y + 0.55f),
			fminf(1.0f, base.z + 0.55f)));
}

static t_vector	wave_marble(t_info *info, float u, float v)
{
	t_vector	p;
	t_vector	b;
	float		t;
	float		n;

	b = info->e->base_color;
	if (info->e->pattern == PAT_STRIPE)
	{
		n = v * 8.0f + 0.25f * sinf(u * 4.0f * (float)M_PI);
		t = 0.5f + 0.5f * sinf(n * 2.0f * (float)M_PI);
		return (new_vec(fminf(1.0f, b.x * (0.25f + 0.75f * t) + 0.15f * t),
				fminf(1.0f, b.y * (0.25f + 0.75f * t) + 0.15f * t),
				fminf(1.0f, b.z * (0.25f + 0.75f * t) + 0.15f * t)));
	}
	p = info->hitpoint;
	n = sinf(p.x * 2.1f + p.y * 1.7f) * 0.5f;
	n += sinf(p.y * 4.3f - p.z * 3.9f) * 0.25f;
	n += sinf(p.z * 8.7f + p.x * 7.5f) * 0.125f;
	t = 0.5f + 0.5f * sinf(p.x * 3.0f + p.z * 2.0f + n * 4.5f);
	n = t * t * 0.25f;
	return (new_vec(fminf(1.0f, b.x * (0.22f + 0.78f * t) + n),
			fminf(1.0f, b.y * (0.22f + 0.78f * t) + n),
			fminf(1.0f, b.z * (0.22f + 0.78f * t) + n)));
}

t_vector	sample_surface_color(t_info *info)
{
	float	u;
	float	v;
	int		pat;

	pat = info->e->pattern;
	if (pat == PAT_NONE)
		return (info->e->base_color);
	object_uv(info, &u, &v);
	if (pat == PAT_CHECKER)
		return (checker_color(info->e->base_color, u, v));
	if (pat == PAT_STRIPE || pat == PAT_MARBLE)
		return (wave_marble(info, u, v));
	if (pat == PAT_PPM && info->e->tex)
		return (sample_ppm(info->e->tex, u, v));
	return (info->e->base_color);
}

void	apply_bump(t_info *info)
{
	t_vector	t;
	t_vector	b;
	float		uv[2];
	float		d[2];
	float		f;

	if (!info || !info->e || info->e->bump <= 0.0f)
		return ;
	object_uv(info, &uv[0], &uv[1]);
	if (fabsf(info->normal.z) < 0.9f)
		t = new_vec(0.0f, 0.0f, 1.0f);
	else
		t = new_vec(1.0f, 0.0f, 0.0f);
	t = normalized(cross_product(t, info->normal));
	b = cross_product(info->normal, t);
	f = 2.0f * (float)M_PI * 10.0f;
	d[0] = cosf(uv[0] * f) * cosf(uv[1] * f) * info->e->bump;
	d[1] = -sinf(uv[0] * f) * sinf(uv[1] * f) * info->e->bump;
	info->normal = add_vec(info->normal,
			add_vec(scale_vec(t, d[0]), scale_vec(b, d[1])));
	info->normal = normalized(info->normal);
}
