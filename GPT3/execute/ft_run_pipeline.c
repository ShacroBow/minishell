#include "../minishell.h"

void ft_run_pipeline(t_command *cmds, char **envp)
{
	/* count commands in the pipeline */
	int n = 0;
	for (t_command *tmp = cmds; tmp; tmp = tmp->next)
		++n;
	int pipes[n][2];
	for (int i = 0; i < n - 1; ++i)
	{
		if (pipe(pipes[i]) < 0)
		{
			perror("minishell");
			exit(EXIT_FAILURE);
		}
	}
	pid_t pids[n];
	t_command *cur = cmds;
	for (int i = 0; i < n; ++i, cur = cur->next)
	{
		pids[i] = fork();
		if (pids[i] < 0)
		{
			perror("minishell");
			exit(EXIT_FAILURE);
		}
		if (pids[i] == 0)
		{  /* child process */
			restore_default_signals();
			if (i > 0)
				dup2(pipes[i - 1][0], STDIN_FILENO);
			if (cur->infile) {
				int fd = open(cur->infile, O_RDONLY);
				if (fd < 0) { perror(cur->infile); exit(EXIT_FAILURE); }
				dup2(fd, STDIN_FILENO);
				close(fd);
			}
			if (i < n - 1)
				dup2(pipes[i][1], STDOUT_FILENO);
			if (cur->outfile) {
				int fd = open(cur->outfile, O_WRONLY | O_CREAT |
							(cur->append ? O_APPEND : O_TRUNC), 0644);
				if (fd < 0) { perror(cur->outfile); exit(EXIT_FAILURE); }
				dup2(fd, STDOUT_FILENO);
				close(fd);
			}
			/* close all pipe fds in the child */
			for (int j = 0; j < n - 1; ++j) {
				close(pipes[j][0]);
				close(pipes[j][1]);
			}
			if (cur->subshell)
				exit(ft_execute(cur->subshell_segments));
			if (cur->argv && ft_isbuiltin(cur->argv[0])) {
				execute_builtin(cur, envp);
				exit(g_exit_status);
			}
			if (!cur->argv || !cur->argv[0])
				exit(0);
			char *path = ft_find_binary(cur->argv[0], envp);
			if (!path) {
				write(STDERR_FILENO, "minishell: command not found\n", 29);
				exit(127);
			}
			execve(path, cur->argv, envp);
			perror(cur->argv[0]);
			free(path);
			exit(126);
		}
		/* parent process closes used pipe ends */
		if (i > 0)
			close(pipes[i - 1][0]);
		if (i < n - 1)
			close(pipes[i][1]);
	}
	/* parent ignores Ctrl-C and waits for children */
	signal(SIGINT, SIG_IGN);
	int status = 0;
	for (int i = 0; i < n; ++i)
		waitpid(pids[i], &status, 0);
	setup_signals();
	if (WIFEXITED(status))
		g_exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		if (sig == SIGQUIT)
			write(STDERR_FILENO, "Quit: 3\n", 8);
		else if (sig == SIGINT)
			write(STDOUT_FILENO, "\n", 1);
		g_exit_status = 128 + sig;
	}
}
