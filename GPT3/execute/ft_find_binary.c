#include "../minishell.h"

static char *ft_get_env_val(char **envp, const char *key)
{
	size_t len;
	size_t i;

	i = 0;
	len = ft_strlen(key);
	while (envp && envp[i])
	{
		if (!ft_strncmp(envp[i], key, len) && envp[i][len] == '=')
			return (envp[i] + len + 1);
		i++;
	}
	return NULL;
}

static void ft_free_doubleptr(char **arr)
{
	int i = 0;

	if (arr == NULL)
		return ;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}

static char	*ft_find_path(char **dirs, const char *cmd)
{
	int		i;
	char	*tmp;
	char	*full;

	i = 0;
	while (dirs[i])
	{
		tmp = ft_strjoin(dirs[i], "/");
		if (!tmp)
			return (NULL);
		full = ft_strjoin(tmp, cmd);
		free(tmp);
		if (!full)
			return (NULL);
		if (access(full, X_OK) == 0)
			return (full);
		free(full);
		i++;
	}
	return (NULL);
}

char	*ft_find_binary(const char *cmd, char **envp)
{
	char	**dirs;
	char	*pathval;
	char	*found;

	if (!cmd || !*cmd)
		return (NULL);
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, X_OK) == 0)
			return (ft_strdup(cmd));
		return (NULL);
	}
	pathval = ft_get_env_val(envp, "PATH");
	if (!pathval || !*pathval)
		pathval = "/usr/local/bin:/usr/bin:/bin";
	dirs = ft_split(pathval, ':');
	if (!dirs)
		return (NULL);
	found = ft_find_path(dirs, cmd);
	ft_free_doubleptr(dirs);
	return (found);
}
