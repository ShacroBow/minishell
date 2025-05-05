#include "../minishell.h"

int	ft_execute(t_segment *seg)
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
						return (ft_envpfree(*(seg->envp)), free_segments(seg), -1);
					g_exit_status = skip;
					free_segments(seg);
					return (-1);
				}
				else
					*(seg->envp) = execute_builtin(cmd, *(seg->envp));
			}
			else
				ft_run_pipeline(seg->pipeline, *(seg->envp));
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
