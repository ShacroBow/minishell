/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:03:09 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:21:56 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell_bonus.h"

extern volatile sig_atomic_t	g_exit_status;

static char	*dup_and_shift(char **st, char *nl)
{
	char	*line;

	*nl = '\0';
	line = ft_strdup(*st);
	if (!line)
		return (NULL);
	ft_memmove(*st, nl + 1, ft_strlen(nl + 1) + 1);
	return (line);
}

static char	*ft_readline(const char *prompt)
{
	static char	*stash;
	char		*nl;
	char		*line;

	while (!stash || !*stash)
	{
		free(stash);
		stash = readline(prompt);
		if (!stash)
			return (NULL);
	}
	nl = ft_strchr(stash, '\n');
	if (nl)
		return (dup_and_shift(&stash, nl));
	line = stash;
	stash = NULL;
	return (line);
}

static void	ft_mainloop(char ***env)
{
	char		*line;
	t_segment	*segs;

	while (1)
	{
		line = ft_readline("minishell$ ");
		if (!line)
		{
			write(1, "exit\n", 5);
			break ;
		}
		if (*line)
			add_history(line);
		segs = ft_parse_input(line, env);
		if (segs && ft_execute(segs) == -1)
			break ;
		ft_free_segments(segs);
		rl_on_new_line();
		free(line);
		line = NULL;
		g_exit_status = g_exit_status & 0xFF;
	}
	free(line);
}

int	main(int argc, char **argv, char **envp)
{
	char		**env;

	(void)argv;
	g_exit_status = 0;
	if (argc != 1)
	{
		ft_putstr_fd("minishell: no arguments supported\n", STDERR_FILENO);
		return (EXIT_FAILURE);
	}
	env = ft_dupenvp(envp);
	if (!env)
	{
		ft_putstr_fd("minishell: env memory alloc failed\n", STDERR_FILENO);
		exit(EXIT_FAILURE);
	}
	ft_signal_setup();
	ft_mainloop(&env);
	g_exit_status = g_exit_status & 0xFF;
	rl_clear_history();
	ft_envpfree(env);
	ft_signals_print_handler(1);
	return (g_exit_status);
}
