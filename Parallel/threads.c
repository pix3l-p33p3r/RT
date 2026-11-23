/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:46:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:28:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"
#include <pthread.h>
#include <unistd.h>

static void	*row_worker(void *p)
{
	raytrace_rows((t_thr_arg *)p);
	return (NULL);
}

int	rt_thread_count(void)
{
	char	*env;
	long	n;

	env = getenv("RT_THREADS");
	if (env && *env)
	{
		n = atoi(env);
		if (n >= 1 && n <= 64)
			return ((int)n);
	}
	n = sysconf(_SC_NPROCESSORS_ONLN);
	if (n < 1)
		return (1);
	if (n > 64)
		return (64);
	return ((int)n);
}

static void	init_band(t_thr_arg *arg, t_vars *vars, int i, int chunk)
{
	arg->vars = vars;
	arg->y0 = i * chunk;
	arg->y1 = arg->y0 + chunk;
	if (arg->y1 > HEIGHT)
		arg->y1 = HEIGHT;
	arg->xfact = 1.0f / ((float)WIDTH / 2.0f);
	arg->yfact = 1.0f / ((float)HEIGHT / 2.0f);
	arg->rng = (unsigned int)(vars->frames * 747796405u
			+ (unsigned int)i * 2891336453u);
}

void	threaded_raytrace(t_vars *vars)
{
	int			n;
	int			i;
	int			chunk;
	pthread_t	ids[64];
	t_thr_arg	args[64];

	vars->frames++;
	n = rt_thread_count();
	chunk = (HEIGHT + n - 1) / n;
	i = -1;
	while (++i < n)
	{
		init_band(&args[i], vars, i, chunk);
		if (args[i].y0 >= HEIGHT
			|| pthread_create(&ids[i], NULL, row_worker, &args[i]) != 0)
			ids[i] = 0;
	}
	i = -1;
	while (++i < n)
		if (ids[i])
			pthread_join(ids[i], NULL);
}
