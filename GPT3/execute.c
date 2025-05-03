#include "minishell.h"
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

/* ───────── helper: get $PATH variable value ───────── */
static char *get_env_val(char **envp, const char *key) {
	size_t len = ft_strlen(key);
	for (int i = 0; envp && envp[i]; ++i) {
		if (!ft_strncmp(envp[i], key, len) && envp[i][len] == '=')
			return (envp[i] + len + 1);
	}
	return NULL;
}

static void free_strarray(char **arr) {
	for (int i = 0; arr && arr[i]; ++i)
		free(arr[i]);
	free(arr);
}

/* ───────── search for an executable in PATH ───────── */
char *find_executable(const char *cmd, char **envp) {
	if (!cmd || !*cmd)
		return NULL;
	if (ft_strchr(cmd, '/'))
		return (access(cmd, X_OK) == 0 ? ft_strdup(cmd) : NULL);
	char *pathval = get_env_val(envp, "PATH");
	if (!pathval || !*pathval)
		pathval = "/usr/local/bin:/usr/bin:/bin";
	char **dirs = ft_split(pathval, ':');
	if (!dirs)
		return NULL;
	char *found = NULL;
	for (int i = 0; dirs[i] && !found; ++i) {
		char *tmp = ft_strjoin(dirs[i], "/");
		if (!tmp) { free_strarray(dirs); return NULL; }
		char *full = ft_strjoin(tmp, cmd);
		free(tmp);
		if (!full) { free_strarray(dirs); return NULL; }
		if (access(full, X_OK) == 0)
			found = full;
		else
			free(full);
	}
	free_strarray(dirs);
	return found;
}

/* ───────── execute a pipeline of commands ───────── */
static void run_pipeline(t_command *cmds, char **envp)
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
				exit(execute_segments(cur->subshell_segments, &envp));
			if (cur->argv && ft_isbuiltin(cur->argv[0])) {
				execute_builtin(cur, envp);
				exit(g_exit_status);
			}
			if (!cur->argv || !cur->argv[0])
				exit(0);
			char *path = find_executable(cur->argv[0], envp);
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

/* ───────── execute chained segments (&&, ||) ───────── */
int	execute_segments(t_segment *seg, char ***envp)
{
	int			skip;
	t_command	*cmd;

	skip = 0;
	while (seg)
	{
		if (!skip)
		{
			cmd = seg->pipeline;
			if (cmd && !cmd->next && cmd->argv && ft_isbuiltin(cmd->argv[0]) &&
				!cmd->subshell)
			{
				/* single builtin not in pipeline: run in parent */
				if (!ft_strcmp(cmd->argv[0], "exit"))
				{
					skip = ft_exit(cmd->argv);
					if (skip == -1)
						return (ft_envpfree(*envp), free_segments(seg), -1);
					g_exit_status = skip;
					free_segments(seg);
					return (-1);
				}
				else
					*envp = execute_builtin(cmd, *envp);
			}
			else
				run_pipeline(seg->pipeline, *envp);
		}
		if (seg->op == TOK_AND)
			skip = (g_exit_status != 0);
		else if (seg->op == TOK_OR)
			skip = (g_exit_status == 0);
		else
			skip = 0;
		seg = seg->next;
	}
	return (g_exit_status);
}
