#include "minishell.h"

extern volatile sig_atomic_t    g_exit_status;

/* Free a duplicated environment */
static void ft_envpfree(char **envp) {
	if (!envp) return;
	for (int i = 0; envp[i]; ++i)
		free(envp[i]);
	free(envp);
}

/* Duplicate the environment array */
char **dup_envp(char **envp) {
	int len = 0;
	while (envp && envp[len])
		len++;
	char **copy = calloc(len + 1, sizeof *copy);
	if (!copy)
		exit(EXIT_FAILURE);
	for (int i = 0; i < len; ++i) {
		copy[i] = strdup(envp[i]);
		if (!copy[i]) {
			while (--i >= 0) free(copy[i]);
			free(copy);
			exit(EXIT_FAILURE);
		}
	}
	return copy;
}

int main(int argc, char **argv, char **envp) {
	(void)argv;
	if (argc != 1) {
		fprintf(stderr, "minishell: no arguments supported\n");
		return (EXIT_FAILURE);
	}
	char **env = dup_envp(envp);
	if (!env) {
		perror("minishell");
		exit(EXIT_FAILURE);
	}
	setup_signals();
	while (1) {
		char *line = readline("minishell$ ");
		if (!line) {
			write(1, "exit\n", 5);
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
