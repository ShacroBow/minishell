/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_parse_segments_bonus.c                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:08 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:21:56 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell_bonus.h"

static int	ft_handle_pipe_and_or(t_ctx *c, t_tokentype typ)
{
	if (c->need_cmd)
		return (ft_redir_error());
	if (typ == TOK_PIPE)
	{
		(c->ps->idx)++;
		c->need_cmd = 1;
		return (0);
	}
	ft_push_pipe_to_seg(c->seg_head, c->seg_tail, c->cmd_head, typ);
	c->cmd_head = NULL;
	c->cmd_tail = NULL;
	c->need_cmd = 1;
	(c->ps->idx)++;
	return (0);
}

static int	ft_dispatch_tok(t_ctx *c)
{
	t_tokentype	typ;

	typ = c->tok[c->ps->idx].type;
	if (typ == TOK_LPAREN)
		return (ft_handle_parenthesis(c->tok, &(c->ps->idx), c->ps->n, \
									c->cmd_tail));
	if (typ == TOK_WORD)
		return (ft_handle_word(c->tok, &(c->ps->idx), c->cmd_tail));
	if (typ == TOK_REDIR_IN || typ == TOK_HEREDOC || typ == TOK_REDIR_OUT || \
		typ == TOK_APPEND)
		return (ft_handle_redirection(c->tok, &(c->ps->idx), c->ps->n, \
				c->cmd_tail));
	(c->ps->idx)++;
	return (0);
}

static int	ft_process_iter(t_ctx *c)
{
	t_tokentype	typ;

	typ = c->tok[c->ps->idx].type;
	if (typ == TOK_PIPE || typ == TOK_AND || typ == TOK_OR)
		return (ft_handle_pipe_and_or(c, typ));
	if (c->need_cmd)
		ft_new_command(&(c->cmd_head), \
				&(c->cmd_tail), &(c->need_cmd));
	c->cmd_tail->envp = c->env;
	return (ft_dispatch_tok(c));
}

static int	ft_parse_loop(t_ctx *c)
{
	int	ret;

	c->cmd_head = NULL;
	c->cmd_tail = NULL;
	c->need_cmd = 1;
	while (c->ps->idx < c->ps->n)
	{
		ret = ft_process_iter(c);
		if (ret < 0)
			return (-1);
		if (ret > 0)
			break ;
	}
	if (c->cmd_head)
		ft_push_pipe_to_seg(c->seg_head, c->seg_tail, c->cmd_head, 0);
	return (0);
}

t_segment	*ft_parse_segments(t_token *tok, t_parse_segments *ps, char **env)
{
	t_segment	*seg_head;
	t_segment	*seg_tail;
	t_ctx		ctx;
	int			res;

	seg_head = NULL;
	seg_tail = NULL;
	ctx.tok = tok;
	ctx.ps = ps;
	ctx.env = env;
	ctx.seg_head = &seg_head;
	ctx.seg_tail = &seg_tail;
	res = ft_parse_loop(&ctx);
	if (res < 0)
	{
		ft_free_segments(seg_head);
		if (ctx.cmd_head)
			ft_free_commands(ctx.cmd_head);
		return (NULL);
	}
	return (seg_head);
}
