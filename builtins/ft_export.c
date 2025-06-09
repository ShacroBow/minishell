/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_export.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:03:36 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:03:37 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static char	*ft_dup_keyval(const char *kv, size_t klen)
{
	char	*out;

	if (kv[klen] == '\0')
	{
		out = malloc(klen + 2);
		if (!out)
			return (NULL);
		ft_memcpy(out, kv, klen);
		out[klen] = '=';
		out[klen + 1] = '\0';
		return (out);
	}
	return (ft_strdup(kv));
}

static int	ft_append_env(char ***envp, char *newstr)
{
	char	**grow;
	int		len;
	int		i;

	len = 0;
	while ((*envp)[len])
		len++;
	grow = malloc(sizeof(char *) * (len + 2));
	if (!grow)
		return (1);
	i = 0;
	while (i < len)
	{
		grow[i] = (*envp)[i];
		i++;
	}
	grow[len] = newstr;
	grow[len + 1] = NULL;
	free(*envp);
	*envp = grow;
	return (0);
}

void	ft_put_env(char ***envp, const char *kv)
{
	size_t	klen;
	int		i;
	char	*newstr;

	klen = 0;
	while (kv[klen] && kv[klen] != '=')
		klen++;
	i = 0;
	while ((*envp)[i])
	{
		if (!ft_strncmp((*envp)[i], kv, klen)
			&& (*envp)[i][klen] == '=')
		{
			if (kv[klen] == '\0')
				return ;
			free((*envp)[i]);
			(*envp)[i] = ft_strdup(kv);
			return ;
		}
		i++;
	}
	newstr = ft_dup_keyval(kv, klen);
	if (!newstr || ft_append_env(envp, newstr))
		perror("minishell");
}

static int	ft_is_valid_key(const char *s)
{
	int	i;

	i = 0;
	if (!(*s) || (!ft_isalpha(*s) && *s != '_'))
		return (0);
	while (s[i] && s[i] != '=')
	{
		if (!ft_isalnum(s[i]) && s[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

int	ft_export(char **argv, char ***envp)
{
	int	i;

	i = 1;
	if (!argv[1])
		return (ft_env(*envp));
	while (argv[i])
	{
		if (!ft_is_valid_key(argv[i]))
		{
			ft_putstr_fd("minishell: export: not a valid identifier\n", 2);
			return (1);
		}
		ft_put_env(envp, argv[i]);
		i++;
	}
	return (0);
}
