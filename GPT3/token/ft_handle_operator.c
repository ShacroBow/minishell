#include "../minishell.h"

static t_tokentype	ft_char_op_type_two(char c)
{
	if (c == '&')
		return (TOK_AND);
	else if (c == '|')
		return (TOK_OR);
	else if (c == '<')
		return (TOK_HEREDOC);
	else
		return (TOK_APPEND);
}

static t_tokentype	ft_char_op_type(char c)
{
	if (c == '|')
		return (TOK_PIPE);
	else if (c == '<')
		return (TOK_REDIR_IN);
	else if (c == '>')
		return (TOK_REDIR_OUT);
	else if (c == '(')
		return (TOK_LPAREN);
	else
		return (TOK_RPAREN);
}

static void	ft_handle_double_operator(t_tokenize *t, const char *input, int *i)
{
	t_tokentype	tt;

	if (t->len)
		ft_create_token(t);
	tt = ft_char_op_type_two(input[*i]);
	if (tt == TOK_HEREDOC)
		t->is_heredoc = 1;
	t->tok[t->count].value = ft_strndup(input + *i, 2);
	if (!t->tok[t->count].value)
		ft_exit_tokenize(t, "Token type double operator malloc failed");
	*i += 2;
	t->tok[t->count].type = tt;
	t->tok[t->count].quoted = 0;
	t->count++;
}

static void	ft_handle_simple_operator(t_tokenize *t, const char *input, int *i)
{
	t_tokentype	tt;

	if (t->len)
		ft_create_token(t);
	tt = ft_char_op_type(input[*i]);
	t->tok[t->count].value = ft_strndup(input + *i, 1);
	if (!t->tok[t->count].value)
		ft_exit_tokenize(t, "Token type simple operator malloc failed");
	*i += 1;
	t->tok[t->count].type = tt;
	t->tok[t->count].quoted = 0;
	t->count++;
}

void	ft_handle_operator(t_tokenize *t, const char *input, int *i)
{
	if ((input[*i] == '&' && input[*i + 1] == '&') \
		|| (input[*i] == '|' && input[*i + 1] == '|') \
		|| (input[*i] == '<' && input[*i + 1] == '<') \
		|| (input[*i] == '>' && input[*i + 1] == '>'))
		ft_handle_double_operator(t, input, i);
	else
		ft_handle_simple_operator(t, input, i);
}
