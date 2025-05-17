/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_expandwildcard.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:05 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:04:06 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell.h"

static int	ft_match_pattern(const char *p, const char *t)
{
	if (*p == '\0')
		return (*t == '\0');
	if (*p == '*')
	{
		while (*(p + 1) == '*')
			p++;
		if (ft_match_pattern(p + 1, t))
			return (1);
		if (*t && ft_match_pattern(p, t + 1))
			return (1);
		return (0);
	}
	if (*p == *t && *t)
		return (ft_match_pattern(p + 1, t + 1));
	return (0);
}

static int	ft_add_match(char ***tab, int *cnt, const char *name)
{
	char	**tmp;
	int		i;

	tmp = malloc(sizeof(char *) * (*cnt + 2));
	if (!tmp)
		return (-1);
	i = 0;
	while (i < *cnt)
	{
		tmp[i] = (*tab)[i];
		i++;
	}
	tmp[i] = ft_strdup(name);
	if (!tmp[i])
		return (free(tmp), -1);
	tmp[++i] = NULL;
	free(*tab);
	*tab = tmp;
	(*cnt)++;
	return (0);
}

static void	ft_sort_matches(char **tab, int n)
{
	int		i;
	int		j;
	char	*swap;

	i = 0;
	while (i < n - 1)
	{
		j = i + 1;
		while (j < n)
		{
			if (ft_strcmp(tab[i], tab[j]) > 0)
			{
				swap = tab[i];
				tab[i] = tab[j];
				tab[j] = swap;
			}
			j++;
		}
		i++;
	}
}

char	**ft_expand_wildcard(const char *pat)
{
	DIR				*d;
	struct dirent	*e;
	char			**tab;
	int				cnt;

	tab = NULL;
	cnt = 0;
	d = opendir(".");
	if (!d)
		return (NULL);
	e = readdir(d);
	while (e)
	{
		e = readdir(d);
		if (!e || (!(pat[0] == '.') && e->d_name[0] == '.'))
			continue ;
		if (ft_match_pattern(pat, e->d_name))
			if (ft_add_match(&tab, &cnt, e->d_name) == -1)
				return (closedir(d), ft_free_tab(tab), NULL);
	}
	closedir(d);
	if (!tab)
		return (NULL);
	ft_sort_matches(tab, cnt);
	return (tab);
}
