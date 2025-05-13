#include "../minishell.h"

static int	ft_count_cmds(t_command *c)
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

static void	ft_parent_wait(pid_t *pid, int n)
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
	ft_signal_setup();
	if (WIFEXITED(st))
		g_exit_status = WEXITSTATUS(st);
	else if (WIFSIGNALED(st))
		g_exit_status = 128 + WTERMSIG(st);
}

void	ft_run_pipeline(t_command *cmds, char **envp)
{
	int			n;
	t_pipe_fd	*pipes;
	t_command	*tmp;
	pid_t		*pids;

	n = ft_count_cmds(cmds);
	pipes = ft_make_pipes(n);
	if (!pipes && n > 1)
	{
		g_exit_status = 1;
		return ;
	}
	tmp = cmds;
	while (tmp)
	{
		tmp->pipe_tab = pipes;
		tmp = tmp->next;
	}
	pids = malloc(sizeof(pid_t) * n);
	if (!pids)
		exit(EXIT_FAILURE);
	ft_spawn_children(cmds, n, pids, envp);
	ft_parent_wait(pids, n);
	free(pipes);
	free(pids);
}
