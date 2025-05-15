#include "../minishell.h"

void	expand_exit_status(t_tokenize *t)
{
	char	*tmp;
	int		i;

	i = 0;
	tmp = ft_itoa(g_exit_status);
	if (!tmp)
		clean_exit_tokenize(t, "$? expand malloc failed");
	while (tmp[i])
	{
		buf_append(t, tmp[i]);
		i++;
	}
	free(tmp);
}

void	append_value_to_buffer(t_tokenize *t, char *val)
{
	int	k;

	k = 0;
	while (val[k])
	{
		buf_append(t, val[k]);
		k++;
	}
}

int	is_operator(char c)
{
	return (c == '|' || c == '&' || c == '<' || c == '>' || c == '(' || c == ')');
}
