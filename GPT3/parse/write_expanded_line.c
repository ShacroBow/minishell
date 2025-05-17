/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   write_expanded_line.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:21 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:04:22 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static char	*ft_get_env_val(char **env, const char *key, size_t klen)
{
	while (env && *env)
	{
		if (!ft_strncmp(*env, key, klen) && (*env)[klen] == '=')
			return ((*env) + klen + 1);
		env++;
	}
	return (getenv(ft_substr(key, 0, klen)));
}

static int	ft_write_status(int fd)
{
	char	*val;
	int		res;

	val = ft_itoa(g_exit_status);
	if (!val)
		return (-1);
	res = ft_write_value(fd, val);
	free(val);
	return (res);
}

static int	ft_handle_dollar(char *ln, size_t *i, char **env, int fd)
{
	size_t	j;
	size_t	klen;
	char	*val;

	if (ln[*i + 1] == '?')
	{
		*i += 2;
		return (ft_write_status(fd));
	}
	j = *i + 1;
	while (ft_isalnum(ln[j]) || ln[j] == '_')
		j++;
	klen = j - *i - 1;
	val = ft_get_env_val(env, ln + *i + 1, klen);
	if (ft_write_value(fd, val) == -1)
		return (-1);
	*i = j;
	return (0);
}

int	ft_write_expanded_line(char *ln, char **env, int fd)
{
	size_t	i;

	i = 0;
	while (ln[i])
	{
		if (ln[i] == '$')
		{
			if (ft_handle_dollar(ln, &i, env, fd) == -1)
				return (-1);
			continue ;
		}
		if (write(fd, &ln[i], 1) == -1)
			return (-1);
		i++;
	}
	if (write(fd, "\n", 1) == -1)
		return (-1);
	return (0);
}
