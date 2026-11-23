/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export_ppm.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 01:54:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:54:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"

static unsigned char	clamp_byte(float v)
{
	if (v < 0.0f)
		v = 0.0f;
	if (v > 1.0f)
		v = 1.0f;
	return ((unsigned char)(v * 255.0f));
}

static void	write_ppm_rows(FILE *f, t_image *img)
{
	unsigned char	row[WIDTH * 3];
	int				y;
	int				x;

	y = 0;
	while (y < HEIGHT)
	{
		x = 0;
		while (x < WIDTH)
		{
			row[x * 3] = clamp_byte(img->red[y][x]);
			row[x * 3 + 1] = clamp_byte(img->green[y][x]);
			row[x * 3 + 2] = clamp_byte(img->blue[y][x]);
			x++;
		}
		fwrite(row, 1, WIDTH * 3, f);
		y++;
	}
}

void	export_ppm(t_vars *vars, const char *path)
{
	FILE	*f;

	if (!vars || !vars->image || !path)
		return ;
	f = fopen(path, "wb");
	if (!f)
	{
		printf("[rt] Failed to open %s\n", path);
		fflush(stdout);
		return ;
	}
	fprintf(f, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
	write_ppm_rows(f, vars->image);
	fclose(f);
	printf("[rt] Saved %s\n", path);
	fflush(stdout);
}
