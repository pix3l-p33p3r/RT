/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   refract.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:47:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:47:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

t_vector	glass_tint(t_vector base_color)
{
	return (new_vec(0.35f + 0.65f * base_color.x,
			0.35f + 0.65f * base_color.y,
			0.35f + 0.65f * base_color.z));
}

int	refract_vec(t_vector d, t_vector n, float ior, t_vector *out)
{
	float		cosi;
	float		eta;
	float		k;
	t_vector	nn;

	cosi = dot_product(d, n);
	eta = 1.0f / ior;
	nn = n;
	if (cosi < 0.0f)
		cosi = -cosi;
	else
	{
		eta = ior;
		nn = scale_vec(n, -1.0f);
	}
	k = 1.0f - eta * eta * (1.0f - cosi * cosi);
	if (k < 0.0f)
		return (0);
	*out = normalized(add_vec(scale_vec(d, eta),
				scale_vec(nn, eta * cosi - sqrtf(k))));
	return (1);
}

t_ray	build_refract_ray(t_info *info, t_ray *in_ray)
{
	t_vector	dir;
	t_vector	refr;
	t_vector	orig;
	t_vector	nn;

	dir = normalized(in_ray->direction);
	nn = info->normal;
	if (dot_product(dir, nn) > 0.0f)
		nn = scale_vec(nn, -1.0f);
	if (!refract_vec(dir, info->normal, info->e->ior, &refr))
	{
		refr = reflect(dir, nn);
		orig = add_vec(info->hitpoint, scale_vec(nn, 0.001f));
		return (new_ray(orig, add_vec(orig, refr)));
	}
	orig = sub_vec(info->hitpoint, scale_vec(nn, 0.001f));
	return (new_ray(orig, add_vec(orig, refr)));
}

static t_vector	mul_vec(t_vector a, t_vector b)
{
	return (new_vec(a.x * b.x, a.y * b.y, a.z * b.z));
}

t_vector	shadow_atten(t_light *light, t_object *objects, t_info *info)
{
	t_norm4		norm;
	t_info		test;
	t_vector	atten;
	int			i;

	norm.lighdir = normalized(sub_vec(light->position, info->hitpoint));
	norm.lighdist = length(sub_vec(light->position, info->hitpoint));
	norm.light_ray = new_ray(add_vec(info->hitpoint,
				scale_vec(norm.lighdir, 0.001f)), light->position);
	atten = new_vec(1.0f, 1.0f, 1.0f);
	i = -1;
	while (objects[++i].gtfm)
	{
		test.e = &objects[i];
		if (info->e == &objects[i] || !objects[i].hit(&norm.light_ray, &test)
			|| length(sub_vec(test.hitpoint, norm.light_ray.point1))
			>= norm.lighdist)
			continue ;
		if (objects[i].transp <= 0.0f)
			return (new_vec(0.0f, 0.0f, 0.0f));
		atten = mul_vec(atten, scale_vec(glass_tint(objects[i].base_color),
					objects[i].transp));
	}
	return (atten);
}
