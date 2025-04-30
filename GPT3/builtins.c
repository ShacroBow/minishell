#include "minishell.h"
#include <limits.h>
#include <ctype.h>

static void	put_env(char ***envp, const char *kv);

/* ───────── helper: get environment variable value ───────── */
static char	*get_env_val(char **envp, const char *name)
{
	size_t len;
	int i;

	i = 0;
	len = ft_strlen(name);
	while (envp[i])
	{
		if (!ft_strncmp(envp[i], name, len) && envp[i][len] == '=')
			return (envp[i] + len + 1);
		i++;
	}
	return NULL;
}

/* ───────── detect if a command is a builtin ───────── */
int	is_builtin(const char *cmd)
{
	return (!ft_strcmp(cmd, "echo") || !ft_strcmp(cmd, "cd") || \
			!ft_strcmp(cmd, "pwd") || !ft_strcmp(cmd, "export") || \
			!ft_strcmp(cmd, "unset") || !ft_strcmp(cmd, "env") || \
			!ft_strcmp(cmd, "exit"));
}

/* ───────── builtin echo (with -n option) ───────── */
static int  is_n_flag(const char *s)
{
	int i;

	if (!s || s[0] != '-')
		return (0);
	i = 1;
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (s[i] != 'n')
			return (0);
		i++;
	}
	return (1);
}

int builtin_echo(char **argv)
{
	int i;
	int print_nl;

	i = 1;
	print_nl = 1;
	while (argv[i] && is_n_flag(argv[i]))
	{
		print_nl = 0;
		i++;
	}
	while (argv[i])
	{
		ft_putstr_fd(argv[i], STDOUT_FILENO);
		if (argv[i + 1])
			write(1, " ", 1);
		i++;
	}
	if (print_nl)
		write(1, "\n", 1);
	return (0);
}

/* ───────── builtin pwd (print current directory) ───────── */
static int	builtin_pwd(void)
{
	char buf[PATH_MAX];
	if (getcwd(buf, sizeof(buf)))
	{
		write(1, buf, strlen(buf));
		write(1, "\n", 1);
		return (0);
	}
	perror("pwd");
	return (1);
}

/* ───────── builtin cd (change directory) ───────── */
static const char	*pick_path(char **av, char **env, int *echo_path)
{
	const char	*target;

	*echo_path = 0;
	if (!av[1])
		return (get_env_val(env, "HOME"));
	if (ft_strcmp(av[1], "-") == 0)
	{
		target = get_env_val(env, "OLDPWD");
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
	put_env(e, oldpwd);
	put_env(e, pwd);
	free(oldpwd);
	free(pwd);
	return (0);
}

int builtin_cd(char **argv, char ***envp)
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

/* ───────── builtin env (print environment) ───────── */
static int	builtin_env(char **envp)
{
	int i;

	i = 0;
	while  (envp[i])
	{
		ft_putstr_fd(envp[i], 1);
		ft_putstr_fd("\n", 1);
		i++;
	}
	return (0);
}

/* ───────── helper: validate export key format ───────── */
static int	is_valid_key(const char *s)
{
	int i;

	i = 0;
	if (!(*s) || (!ft_isalpha(*s) && *s != '_'))
		return (0);
	while (s[i] && s[i] != '=')
	{
		if (!ft_isalnum(s[i]) && s[i] != '_')
			return (0);
		i++;
	}
	return (1);
}

/* ───────── helper: add or replace an env variable ───────── */
static char	*dup_keyval(const char *kv, size_t klen)
{
	char	*out;

	if (kv[klen] == '\0')
	{
		out = malloc(klen + 2);
		if (!out)
			return (NULL);
		ft_memcpy(out, kv, klen);
		out[klen] = '=';
		out[klen + 1] = '\0';
		return (out);
	}
	return (ft_strdup(kv));
}

static int	append_env(char ***envp, char *newstr)
{
	char	**grow;
	int		len;
	int		i;

	len = 0;
	while ((*envp)[len])
		len++;
	grow = malloc(sizeof(char *) * (len + 2));
	if (!grow)
		return (1);
	i = 0;
	while (i < len)
	{
		grow[i] = (*envp)[i];
		i++;
	}
	grow[len] = newstr;
	grow[len + 1] = NULL;
	free(*envp);
	*envp = grow;
	return (0);
}

void	put_env(char ***envp, const char *kv)
{
	size_t	klen;
	int		i;
	char	*newstr;

	klen = 0;
	while (kv[klen] && kv[klen] != '=')
		klen++;
	i = 0;
	while ((*envp)[i])
	{
		if (!ft_strncmp((*envp)[i], kv, klen)
			&& (*envp)[i][klen] == '=')
		{
			if (kv[klen] == '\0')
				return ;
			free((*envp)[i]);
			(*envp)[i] = ft_strdup(kv);
			return ;
		}
		i++;
	}
	newstr = dup_keyval(kv, klen);
	if (!newstr || append_env(envp, newstr))
		perror("minishell");
}

/* ───────── builtin export (add to env) ───────── */
static int	builtin_export(char **argv, char ***envp)
{
	int i;

	i = 1;
	if (!argv[1])
		return (builtin_env(*envp));
	while (argv[i])
	{
		if (!is_valid_key(argv[i]))
		{
			ft_putstr_fd("minishell: export: not a valid identifier\n", 2);
			return (1);
		}
		put_env(envp, argv[i]);
		i++;
	}
	return (0);
}

/* ───────── builtin unset (remove from env) ───────── */
static void remove_at(char ***envp, int idx)
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
static int	builtin_unset(char **argv, char ***envp)
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
				remove_at(envp, j);
				break;
			}
			j++;
		}
		i++;
	}
	return (0);
}

/* ───────── builtin exit (exit shell) ───────── */
static int  parse_numeric(const char *str, long *num)
{
	const char	*ptr;
	int			digits;

	ptr = str;
	while (ft_isspace((unsigned char)*ptr))
		ptr++;
	*num = ft_atol(ptr, &digits);
	if (digits == 0)
		return (1);
	if (*ptr == '+' || *ptr == '-')
		ptr++;
	while (*ptr == '0')
		ptr++;
	ptr += digits;
	while (ft_isspace((unsigned char)*ptr))
		ptr++;
	return (*ptr != '\0');
}

int builtin_exit(char **argv)
{
	int		argc;
	long	code;

	argc = 0;
	while (argv[argc])
		argc++;
	if (argc > 2)
		return (ft_putstr_fd("minishell: exit: too many arguments\n", 2), 1);
	if (argc == 2)
	{
		if (parse_numeric(argv[1], &code))
		{
			ft_putstr_fd("minishell: exit: numeric argument required\n", 2);
			ft_putstr_fd("exit\n", 1);
			exit(255);
		}
		code &= 0xFF;
	}
	else
		code = g_exit_status & 0xFF;
	ft_putstr_fd("exit\n", 1);
	exit((int)code);
	return (0);
}

/* ───────── dispatch to the correct builtin ───────── */
char	**execute_builtin(t_command *cmd, char **envp)
{
	if (!ft_strcmp(cmd->argv[0], "echo"))
		g_exit_status = builtin_echo(cmd->argv);
	else if (!ft_strcmp(cmd->argv[0], "cd"))
		g_exit_status = builtin_cd(cmd->argv, &envp);
	else if (!ft_strcmp(cmd->argv[0], "pwd"))
		g_exit_status = builtin_pwd();
	else if (!ft_strcmp(cmd->argv[0], "export"))
		g_exit_status = builtin_export(cmd->argv, &envp);
	else if (!ft_strcmp(cmd->argv[0], "unset"))
		g_exit_status = builtin_unset(cmd->argv, &envp);
	else if (!ft_strcmp(cmd->argv[0], "env")) {
		if (cmd->argv[1]) {
			write(2, "minishell: env: too many arguments\n", 35);
			g_exit_status = 1;
		} else {
			g_exit_status = builtin_env(envp);
		}
	} else if (!ft_strcmp(cmd->argv[0], "exit")) {
		g_exit_status = builtin_exit(cmd->argv);
	}
	return envp;
}
