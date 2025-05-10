#include "../minishell.h"

void	ft_close_pipes(t_pipe_fd *tab, int count)
{
	int	k;

	k = 0;
	while (k < count)
	{
		close(tab[k][0]);
		close(tab[k][1]);
		k++;
	}
}

t_pipe_fd	*ft_make_pipes(int n)
{
	int			i;
	t_pipe_fd	*tab;

	if (n <= 1)
		return (NULL);
	tab = malloc(sizeof(t_pipe_fd) * (n - 1));
	if (!tab)
		exit(EXIT_FAILURE);
	i = 0;
	while (i < n - 1)
	{
		if (pipe(tab[i]) == -1)
		{
			perror("pipe");
			ft_close_pipes(tab, i);
			free(tab);
			return (NULL);
		}
		i++;
	}
	return (tab);
}
