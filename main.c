/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/08 04:36:04 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:20:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"
#include "libft/libft.h"
#include "parsing/miniRT.h"
#include <string.h>

void	cleanup_vars(t_vars *vars)
{
	int	i;

	gpu_shutdown(vars);
	if (vars->objects)
	{
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
	free(vars->lights);
	vars->lights = NULL;
	free(vars->ambient);
	vars->ambient = NULL;
	if (vars->image)
		delete_image(vars->image);
	vars->image = NULL;
	free(vars->buffer_img);
	vars->buffer_img = NULL;
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

int	close_hook(t_vars *vars)
{
	cleanup_vars(vars);
	exit(0);
	return (0);
}

int	loop(t_vars *vars)
{
	if (vars->max_frames > 0 && vars->frames >= vars->max_frames)
	{
		if (getenv("RT_MAX_FRAMES"))
			close_hook(vars);
		return (0);
	}
	raytrace(vars);
	render(vars);
	return (0);
}

int	key_hook(int keycode, t_vars *vars)
{
	if (keycode == 65307)
		close_hook(vars);
	return (keycode);
}

int	expose_hook(t_vars *vars)
{
	blit_frame(vars);
	return (0);
}

static int	read_max_frames(void)
{
	char	*env;
	int		n;

	env = getenv("RT_MAX_FRAMES");
	if (!env)
		return (1337);
	n = atoi(env);
	if (n < 1)
		return (1);
	return (n);
}

int	main(int ac, char **av)
{
	t_vars	vars;

	vars = (t_vars){0};
	if (ac != 2 || check_extention(av[1]) == 1)
		return (1);
	vars.use_gpu = (getenv("RT_USE_GPU")
			&& strcmp(getenv("RT_USE_GPU"), "1") == 0);
	vars.max_frames = read_max_frames();
	vars.pos = 0;
	vars.len = 100;
	vars.objects = ft_calloc(vars.len + 1, sizeof(t_object));
	vars.pos1 = 0;
	vars.len1 = 100;
	vars.lights = ft_calloc(vars.len1 + 1, sizeof(t_light));
	check_element(av[1]);
	read_scenes(av[1], &vars);
	vars.image = new_image();
	vars.buffer_img = calloc(WIDTH * HEIGHT, sizeof(t_vector));
	vars.mlx = mlx_init();
	vars.win = mlx_new_window(vars.mlx, WIDTH, HEIGHT, "rt");
	vars.mlx_img = new_mlx_img(&vars);
	vars.frames = 0;
	if (!gpu_init(&vars))
	{
		printf("[rt] CPU pthread renderer ready (%d threads)\n",
			rt_thread_count());
		fflush(stdout);
	}
	mlx_loop_hook(vars.mlx, loop, &vars);
	mlx_hook(vars.win, 2, 1L << 0, key_hook, &vars);
	mlx_hook(vars.win, 12, 1L << 15, expose_hook, &vars);
	mlx_hook(vars.win, 17, 1L << 0, close_hook, &vars);
	mlx_loop(vars.mlx);
	cleanup_vars(&vars);
	return (0);
}
