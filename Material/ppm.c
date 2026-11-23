/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ppm.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:55:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:55:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

void	free_ppm(t_ppm *ppm)
{
	if (!ppm)
		return ;
	free(ppm->rgb);
	free(ppm);
}

static int	read_ppm_int(FILE *fp, int *out)
{
	int	c;
	int	val;

	c = fgetc(fp);
	while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '#')
	{
		if (c == '#')
		{
			while (c != '\n' && c != EOF)
				c = fgetc(fp);
		}
		c = fgetc(fp);
	}
	if (c < '0' || c > '9')
		return (0);
	val = 0;
	while (c >= '0' && c <= '9')
	{
		val = val * 10 + (c - '0');
		c = fgetc(fp);
	}
	if (c == '#')
		ungetc(c, fp);
	*out = val;
	return (1);
}

static t_ppm	*read_ppm_body(FILE *fp, int *h)
{
	t_ppm	*ppm;
	size_t	n;
	size_t	i;
	int		v;

	ppm = ft_calloc(1, sizeof(t_ppm));
	n = (size_t)h[2] * (size_t)h[3] * 3;
	if (!ppm)
		return (NULL);
	ppm->w = h[2];
	ppm->h = h[3];
	ppm->rgb = malloc(n);
	if (ppm->rgb && h[1] == '6' && fread(ppm->rgb, 1, n, fp) == n)
		return (ppm);
	i = 0;
	while (ppm->rgb && h[1] == '3' && i < n && read_ppm_int(fp, &v))
	{
		if (v > h[0])
			v = h[0];
		ppm->rgb[i++] = (unsigned char)((v * 255) / h[0]);
	}
	if (ppm->rgb && h[1] == '3' && i == n)
		return (ppm);
	free_ppm(ppm);
	return (NULL);
}

t_ppm	*load_ppm(const char *path)
{
	FILE	*fp;
	t_ppm	*ppm;
	int		h[4];

	fp = fopen(path, "rb");
	if (!fp)
		return (NULL);
	h[0] = fgetc(fp);
	h[1] = fgetc(fp);
	ppm = NULL;
	if (h[0] == 'P' && (h[1] == '6' || h[1] == '3')
		&& read_ppm_int(fp, &h[2]) && read_ppm_int(fp, &h[3])
		&& read_ppm_int(fp, &h[0]) && h[2] > 0 && h[3] > 0
		&& h[2] <= 8192 && h[3] <= 8192 && h[0] > 0 && h[0] <= 255)
		ppm = read_ppm_body(fp, h);
	fclose(fp);
	return (ppm);
}

t_vector	sample_ppm(t_ppm *ppm, float u, float v)
{
	int		x;
	int		y;
	size_t	idx;

	if (!ppm || !ppm->rgb || ppm->w <= 0 || ppm->h <= 0)
		return (new_vec(1.0f, 0.0f, 1.0f));
	u = u - floorf(u);
	v = v - floorf(v);
	x = (int)(u * (float)ppm->w);
	y = (int)(v * (float)ppm->h);
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x >= ppm->w)
		x = ppm->w - 1;
	if (y >= ppm->h)
		y = ppm->h - 1;
	idx = ((size_t)y * (size_t)ppm->w + (size_t)x) * 3;
	return (new_vec((float)ppm->rgb[idx] / 255.0f,
			(float)ppm->rgb[idx + 1] / 255.0f,
			(float)ppm->rgb[idx + 2] / 255.0f));
}
