/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_heredoc_bonus.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:04:09 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:21:56 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell_bonus.h"

static int	ft_hd_tmp(char **file, int *fd)
{
	*file = ft_hd_tmp_name();
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

static int	ft_hd_write(int fd, char *ln, int raw, char **env)
{
	if (raw)
	{
		if (write(fd, ln, ft_strlen(ln)) == -1)
			return (-1);
		return (write(fd, "\n", 1));
	}
	return (ft_write_expanded_line(ln, env, fd));
}

static int	ft_hd_loop(int fd, char *lim, int raw, char **env)
{
	char	*ln;

	ft_signals_heredoc_setup();
	ln = readline("> ");
	while (g_exit_status != 130 && ln && ft_strcmp(ln, lim))
	{
		if (ft_hd_write(fd, ln, raw, env) == -1)
			return (free(ln), -1);
		free(ln);
		ln = readline("> ");
	}
	free(ln);
	ft_signal_setup();
	return (g_exit_status == 130);
}

int	ft_handle_heredoc(t_token *tk, int *idx, t_command *c)
{
	char	*file;
	int		fd;
	char	*lim;
	int		raw;

	lim = tk[*idx].value;
	raw = tk[*idx].quoted;
	if (ft_hd_tmp(&file, &fd) == -1)
		return (-1);
	if (ft_hd_loop(fd, lim, raw, c->envp))
		return (close(fd), unlink(file), free(file), -1);
	close(fd);
	if (c->infile && c->heredoc)
		unlink(c->infile);
	free(c->infile);
	c->infile = file;
	c->heredoc = 1;
	return (0);
}
