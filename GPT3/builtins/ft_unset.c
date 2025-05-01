#include "../minishell.h"

static void	ft_remove_at(char ***envp, int idx)
{
	int	k;

	free((*envp)[idx]);
	k = idx;
	while ((*envp)[k])
	{
		(*envp)[k] = (*envp)[k + 1];
		k++;
	}
}

int	ft_unset(char **argv, char ***envp)
{
	int		i;
	int		j;
	size_t	len;

	i = 1;
	while (argv[i])
	{
		len = ft_strlen(argv[i]);
		j = 0;
		while ((*envp)[j])
		{
			if (!ft_strncmp((*envp)[j], argv[i], len) && \
				(*envp)[j][len] == '=')
			{
				ft_remove_at(envp, j);
				break ;
			}
			j++;
		}
		i++;
	}
	return (0);
}
