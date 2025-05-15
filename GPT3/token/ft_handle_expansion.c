#include "../minishell.h"

static char	*ft_extract_simple_name(t_tokenize *t, const char *input, int *i)
{
	int		j;
	char	*name;

	j = *i + 1;
	if (!ft_isalpha(input[j]) && input[j] != '_')
		return (NULL);
	while (ft_isalnum(input[j]) || input[j] == '_')
		j++;
	name = ft_strndup(input + *i + 1, j - (*i + 1));
	if (!name)
		ft_exit_tokenize(t, "$VAR malloc failed");
	*i = j;
	return (name);
}

static char	*ft_extract_braced_name(t_tokenize *t, const char *input, int *i)
{
	int		j;
	char	*name;

	j = *i + 2;
	while (ft_isalnum(input[j]) || input[j] == '_')
		j++;
	if (input[j] != '}')
		return (NULL);
	name = ft_strndup(input + *i + 2, j - (*i + 2));
	if (!name)
		ft_exit_tokenize(t, "${VAR} malloc failed");
	j++;
	*i = j;
	return (name);
}

static void	ft_handle_question_expansion(t_tokenize *t, int *i)
{
	ft_expand_exit_status(t);
	*i += 2;
}

static int	ft_handle_escape(t_tokenize *t, const char *input, int *i)
{
	if (*i > 0 && input[*i] == '\\')
	{
		ft_tok_append(t, '$');
		(*i) += 2;
		return (1);
	}
	return (0);
}

void	ft_handle_expansion(t_tokenize *t, const char *input, int *i)
{
	char	*name;
	char	*val;

	if (ft_handle_escape(t, input, i))
		return ;
	if (input[*i + 1] == '?')
	{
		ft_handle_question_expansion(t, i);
		return ;
	}
	if (input[*i + 1] == '{')
		name = ft_extract_braced_name(t, input, i);
	else
		name = ft_extract_simple_name(t, input, i);
	if (name == NULL)
	{
		ft_tok_append(t, '$');
		(*i)++;
		return ;
	}
	val = ft_get_env_value(t, name);
	if (val)
		ft_tok_append_str(t, val);
	free(name);
}
