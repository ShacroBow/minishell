#include "../minishell.h"

int	handle_quotes_and_whitespace(t_tokenize *t, const char *input, int *i)
{
	char	c;

	c = input[*i];
	if (c == '\'' && !t->in_d)
	{
		t->in_s ^= 1;
		t->quoted = 1;
		(*i)++;
		return (1);
	}
	else if (c == '\"' && !t->in_s)
	{
		t->in_d ^= 1;
		t->quoted = 1;
		(*i)++;
		return (1);
	}
	if (!t->in_s && !t->in_d && (c == ' ' || c == '\t'))
	{
		if (t->len)
			create_token(t);
		(*i)++;
		return (1);
	}
	return (0);
}