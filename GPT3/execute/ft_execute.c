/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_execute.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:03:42 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:03:43 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static int	ft_runbuiltin(t_command *cmd, char ***envp)
{
	int	ret;

	if (ft_strcmp(cmd->argv[0], "exit") == 0)
	{
		ret = ft_exit(cmd->argv);
		if (ret == -1)
			return (-1);
		g_exit_status = ret;
		return (-1);
	}
	else
		*envp = ft_execute_builtin(cmd, *envp);
	return (0);
}

static void	ft_handle_skip(t_segment *seg, int *skip)
{
	if (seg->op == TOK_AND)
		*skip = (g_exit_status != 0);
	else if (seg->op == TOK_OR)
		*skip = (g_exit_status == 0);
	else
		*skip = 0;
}

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
			if (cmd && !cmd->next && cmd->argv && ft_isbuiltin(cmd->argv[0])
				&& !cmd->subshell)
			{
				if (ft_runbuiltin(cmd, seg->envp))
					return (ft_free_segments(seg), -1);
			}
			else
				ft_run_pipeline(cmd, seg);
		}
		ft_handle_skip(seg, &skip);
		if (seg->next)
			seg->next->envp = seg->envp;
		seg = seg->next;
	}
	return (g_exit_status);
}
