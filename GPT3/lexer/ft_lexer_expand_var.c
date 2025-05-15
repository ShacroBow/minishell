#include "../minishell.h"

static char	*get_env_value(t_tokenize *t, char *name)
{
	size_t	namelen;
	int		k;

	if (!name || !*name || !t->env_list)
		return (NULL);
	namelen = ft_strlen(name);
	k = 0;
	while (t->env_list && t->env_list[k])
	{
		if (!ft_strncmp(t->env_list[k], name, namelen))
		{
			if (t->env_list[k][namelen] == '=')
				return (t->env_list[k] + namelen + 1);
		}
		k++;
	}
	return (getenv(name));
}

static char	*extract_simple_name(t_tokenize *t, const char *input, int *i)
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
		clean_exit_tokenize(t, "$VAR malloc failed");
	*i = j;
	return (name);
}

static char	*extract_braced_name(t_tokenize *t, const char *input, int *i)
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
		clean_exit_tokenize(t, "${VAR} malloc failed");
	j++;
	*i = j;
	return (name);
}

static void	handle_question_expansion(t_tokenize *t, int *i)
{
	expand_exit_status(t);
	*i += 2;
}

void	handle_var_expansion(t_tokenize *t, const char *input, int *i)
{
	char	*name;
	char	*val;

	if (input[*i + 1] == '?')
	{
		handle_question_expansion(t, i);
		return ;
	}
	if (input[*i + 1] == '{')
		name = extract_braced_name(t, input, i);
	else
		name = extract_simple_name(t, input, i);
	if (name == NULL)
	{
		buf_append(t, '$');
		(*i)++;
		return ;
	}
	val = get_env_value(t, name);
	if (val)
		append_value_to_buffer(t, val);
	free(name);
}