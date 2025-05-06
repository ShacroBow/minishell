#include "../minishell.h"

static int	count_cmds(t_command *c)
{
	int	n;

	n = 0;
	while (c)
	{
		n++;
		c = c->next;
	}
	return (n);
}

/* ---------- allocate + open (n‑1) pipes --------------------------------- */
static t_pipe_fd	*make_pipes(int n)
{
	int			i;
	t_pipe_fd	*tab;

	if (n <= 1)
		return (NULL);
	tab = malloc(sizeof(t_pipe_fd) * (n - 1));
	if (!tab)
		exit(EXIT_FAILURE);
	i = 0;
	while (i < n - 1)
	{
		if (pipe(tab[i]) == -1)
			exit(EXIT_FAILURE);
		i++;
	}
	return (tab);
}

/* ---------- simple file redirection ------------------------------------- */
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

/* ---------- duplicate pipes + close unused ------------------------------ */
static void	child_dup_pipes(int idx, int n, t_pipe_fd *p)
{
	int	k;

	if (idx && dup2(p[idx - 1][0], STDIN_FILENO) == -1)
		exit(EXIT_FAILURE);
	if (idx < n - 1 && dup2(p[idx][1], STDOUT_FILENO) == -1)
		exit(EXIT_FAILURE);
	k = 0;
	while (k < n - 1)
	{
		close(p[k][0]);
		close(p[k][1]);
		k++;
	}
}

/* ---------- code executed by each child --------------------------------- */
static void	child_exec(t_command *c, int idx, int n, t_pipe_fd *p, char **env)
{
	char	*path;

	restore_default_signals();
	child_dup_pipes(idx, n, p);
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
		(write(2, "minishell: command not found\n", 29), exit(127));
	execve(path, c->argv, env);
	perror(c->argv[0]);
	exit(126);
}

/* ---------- fork loop (parent side) ------------------------------------- */
static void	spawn_children(t_command *cmds, int n,
						t_pipe_fd *p, pid_t *pid, char **env)
{
	int			i;
	t_command	*cur;

	cur = cmds;
	i = 0;
	while (i < n)
	{
		pid[i] = fork();
		if (pid[i] == -1)
			exit(EXIT_FAILURE);
		if (pid[i] == 0)
			child_exec(cur, i, n, p, env);
		if (i)
			close(p[i - 1][0]);
		if (i < n - 1)
			close(p[i][1]);
		cur = cur->next;
		i++;
	}
}

/* ---------- wait for children, set global status ------------------------ */
static void	parent_wait(pid_t *pid, int n)
{
	int	i;
	int	st;

	signal(SIGINT, SIG_IGN);
	st = 0;
	i = 0;
	while (i < n)
	{
		waitpid(pid[i], &st, 0);
		i++;
	}
	setup_signals();
	if (WIFEXITED(st))
		g_exit_status = WEXITSTATUS(st);
	else if (WIFSIGNALED(st))
		g_exit_status = 128 + WTERMSIG(st);
}

/* ---------- public entry ------------------------------------------------- */
void	ft_run_pipeline(t_command *cmds, char **envp)
{
	int			n;
	t_pipe_fd	*pipes;
	pid_t		*pids;

	n = count_cmds(cmds);
	pipes = make_pipes(n);
	pids = malloc(sizeof(pid_t) * n);
	if (!pids)
		exit(EXIT_FAILURE);
	spawn_children(cmds, n, pipes, pids, envp);
	parent_wait(pids, n);
	free(pipes);
	free(pids);
}
