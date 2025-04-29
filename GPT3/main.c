#include "minishell.h"

extern volatile sig_atomic_t    g_exit_status;

/* Free a duplicated environment */
static void ft_envpfree(char **envp)
{
	int i;

	i = 0;
	if (!envp)
		return;
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

int main(int argc, char **argv, char **envp)
{
	char *line;
	(void)argv;
	if (argc != 1) {
		fprintf(stderr, "minishell: no arguments supported\n");
		return (EXIT_FAILURE);
	}
	char **env = ft_dupenvp(envp);
	if (!env) {
		perror("minishell");
		exit(EXIT_FAILURE);
	}
	setup_signals();
	while (1)
	{
		line = readline("minishell$ ");
		if (!line)
		{
			write(1, "exit\n", 5);
			free(line);
			break;
		}
		if (*line)
			add_history(line);
		t_segment *segs = parse_input(line, &env);
		if (segs) {
			execute_segments(segs, &env);
			free_segments(segs);
		}
		rl_on_new_line();
		free(line);
	}
	rl_clear_history();
	ft_envpfree(env);
	signals_print_handler(1);
	return (g_exit_status);
}
