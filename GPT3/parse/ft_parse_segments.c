#include "../minishell.h"

#include "../minishell.h"

/* ───────── local context ───────── */


/* ───────── tiny helpers ───────── */

static int	handle_rparen(t_ctx *c)
{
	if (c->ps->in_sub == 0)
	{
		redir_error();
		free_segments(*(c->seg_head));
		return (-1);
	}
	(c->ps->idx)++;
	return (1);
}

static int	handle_pipe_andor(t_ctx *c, t_tokentype typ)
{
	if (c->need_cmd)
		return (redir_error());
	if (typ == TOK_PIPE)
	{
		(c->ps->idx)++;
		c->need_cmd = 1;
		return (0);
	}
	push_pipeline_to_segments(c->seg_head, c->seg_tail, c->cmd_head, typ);
	c->cmd_head = NULL;
	c->cmd_tail = NULL;
	c->need_cmd = 1;
	(c->ps->idx)++;
	return (0);
}

static int	dispatch_tok(t_ctx *c)
{
	t_tokentype	typ;

	typ = c->tok[c->ps->idx].type;
	if (typ == TOK_LPAREN)
		return (handle_parenthesis(c->tok, &(c->ps->idx), c->ps->n, \
									c->cmd_tail));
	if (typ == TOK_WORD)
		return (handle_word(c->tok, &(c->ps->idx), c->cmd_tail));
	if (typ == TOK_REDIR_IN || typ == TOK_HEREDOC || typ == TOK_REDIR_OUT || \
		typ == TOK_APPEND)
		return (handle_redirection(c->tok, &(c->ps->idx), c->ps->n, \
				c->cmd_tail));
	(c->ps->idx)++;
	return (0);
}

static int	process_iter(t_ctx *c)
{
	t_tokentype	typ;

	typ = c->tok[c->ps->idx].type;
	if (typ == TOK_RPAREN)
		return (handle_rparen(c));
	if (typ == TOK_PIPE || typ == TOK_AND || typ == TOK_OR)
		return (handle_pipe_andor(c, typ));
	if (c->need_cmd)
		new_command_if_needed(&(c->cmd_head),
				&(c->cmd_tail), &(c->need_cmd));
	c->cmd_tail->envp = c->env;
	return (dispatch_tok(c));
}

static int	parse_loop(t_ctx *c)
{
	int	ret;

	c->cmd_head = NULL;
	c->cmd_tail = NULL;
	c->need_cmd = 1;
	while (c->ps->idx < c->ps->n)
	{
		ret = process_iter(c);
		if (ret < 0)
			return (-1);
		if (ret > 0)
			break ;
	}
	if (c->cmd_head)
		push_pipeline_to_segments(c->seg_head, c->seg_tail, c->cmd_head, 0);
	return (0);
}

/* ───────── public entry ───────── */

t_segment	*parse_segments(t_token *tok, t_parse_segments *ps, char **env)
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
	res = parse_loop(&ctx);
	if (res < 0)
		return (NULL);
	return (seg_head);
}
