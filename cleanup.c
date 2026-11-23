/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cleanup.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/23 00:28:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"

static void	free_objects(t_vars *vars)
{
	int	i;

	if (!vars->objects)
		return ;
	i = 0;
	while (i < vars->pos)
	{
		if (vars->objects[i].gtfm)
		{
			delete_matrix(vars->objects[i].gtfm[0]);
			delete_matrix(vars->objects[i].gtfm[1]);
			free(vars->objects[i].gtfm);
		}
		i++;
	}
	free(vars->objects);
	vars->objects = NULL;
}

static void	free_mlx(t_vars *vars)
{
	if (vars->mlx && vars->mlx_img)
	{
		if (vars->mlx_img->img)
			mlx_destroy_image(vars->mlx, vars->mlx_img->img);
		free(vars->mlx_img);
		vars->mlx_img = NULL;
	}
	if (vars->mlx && vars->win)
		mlx_destroy_window(vars->mlx, vars->win);
	if (vars->mlx)
	{
		mlx_destroy_display(vars->mlx);
		free(vars->mlx);
		vars->mlx = NULL;
	}
}

void	cleanup_vars(t_vars *vars)
{
	gpu_shutdown(vars);
	free_objects(vars);
	free(vars->lights);
	vars->lights = NULL;
	free(vars->ambient);
	vars->ambient = NULL;
	if (vars->image)
		delete_image(vars->image);
	vars->image = NULL;
	free(vars->buffer_img);
	vars->buffer_img = NULL;
	free_mlx(vars);
}

int	close_hook(t_vars *vars)
{
	cleanup_vars(vars);
	exit(0);
	return (0);
}
