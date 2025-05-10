#include "minishell.h"

extern volatile sig_atomic_t	g_exit_status;

/* Free a duplicated environment */
void	ft_envpfree(char **envp)
{
	int	i;

	i = 0;
	if (!envp)
		return ;
	while (envp[i])
	{
		free(envp[i]);
		i++;
	}
	free(envp);
}

/* Duplicate the environment array */
char	**ft_dupenvp(char **envp)
{
	size_t	count;
	size_t	i;
	char	**copy;

	count = 0;
	while (envp && envp[count])
		count++;
	copy = malloc(sizeof(char *) * (count + 1));
	if (!copy)
		return (NULL);
	i = 0;
	while (i < count)
	{
		copy[i] = ft_strdup(envp[i]);
		if (!copy[i])
			return (ft_envpfree(copy), NULL);
		i++;
	}
	copy[count] = NULL;
	return (copy);
}

static void	ft_mainloop(char **env)
{
	char		*line;
	t_segment	*segs;

	while (1)
	{
		line = readline("minishell$ ");
		if (!line)
		{
			write(1, "exit\n", 5);
			free(line);
			break ;
		}
		if (*line)
			add_history(line);
		segs = parse_input(line, &env);
		if (segs)
		{
			if (ft_execute(segs) == -1)
				break ;
			free_segments(segs);
		}
		rl_on_new_line();
		free(line);
		g_exit_status = g_exit_status & 0xFF;
	}
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
	setup_signals();
	ft_mainloop(env);
	g_exit_status = g_exit_status & 0xFF;
	rl_clear_history();
	ft_envpfree(env);
	signals_print_handler(1);
	return (g_exit_status);
}
