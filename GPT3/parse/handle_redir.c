#include "../minishell.h"

int	write_value(int fd, const char *s)
{
	if (s)
		while (*s)
			if (write(fd, s++, 1) == -1)
				return (-1);
	return (0);
}

char	*hd_tmp_name(void)
{
	static int	seq;
	char		*pid_str;
	char		*seq_str;
	char		*tmp;
	char		*res;

	pid_str = ft_itoa(getpid());
	seq_str = ft_itoa(seq++);
	if (!pid_str || !seq_str)
		return (free(pid_str), free(seq_str), NULL);
	tmp = ft_strjoin(HD_PREFIX, pid_str);
	res = ft_strjoin(tmp, "_");
	free(tmp);
	tmp = res;
	res = ft_strjoin(res, seq_str);
	free(pid_str);
	free(seq_str);
	free(tmp);
	return (res);
}

static int	set_infile(char *name, t_command *c)
{
	int	ret;

	if (c->infile && c->heredoc)
		unlink(c->infile);
	free(c->infile);
	c->infile = ft_strdup(name);
	c->heredoc = 0;
	ret = 0;
	if (c->infile == NULL)
		ret = -1;
	return (ret);
}

static int	set_outfile(char *name, t_command *c, int append)
{
	free(c->outfile);
	c->outfile = ft_strdup(name);
	if (!c->outfile)
		return (-1);
	c->append = append;
	return (0);
}

int	handle_redirection(t_token *tk, int *i, int n, t_command *c)
{
	t_tokentype	tp;

	tp = tk[*i].type;
	(*i)++;
	if (*i >= n || tk[*i].type != TOK_WORD)
		return (redir_error());
	if (tp == TOK_REDIR_IN && set_infile(tk[*i].value, c) == -1)
		return (-1);
	if (tp == TOK_HEREDOC && handle_here_doc(tk, i, c) == -1)
		return (-1);
	if ((tp == TOK_REDIR_OUT || tp == TOK_APPEND) && \
		set_outfile(tk[*i].value, c, tp == TOK_APPEND) == -1)
		return (-1);
	(*i)++;
	return (0);
}
