#include "../minishell.h"

int	ft_isbuiltin(const char *cmd)
{
	return (!ft_strcmp(cmd, "echo") || !ft_strcmp(cmd, "cd") || \
			!ft_strcmp(cmd, "pwd") || !ft_strcmp(cmd, "export") || \
			!ft_strcmp(cmd, "unset") || !ft_strcmp(cmd, "env") || \
			!ft_strcmp(cmd, "exit"));
}

static int	ft_pwd(void)
{
	char	buf[PATH_MAX];

	if (getcwd(buf, sizeof(buf)))
	{
		write(1, buf, strlen(buf));
		write(1, "\n", 1);
		return (0);
	}
	perror("pwd");
	return (1);
}

int	ft_env(char **envp)
{
	int	i;

	i = 0;
	while (envp[i])
	{
		ft_putstr_fd(envp[i], 1);
		ft_putstr_fd("\n", 1);
		i++;
	}
	return (0);
}

char	**execute_builtin(t_command *cmd, char **envp)
{
	if (!ft_strcmp(cmd->argv[0], "echo"))
		g_exit_status = ft_echo(cmd->argv);
	else if (!ft_strcmp(cmd->argv[0], "cd"))
		g_exit_status = ft_cd(cmd->argv, &envp);
	else if (!ft_strcmp(cmd->argv[0], "pwd"))
		g_exit_status = ft_pwd();
	else if (!ft_strcmp(cmd->argv[0], "export"))
		g_exit_status = ft_export(cmd->argv, &envp);
	else if (!ft_strcmp(cmd->argv[0], "unset"))
		g_exit_status = ft_unset(cmd->argv, &envp);
	else if (!ft_strcmp(cmd->argv[0], "env"))
	{
		if (cmd->argv[1])
		{
			write(2, "minishell: env: too many arguments\n", 35);
			g_exit_status = 1;
		}
		else
			g_exit_status = ft_env(envp);
	}
	else if (!ft_strcmp(cmd->argv[0], "exit"))
		g_exit_status = ft_exit(cmd->argv);
	return (envp);
}
