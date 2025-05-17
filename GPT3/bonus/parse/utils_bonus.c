/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:17 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:21:56 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell_bonus.h"

int	ft_handle_parenthesis(t_token *tok, int *idx, int n, t_command *cur)
{
	t_parse_segments	ps;

	(*idx)++;
	cur->subshell = 1;
	ps.idx = *idx;
	ps.n = n;
	ps.in_sub = 1;
	cur->subshell_segments = ft_parse_segments(tok, &ps, cur->envp);
	if (!cur->subshell_segments)
		return (-1);
	*idx = ps.idx;
	cur->argv = malloc(sizeof(char *) * 2);
	if (!cur->argv)
	{
		perror("minishell");
		return (-1);
	}
	cur->argv[0] = ft_strdup("SUBSHELL");
	if (!cur->argv[0])
	{
		perror("minishell");
		return (-1);
	}
	cur->argv[1] = NULL;
	return (0);
}

int	ft_handle_word(t_token *tk, int *idx, t_command *cur)
{
	char	**match;
	int		k;

	if (!tk[*idx].quoted && ft_strchr(tk[*idx].value, '*'))
	{
		match = ft_expand_wildcard(tk[*idx].value);
		if (match)
		{
			k = 0;
			while (match[k])
				ft_add_arg(cur, match[k++]);
			ft_free_tab(match);
		}
		else
			ft_add_arg(cur, tk[*idx].value);
	}
	else
		ft_add_arg(cur, tk[*idx].value);
	(*idx)++;
	return (0);
}

void	ft_new_command(t_command **cmd_head, \
								t_command **cmd_tail, int *need_cmd)
{
	t_command	*cm;

	if (*need_cmd)
	{
		cm = ft_calloc(1, sizeof(t_command));
		if (!cm)
		{
			perror("minishell");
			exit(EXIT_FAILURE);
		}
		if (!*cmd_head)
			*cmd_head = cm;
		else
			(*cmd_tail)->next = cm;
		*cmd_tail = cm;
		*need_cmd = 0;
	}
}

int	ft_redir_error(void)
{
	ft_putstr_fd("minishell: syntax error near unexpected token\n", \
				STDERR_FILENO);
	g_exit_status = 258;
	return (-1);
}

void	ft_push_pipe_to_seg(t_segment **h, t_segment **t, \
									t_command *pipe, t_tokentype op)
{
	t_segment	*seg;

	seg = malloc(sizeof(t_segment));
	if (!seg)
	{
		perror("minishell");
		exit(EXIT_FAILURE);
	}
	seg->pipeline = pipe;
	seg->op = op;
	seg->next = NULL;
	if (!*h)
		*h = seg;
	else
		(*t)->next = seg;
	*t = seg;
}
