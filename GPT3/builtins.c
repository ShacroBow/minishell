#include "minishell.h"
#include <limits.h>
#include <ctype.h>

static void     put_env(char ***envp, const char *kv);

/* ───────── helper: get environment variable value ───────── */
static char *get_env_val(char **envp, const char *name) {
	size_t len = strlen(name);
	for (int i = 0; envp[i]; ++i) {
		if (!ft_strncmp(envp[i], name, len) && envp[i][len] == '=')
			return (envp[i] + len + 1);
	}
	return NULL;
}

/* ───────── detect if a command is a builtin ───────── */
int is_builtin(const char *cmd) {
	return (!strcmp(cmd, "echo")   || !strcmp(cmd, "cd")    ||
			!strcmp(cmd, "pwd")    || !strcmp(cmd, "export") ||
			!strcmp(cmd, "unset")  || !strcmp(cmd, "env")    ||
			!strcmp(cmd, "exit"));
}

/* ───────── builtin echo (with -n option) ───────── */
static int builtin_echo(char **argv) {
	int i = 1;
	int no_newline = 0;
	if (argv[1] && !strcmp(argv[1], "-n")) {
		no_newline = 1;
		i++;
	}
	for (; argv[i]; ++i) {
		write(1, argv[i], strlen(argv[i]));
		if (argv[i + 1])
			write(1, " ", 1);
	}
	if (!no_newline)
		write(1, "\n", 1);
	return (0);
}

/* ───────── builtin pwd (print current directory) ───────── */
static int builtin_pwd(void) {
	char buf[PATH_MAX];
	if (getcwd(buf, sizeof(buf))) {
		write(1, buf, strlen(buf));
		write(1, "\n", 1);
		return (0);
	}
	perror("pwd");
	return (1);
}

/* ───────── builtin cd (change directory) ───────── */
static int builtin_cd(char **argv, char ***envp) {
	if (argv[1] && argv[2])
		return (write(2, "minishell: cd: too many arguments\n", 34), 1);
	const char *path = argv[1] ? argv[1] : get_env_val(*envp, "HOME");
	if (!path)
		return (write(2, "minishell: cd: HOME not set\n", 28), 1);
	char cwd_old[PATH_MAX];
	if (!getcwd(cwd_old, sizeof(cwd_old)))
		cwd_old[0] = '\0';
	if (chdir(path) == -1)
		return (perror("cd"), 1);
	char cwd_new[PATH_MAX];
	if (getcwd(cwd_new, sizeof(cwd_new))) {
		char *oldpwd_str = ft_strjoin("OLDPWD=", cwd_old);
		if (!oldpwd_str)
			return (perror("minishell"), 1);
		char *pwd_str = ft_strjoin("PWD=", cwd_new);
		if (!pwd_str) {
			free(oldpwd_str);
			return (perror("minishell"), 1);
		}
		put_env(envp, oldpwd_str);
		put_env(envp, pwd_str);
		free(oldpwd_str);
		free(pwd_str);
	}
	return (0);
}

/* ───────── builtin env (print environment) ───────── */
static int builtin_env(char **envp) {
	for (int i = 0; envp[i]; ++i) {
		write(1, envp[i], strlen(envp[i]));
		write(1, "\n", 1);
	}
	return (0);
}

/* ───────── helper: validate export key format ───────── */
static int is_valid_key(const char *s) {
	if (!(*s) || (!isalpha(*s) && *s != '_'))
		return (0);
	while (*s && *s != '=') {
		if (!isalnum(*s) && *s != '_')
			return (0);
		++s;
	}
	return (1);
}

/* ───────── helper: add or replace an env variable ───────── */
static void put_env(char ***envp, const char *kv) {
	size_t keylen = 0;
	while (kv[keylen] && kv[keylen] != '=')
		++keylen;
	for (int i = 0; (*envp)[i]; ++i) {
		if (!strncmp((*envp)[i], kv, keylen) && (*envp)[i][keylen] == '=') {
			if (kv[keylen] == '\0')
				return;
			free((*envp)[i]);
			(*envp)[i] = ft_strdup(kv);
			return;
		}
	}
	int count = 0;
	while ((*envp)[count])
		++count;
	char **newenv = malloc((count + 2) * sizeof(char *));
	if (!newenv) {
		perror("minishell");
		return;
	}
	for (int j = 0; j < count; ++j)
		newenv[j] = (*envp)[j];
	char *new_str;
	if (kv[keylen] == '\0') {
		new_str = malloc(keylen + 2);
		if (!new_str) {
			perror("minishell");
			return;
		}
		memcpy(new_str, kv, keylen);
		new_str[keylen] = '=';
		new_str[keylen+1] = '\0';
	} else {
		new_str = ft_strdup(kv);
		if (!new_str) {
			perror("minishell");
			return;
		}
	}
	newenv[count] = new_str;
	newenv[count + 1] = NULL;
	free(*envp);
	*envp = newenv;
}

/* ───────── builtin export (add to env) ───────── */
static int builtin_export(char **argv, char ***envp) {
	if (!argv[1])
		return (builtin_env(*envp));
	for (int i = 1; argv[i]; ++i) {
		if (!is_valid_key(argv[i])) {
			write(2, "minishell: export: not a valid identifier\n", 42);
			return (1);
		}
		put_env(envp, argv[i]);
	}
	return (0);
}

/* ───────── builtin unset (remove from env) ───────── */
static int builtin_unset(char **argv, char ***envp) {
	for (int i = 1; argv[i]; ++i) {
		size_t len = strlen(argv[i]);
		for (int j = 0; (*envp)[j]; ++j) {
			if (!strncmp((*envp)[j], argv[i], len) && (*envp)[j][len] == '=') {
				free((*envp)[j]);
				while ((*envp)[j]) {
					(*envp)[j] = (*envp)[j + 1];
					++j;
				}
				break;
			}
		}
	}
	return (0);
}

/* ───────── builtin exit (exit shell) ───────── */
int builtin_exit(char **argv) {
	if (argv[1] && argv[2]) {
		write(2, "minishell: exit: too many arguments\n", 35);
		return (1);
	}
	long code = 0;
	if (argv[1]) {
		char *end;
		code = strtol(argv[1], &end, 10);
		if (*end) {
			write(2, "minishell: exit: numeric argument required\n", 44);
			code = 255;
			write(1, "exit\n", 5);
			exit(255);
		}
		code &= 0xff;
	} else {
		code = g_exit_status & 0xff;
	}
	write(1, "exit\n", 5);
	exit((int)code);
	return (0);
}

/* ───────── dispatch to the correct builtin ───────── */
char **execute_builtin(t_command *cmd, char **envp) {
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
