#include "../minishell.h"

static void	child_redir(t_command *c)
{
	int	fd;

	if (c->infile)
	{
		fd = open(c->infile, O_RDONLY);
		if (fd < 0)
			exit(EXIT_FAILURE);
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
	if (c->outfile)
	{
		if (c->append)
			fd = open(c->outfile, (O_CREAT | O_WRONLY | O_APPEND), 0644);
		else
			fd = open(c->outfile, (O_CREAT | O_WRONLY | O_TRUNC), 0644);
		if (fd < 0)
			exit(EXIT_FAILURE);
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
}

static void	child_dup_pipes(t_command *cmds, int idx, int n)
{
	int	k;

	if (idx && dup2(cmds->pipe_tab[idx - 1][0], STDIN_FILENO) == -1)
		exit(EXIT_FAILURE);
	if (idx < n - 1 && dup2(cmds->pipe_tab[idx][1], STDOUT_FILENO) == -1)
		exit(EXIT_FAILURE);
	k = 0;
	while (k < n - 1)
	{
		close(cmds->pipe_tab[k][0]);
		close(cmds->pipe_tab[k][1]);
		k++;
	}
}

static int	child_exec(t_command *c, int idx, int n, char **env)
{
	char	*path;

	ft_default_signals();
	child_dup_pipes(c, idx, n);
	child_redir(c);
	if (c->subshell)
		exit(ft_execute(c->subshell_segments));
	if (c->argv && ft_isbuiltin(c->argv[0]))
	{
		ft_execute_builtin(c, env);
		exit(g_exit_status);
	}
	if (!c->argv || !c->argv[0])
		exit(0);
	path = ft_find_binary(c->argv[0], env);
	if (!path)
	{
		ft_putstr_fd("minishell: command not found\n", STDERR_FILENO);
		return (127);
	}
	execve(path, c->argv, env);
	perror(c->argv[0]);
	return (126);
}

static void	ft_childrenfree(t_command *cmds, t_segment *seg, pid_t *pid, int ex)
{
	free_commands(cmds);
	ft_envpfree(*(seg->envp));
	free(seg);
	free(pid);
	exit(ex);
}

void	ft_spawn_children(t_command *cmds, int n, pid_t *pid, t_segment *seg)
{
	int			i;
	t_command	*cur;

	cur = cmds;
	i = 0;
	while (i < n)
	{
		pid[i] = fork();
		if (pid[i] == -1)
			ft_childrenfree(cmds, seg, pid, EXIT_FAILURE);
		if (pid[i] == 0)
			ft_childrenfree(cmds, seg, pid, child_exec(cur, i, n, *seg->envp));
		if (i)
			close(cmds->pipe_tab[i - 1][0]);
		if (i < n - 1)
			close(cmds->pipe_tab[i][1]);
		cur = cur->next;
		i++;
	}
}
