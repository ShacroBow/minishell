#include "../minishell.h"

void	*ft_resize_buffer(void *ptr, size_t size)
{
	void	*new_ptr;

	new_ptr = ft_calloc(size, sizeof(char));
	if (!new_ptr)
		exit_error("Allocation new buffer malloc failed");
	if (ptr)
	{
		ft_memcpy(new_ptr, ptr, size / 2);
		free(ptr);
	}
	return (new_ptr);
}

void	expand_exit_status(t_tokenize *t)
{
	char	*tmp;
	int		i;

	i = 0;
	tmp = ft_itoa(g_exit_status);
	if (!tmp)
		exit_error("malloc failed");
	while (tmp[i])
	{
		buf_append(&t->buf, &t->cap, &t->len, tmp[i]);
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
		buf_append(&t->buf, &t->cap, &t->len, val[k]);
		k++;
	}
}

int	is_operator(char c)
{
	return (c == '|' || c == '&' || c == '<' || c == '>' || c == '(' || c == ')');
}
void	exit_error(char *context)
{
	ft_putstr_fd("minishell: ", STDERR_FILENO);
	ft_putstr_fd(context, STDERR_FILENO);
	ft_putstr_fd(": ", STDERR_FILENO);
	ft_putstr_fd(strerror(errno), STDERR_FILENO);
	ft_putstr_fd("\n", STDERR_FILENO);
	exit(EXIT_FAILURE);
}