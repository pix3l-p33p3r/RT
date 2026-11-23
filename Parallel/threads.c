/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elel-yak <elel-yak@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/22 23:46:00 by elel-yak          #+#    #+#             */
/*   Updated: 2026/11/23 00:20:00 by elel-yak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../main.h"
#include <pthread.h>
#include <unistd.h>

typedef struct s_thr_arg
{
	t_vars			*vars;
	int				y0;
	int				y1;
	unsigned int	rng;
	float			xfact;
	float			yfact;
}	t_thr_arg;

static void	*row_worker(void *p)
{
	t_thr_arg	*a;

	a = (t_thr_arg *)p;
	raytrace_rows(a->vars, a->y0, a->y1, a->rng, a->xfact, a->yfact);
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

void	threaded_raytrace(t_vars *vars)
{
	int			n;
	int			i;
	int			chunk;
	float		xfact;
	float		yfact;
	pthread_t	ids[64];
	t_thr_arg	args[64];

	vars->frames++;
	xfact = 1.0f / ((float)WIDTH / 2.0f);
	yfact = 1.0f / ((float)HEIGHT / 2.0f);
	n = rt_thread_count();
	chunk = (HEIGHT + n - 1) / n;
	i = -1;
	while (++i < n)
	{
		args[i].vars = vars;
		args[i].y0 = i * chunk;
		args[i].y1 = args[i].y0 + chunk;
		if (args[i].y1 > HEIGHT)
			args[i].y1 = HEIGHT;
		args[i].xfact = xfact;
		args[i].yfact = yfact;
		args[i].rng = (unsigned int)(vars->frames * 747796405u + (unsigned)i
				* 2891336453u);
		if (args[i].y0 >= HEIGHT
			|| pthread_create(&ids[i], NULL, row_worker, &args[i]) != 0)
		{
			args[i].y0 = HEIGHT;
			ids[i] = 0;
		}
	}
	i = -1;
	while (++i < n)
	{
		if (ids[i])
			pthread_join(ids[i], NULL);
	}
}
