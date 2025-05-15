#include "../minishell.h"

static void	*ft_resize_buffer(void *ptr, size_t size)
{
	void	*new_ptr;

	new_ptr = ft_calloc(size, sizeof(char));
	if (!new_ptr)
		ft_exit_error("Allocation new buffer malloc failed");
	if (ptr)
	{
		ft_memcpy(new_ptr, ptr, size / 2);
		free(ptr);
	}
	return (new_ptr);
}

void	ft_tok_append(t_tokenize *t, char c)
{
	char	*new_buf;

	if (t->len + 1 >= t->cap)
	{
		if (t->cap > INT_MAX / 2)
			ft_exit_tokenize(t, "Capacity buffer realloc: buffer overflow");
		t->cap *= 2;
		new_buf = ft_resize_buffer(t->buf, t->cap);
		if (!new_buf)
			ft_exit_tokenize(t, "buffer realloc failed: mallocfailed");
		t->buf = new_buf;
	}
	t->buf[t->len++] = c;
	t->buf[t->len] = '\0';
}

int	ft_handle_filler(t_tokenize *t, const char *input, int *i)
{
	char	c;

	c = input[*i];
	if (c == '\'' && !t->in_d)
	{
		t->in_s ^= 1;
		t->quoted = 1;
		(*i)++;
		return (1);
	}
	else if (c == '\"' && !t->in_s)
	{
		t->in_d ^= 1;
		t->quoted = 1;
		(*i)++;
		return (1);
	}
	if (!t->in_s && !t->in_d && ft_isspace(c))
	{
		if (t->len)
			ft_create_token(t);
		(*i)++;
		return (1);
	}
	return (0);
}
