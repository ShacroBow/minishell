#include "../minishell.h"

extern volatile sig_atomic_t	g_exit_status;

/* void	free_tokens(t_token *tok, int count)
{
	if (!tok)
		return;
	for (int i = 0; i < count; ++i)
		free(tok[i].value);
	free(tok);
} */

void	free_tokens(t_token *tok, int count)
{
	int	i;

	i = 0;
	if (!tok)
		return ;
	while (i <= count)
	{
		free(tok[i].value);
		tok[i].value = NULL;
		i++;
	}
	free(tok);
	tok = NULL;
}

/* ───────── free command and segment lists ───────── */
/* void free_commands(t_command *cmd)
{
	while (cmd)
	{
		t_command *nx = cmd->next;
		if (cmd->argv)
		{
			for (int i = 0; cmd->argv[i]; ++i)
				free(cmd->argv[i]);
			free(cmd->argv);
		}
		if (cmd->infile)
		{
			if (cmd->heredoc)
				unlink(cmd->infile);
			free(cmd->infile);
		}
		if (cmd->outfile)
			free(cmd->outfile);
		if (cmd->subshell && cmd->subshell_segments)
			free_segments(cmd->subshell_segments);
		free(cmd);
		cmd = nx;
	}
} */

void	free_commands(t_command *cmd)
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
			free_segments(cmd->subshell_segments);
		free(cmd);
		cmd = next;
	}
}

/* void free_segments(t_segment *seg)
{
	while (seg) {
		t_segment *nx = seg->next;
		if (seg->pipeline)
			free_commands(seg->pipeline);
		free(seg);
		seg = nx;
	}
} */

void	free_segments(t_segment *seg)
{
	t_segment	*next;

	while (seg)
	{
		next = seg->next;
		if (seg->pipeline)
			free_commands(seg->pipeline);
		free(seg);
		seg = next;
	}
}

/* ───────── dynamic buffer append for tokenization ───────── */
/* void buf_append(char **buf, int *cap, int *len, char c)
{
	if (*len + 1 >= *cap) {
		*cap *= 2;
		*buf = realloc(*buf, *cap);
		if (!*buf)
		{
			perror("minishell");
			exit(EXIT_FAILURE);
		}
	}
	(*buf)[(*len)++] = c;
	(*buf)[*len] = '\0';
} */

/* ───────── error message for syntax errors ───────── */
int	redir_error(void)
{
	write(2, "minishell: syntax error near unexpected token\n", 47);
	g_exit_status = 258;
	return (-1);
}
/* static void add_arg(t_command *cmd, const char *val)
{
	int argc = 0;
	while (cmd->argv && cmd->argv[argc])
		argc++;
	char **newargv = malloc((argc + 2) * sizeof(char *));
	if (!newargv) { perror("minishell"); exit(EXIT_FAILURE); }
	for (int i = 0; i < argc; ++i)
		newargv[i] = cmd->argv[i];
	newargv[argc] = ft_strdup(val);
	if (!newargv[argc]) { perror("minishell"); exit(EXIT_FAILURE); }
	newargv[argc + 1] = NULL;
	free(cmd->argv);
	cmd->argv = newargv;
} */

/* ───────── add an argument to a command's argv ───────── */
static void	add_arg(t_command *cmd, const char *val)
{
	int		argc;
	char	**newargv;
	int		i;

	argc = 0;
	i = -1;
	while (cmd->argv && cmd->argv[argc])
		argc++;
	newargv = malloc((argc + 2) * sizeof(char *));
	if (!newargv)
	{
		perror("minishell");
		exit(EXIT_FAILURE);
	}
	while (++i < argc)
		newargv[i] = cmd->argv[i];
	newargv[argc] = ft_strdup(val);
	if (!newargv[argc])
	{
		perror("minishell");
		exit(EXIT_FAILURE);
	}
	newargv[argc + 1] = NULL;
	free(cmd->argv);
	cmd->argv = newargv;
}

/* ───────── allocate a new command in the pipeline if needed ───────── */
void	new_command_if_needed(t_command **cmd_head, \
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

/*int	handle_parenthesis(t_token *tok, int *idx, int n, t_command *cur)
{
	t_parse_segments ps;

	(*idx)++;
	cur->subshell = 1;
	ps.idx = *idx;
	ps.n = n;
	ps.in_sub = 1;
	cur->subshell_segments = parse_segments(tok, &ps, cur->envp);
	if (!cur->subshell_segments)
	{
		// inner parse error: abort parsing 
		return (-1);
	}
	*idx = ps.idx;               // advance outer index past ')'
	cur->argv = malloc(sizeof(char*) * 2);
	if (!cur->argv) { perror("minishell"); exit(EXIT_FAILURE); }
	cur->argv[0] = ft_strdup("SUBSHELL");
	cur->argv[1] = NULL;
	return 0;
}*/

/* ───────── handle subshell start '(' ───────── */
int	handle_parenthesis(t_token *tok, int *idx, int n, t_command *cur)
{
	t_parse_segments	ps;

	(*idx)++;
	cur->subshell = 1;
	ps.idx = *idx;
	ps.n = n;
	ps.in_sub = 1;
	cur->subshell_segments = parse_segments(tok, &ps, cur->envp);
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

/* ───────── handle a word token (arguments and wildcards) ───────── */
/*int	handle_word(t_token *tok, int *idx, t_command *cur)
{
	char *val = tok[*idx].value;
	if (!tok[*idx].quoted && ft_strchr(val, '*')) {
		char **matches = expand_wildcard(val);
		if (matches) {
			for (int k = 0; matches[k]; ++k)
				add_arg(cur, matches[k]);
			for (int k = 0; matches[k]; ++k)
				free(matches[k]);
			free(matches);
		} else {
			add_arg(cur, val);
		}
	} else {
		add_arg(cur, val);
	}
	(*idx)++;
	return 0;
} */

static void	ft_free_tab(char **tab)
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

int	handle_word(t_token *tk, int *idx, t_command *cur)
{
	char	**match;
	int		k;

	if (!tk[*idx].quoted && ft_strchr(tk[*idx].value, '*'))
	{
		match = expand_wildcard(tk[*idx].value);
		if (match)
		{
			k = 0;
			while (match[k])
				add_arg(cur, match[k++]);
			ft_free_tab(match);
		}
		else
			add_arg(cur, tk[*idx].value);
	}
	else
		add_arg(cur, tk[*idx].value);
	(*idx)++;
	return (0);
}

/* ───────── push a completed pipeline into segment list ───────── */
void	push_pipeline_to_segments(t_segment **h, t_segment **t, \
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

/* ───────── handle &&, ||, | operators ───────── */
/* ───────── recursive descent parser for command sequence ───────── */

static t_segment	*handle_parse_segments(t_token *tok, int tcount, char **env)
{
	t_segment			*ast;
	t_parse_segments	ps;

	ps.idx = 0;
	ps.in_sub = 0;
	ps.n = tcount;
	ast = parse_segments(tok, &ps, env);
	if (ps.idx < tcount && ast)
	{
		ft_putstr_fd("minishell: syntax error near unexpected token\n", \
						STDERR_FILENO);
		g_exit_status = 258;
		free_segments(ast);
		ast = NULL;
	}
	return (ast);
}

/* ───────── top-level parse function ───────── */
static int	detect_unclosed_quotes(const char *input)
{
	int	quotes_s;
	int	quotes_d;
	int	i;

	quotes_s = 0;
	quotes_d = 0;
	i = 0;
	while (input[i] != '\0')
	{
		if (input[i] == '\'' && !quotes_d)
			quotes_s ^= 1;
		else if (input[i] == '"' && !quotes_s)
			quotes_d ^= 1;
		i++;
	}
	return (quotes_s || quotes_d);
}

/* ───────── top-level parse function ───────── */
t_segment	*parse_input(const char *input, char ***envp)
{
	int					tcount;
	t_token				*tok;
	t_segment			*ast;

	tcount = 0;
	if (detect_unclosed_quotes(input))
	{
		ft_putstr_fd("minishell: syntax error unclosed quote\n", STDERR_FILENO);
		return (g_exit_status = 258, NULL);
	}
	tok = ft_tokenize(input, &tcount, *envp);
	if (!tok)
		return (NULL);
	ast = handle_parse_segments(tok, tcount, *envp);
	free_tokens(tok, tcount);
	if (ast)
		ast->envp = envp;
	return (ast);
}
