/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_free_segments_bonus.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:05:02 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:21:56 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell_bonus.h"

void	ft_free_tab(char **tab)
{
	int	i;

	if (!tab)
		return ;
	i = 0;
	while (tab[i])
	{
		free(tab[i]);
		i++;
	}
	free(tab);
	tab = NULL;
}

void	ft_free_commands(t_command *cmd)
{
	t_command	*next;
	int			i;

	while (cmd)
	{
		next = cmd->next;
		i = 0;
		while (cmd->argv && cmd->argv[i])
		{
			free(cmd->argv[i]);
			i++;
		}
		free(cmd->argv);
		if (cmd->infile && cmd->heredoc)
			unlink(cmd->infile);
		free(cmd->infile);
		free(cmd->outfile);
		if (cmd->subshell)
			ft_free_segments(cmd->subshell_segments);
		free(cmd);
		cmd = next;
	}
}

void	ft_free_segments(t_segment *seg)
{
	t_segment	*next;

	while (seg)
	{
		next = seg->next;
		if (seg->pipeline)
			ft_free_commands(seg->pipeline);
		free(seg);
		seg = next;
	}
}
