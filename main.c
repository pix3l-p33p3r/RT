/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/08 04:36:04 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 01:58:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "main.h"
#include "libft/libft.h"
#include "parsing/miniRT.h"
#include <string.h>

int	loop(t_vars *vars)
{
	char	*out;

	if (vars->max_frames > 0 && vars->frames >= vars->max_frames)
	{
		out = getenv("RT_EXPORT_PATH");
		if (out && *out)
			export_ppm(vars, out);
		if (getenv("RT_MAX_FRAMES"))
			close_hook(vars);
		return (0);
	}
	raytrace(vars);
	render(vars);
	return (0);
}

int	expose_hook(t_vars *vars)
{
	blit_frame(vars);
	return (0);
}

static void	init_scene_and_mlx(t_vars *vars, char *path)
{
	char	*env;

	vars->use_gpu = (getenv("RT_USE_GPU")
			&& strcmp(getenv("RT_USE_GPU"), "1") == 0);
	env = getenv("RT_STEREO");
	if (env && atoi(env) >= 0 && atoi(env) <= 2)
		vars->stereo_mode = atoi(env);
	env = getenv("RT_MAX_FRAMES");
	vars->max_frames = 1337;
	if (env && atoi(env) > 0)
		vars->max_frames = atoi(env);
	vars->len = 100;
	vars->objects = ft_calloc(vars->len + 1, sizeof(t_object));
	vars->len1 = 100;
	vars->lights = ft_calloc(vars->len1 + 1, sizeof(t_light));
	check_element(path);
	read_scenes(path, vars);
	vars->image = new_image();
	vars->buffer_img = calloc(WIDTH * HEIGHT, sizeof(t_vector));
	vars->mlx = mlx_init();
	vars->win = mlx_new_window(vars->mlx, WIDTH, HEIGHT, "rt");
	vars->mlx_img = new_mlx_img(vars);
}

int	main(int ac, char **av)
{
	t_vars	vars;

	vars = (t_vars){0};
	if (ac != 2 || check_extention(av[1]) == 1)
		return (1);
	init_scene_and_mlx(&vars, av[1]);
	if (!gpu_init(&vars))
	{
		printf("[rt] CPU pthread renderer ready (%d threads)\n",
			rt_thread_count());
		fflush(stdout);
	}
	mlx_loop_hook(vars.mlx, loop, &vars);
	mlx_hook(vars.win, 2, 1L << 0, handle_keypress, &vars);
	mlx_hook(vars.win, 12, 1L << 15, expose_hook, &vars);
	mlx_hook(vars.win, 17, 1L << 0, close_hook, &vars);
	mlx_loop(vars.mlx);
	cleanup_vars(&vars);
	return (0);
}
