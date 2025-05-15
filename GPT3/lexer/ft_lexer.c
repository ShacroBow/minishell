#include "../minishell.h"

void	buf_append(char **buf, int *cap, int *len, char c)
{
	int	new_cap;

	if (*len + 1 >= *cap)
	{
		if (*cap > INT_MAX / 2)
			exit_error("Buffer resize failed: capacity overflow");
		new_cap = *cap * 2;
		*cap = new_cap;
		*buf = ft_resize_buffer(*buf, *cap);
		if (!*buf)
			exit_error("Allocation new buffer malloc failed");
	}
	(*buf)[(*len)++] = c;
	(*buf)[*len] = '\0';
}

void	create_token(t_tokenize *t)
{
	t->tok[t->count].type = TOK_WORD;
	t->tok[t->count].value = t->buf;
	t->tok[t->count].quoted = t->quoted;
	t->count++;
	if (t->is_heredoc == 1)
		t->is_heredoc = 0;
	t->cap = 128;
	t->len = 0;
	t->buf = ft_calloc(t->cap, sizeof(char));
	if (!t->buf)
		exit_error("Create new token malloc failed");
	t->buf[0] = '\0';
	t->quoted = 0;
}

static void	start_tokenize(t_tokenize *t, const char *input, int *i)
{
	if (handle_quotes_and_whitespace(t, input, i))
		return ;
	if (!t->in_s && !t->in_d && is_operator(input[*i]))
	{
		handle_operator(t, input, i);
		return ;
	}
	if (input[*i] == '$' && !t->in_s && !t->is_heredoc)
	{
		handle_var_expansion(t, input, i);
			return ;
	}
	buf_append(&t->buf, &t->cap, &t->len, input[*i]);
	if (t->in_s || t->in_d)
		t->quoted = 1;
	(*i)++;
}

static void	init_struct_tokenization(t_tokenize *t, const char *input, char **env_list)
{
	t->cap = 128;
	t->len = 0;
	t->count = 0;
	t->buf = ft_calloc(t->cap, sizeof(char));
	if (!t->buf)
		exit_error("Token buffer initialisation malloc failed");
	t->tok = malloc(sizeof(t_token) * (ft_strlen(input) + 1));
	if (!t->tok)
	{
		free(t->buf);
		exit_error("Array of struct Token initialisation malloc failed");
	}
	t->in_s = 0;
	t->in_d = 0;
	t->quoted = 0;
	t->env_list = env_list;
	t->is_heredoc = 0;
}

t_token	*tokenize(const char *input, int *out_count, char **env_list)
{
	t_tokenize	t;
	int			i;

	//No handling of SIGINT during tokenization
	//if (setup_tokenize_signals() == -1) // New function
    //    return (NULL);
	init_struct_tokenization(&t, input, env_list);
	i = 0;
	while (input[i])
		start_tokenize(&t, input, &i);
	if (t.len)
		create_token(&t);
	else
		free(t.buf);
	*out_count = t.count;
	return (t.tok);
}
