#include "../minishell.h"

void	ft_expand_exit_status(t_tokenize *t)
{
	char	*tmp;
	int		i;

	i = 0;
	tmp = ft_itoa(g_exit_status);
	if (!tmp)
		ft_exit_tokenize(t, "$? expand malloc failed");
	while (tmp[i])
	{
		ft_tok_append(t, tmp[i]);
		i++;
	}
	free(tmp);
}

void	ft_tok_append_str(t_tokenize *t, char *val)
{
	int	k;

	k = 0;
	while (val[k])
	{
		ft_tok_append(t, val[k]);
		k++;
	}
}

int	ft_is_operator(char c)
{
	return (c == '|' || c == '&' || c == '<' || \
			c == '>' || c == '(' || c == ')');
}

char	*ft_get_env_value(t_tokenize *t, char *name)
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
