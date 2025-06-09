/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cd_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: kmashkoo <kmashkoo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 19:03:29 by kmashkoo          #+#    #+#             */
/*   Updated: 2025/05/17 19:21:56 by kmashkoo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minishell_bonus.h"

static char	*ft_get_env_val(char **envp, const char *name)
{
	size_t	len;
	int		i;

	i = 0;
	len = ft_strlen(name);
	while (envp[i])
	{
		if (!ft_strncmp(envp[i], name, len) && envp[i][len] == '=')
			return (envp[i] + len + 1);
		i++;
	}
	return (NULL);
}

static const char	*pick_path(char **av, char **env, int *echo_path)
{
	const char	*target;

	*echo_path = 0;
	if (!av[1])
		return (ft_get_env_val(env, "HOME"));
	if (ft_strcmp(av[1], "-") == 0)
	{
		target = ft_get_env_val(env, "OLDPWD");
		if (!target)
			return (ft_putstr_fd("minishell: cd: OLDPWD not set\n", 2), NULL);
		*echo_path = 1;
		return (target);
	}
	return (av[1]);
}

static int	update_pwd(char ***e, const char *old, const char *new_dir)
{
	char	*oldpwd;
	char	*pwd;

	oldpwd = ft_strjoin("OLDPWD=", old);
	if (!oldpwd)
		return (perror("minishell"), 1);
	pwd = ft_strjoin("PWD=", new_dir);
	if (!pwd)
	{
		free(oldpwd);
		return (perror("minishell"), 1);
	}
	ft_put_env(e, oldpwd);
	ft_put_env(e, pwd);
	free(oldpwd);
	free(pwd);
	return (0);
}

int	ft_cd(char **argv, char ***envp)
{
	char		cwd_old[PATH_MAX];
	char		cwd_new[PATH_MAX];
	const char	*path;
	int			echo_path;

	if (argv[1] && argv[2])
		return (ft_putstr_fd("minishell: cd: too many arguments\n", 2), 1);
	path = pick_path(argv, *envp, &echo_path);
	if (!path)
		return (ft_putstr_fd("minishell: cd: HOME not set\n", 2), 1);
	if (!getcwd(cwd_old, PATH_MAX))
		cwd_old[0] = '\0';
	if (chdir(path) == -1)
		return (perror("cd"), 1);
	if (getcwd(cwd_new, PATH_MAX))
		if (update_pwd(envp, cwd_old, cwd_new))
			return (1);
	if (echo_path)
		printf("%s\n", cwd_new);
	return (0);
}
