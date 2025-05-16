#include "../minishell.h"

static int	hd_tmp(char **file, int *fd)
{
	*file = hd_tmp_name();
	if (!*file)
		return (-1);
	*fd = open(*file, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (*fd < 0)
	{
		perror("minishell");
		free(*file);
		return (-1);
	}
	return (0);
}

static int	hd_write(int fd, char *ln, int raw, char **env)
{
	if (raw)
	{
		if (write(fd, ln, ft_strlen(ln)) == -1)
			return (-1);
		return (write(fd, "\n", 1));
	}
	return (write_expanded_line(ln, env, fd));
}

static int	hd_loop(int fd, char *lim, int raw, char **env)
{
	char	*ln;

	ft_signals_heredoc_setup();
	ln = readline("> ");
	while (g_exit_status != 130 && ln && ft_strcmp(ln, lim))
	{
		if (hd_write(fd, ln, raw, env) == -1)
			return (free(ln), -1);
		free(ln);
		ln = readline("> ");
	}
	free(ln);
	ft_signal_setup();
	return (g_exit_status == 130);
}

int	handle_here_doc(t_token *tk, int *idx, t_command *c)
{
	char	*file;
	int		fd;
	char	*lim;
	int		raw;

	lim = tk[*idx].value;
	raw = tk[*idx].quoted;
	if (hd_tmp(&file, &fd) == -1)
		return (-1);
	if (hd_loop(fd, lim, raw, c->envp))
		return (close(fd), unlink(file), free(file), -1);
	close(fd);
	if (c->infile && c->heredoc)
		unlink(c->infile);
	free(c->infile);
	c->infile = file;
	c->heredoc = 1;
	return (0);
}
